/*
idea principle: price rever to mean after a dramatic movement
implementation: a. bollinger band + anti trend, regression mean price;
if price jump up or down, oberve a period to confirm the sudden jump, then hold until it reverse
*/


#ifndef _STRAT_BOLLINGER
#define _STRAT_BOLLINGER

#include "tick.h"
#include "position.h"
#include "algo_bar.h"
#include "indicator/sd.h"

#include <vector>
#include <cmath>

#include <boost/date_time.hpp>

#ifndef MQL5_RELEASE

#include "optimizer/optimizable_algo_genetic.h"
#include <concurrent_vector.h>

#endif MQL5_RELEASE

using std::string;

namespace strat{

#ifdef MQL5_RELEASE
	class algo_bollinger : public algo_bar{
#else
	class algo_bollinger : public algo_bar,
		public optimizable_algo_genetic < size_t, double, double, double > {
#endif MQL5_RELEASE

	private:

#pragma region variables

		//input params------------
		size_t _obser_max;
		double _exit_lev;
		double _initial_threshold;
		double _obser_threshold;
		//input params end--------

		tick _last_obser_tick;
		bool _can_obser;
		bool _obser_up;

		const int _last_entry_hour = 21;
		const int _start_close_hour = 22; 

		int _crr_hour;
		bool _can_close_pos;

#pragma endregion variables

		bool _check_if_obser(const tick& crr_tick){

			double diff = crr_tick.last - _last_obser_tick.last;
			_obser_up = diff > 0;
			return std::abs(diff) > _initial_threshold;
		}

#pragma region bar_warcher handlers

		void _on_new_15sec_bar(const tick& crr_tick, const bar& last_bar){

			_can_close_pos = true;
		}

		void _on_new_min_bar(const tick& crr_tick, const bar& last_bar){

			// update _last_obser_tick every minute
			if (_can_obser)
				_last_obser_tick = crr_tick;
		}

		void _on_new_hour_bar(const tick& crr_tick, const bar& last_bar){

			_crr_hour = crr_tick.time.time_of_day().hours();
		}

#pragma endregion bar_warcher handlers

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {

			signal ret_sig = signal::NONE;

			if (_obser_up && crr_tick.last >= _last_obser_tick.last + _obser_threshold){

				ret_sig = signal::SELL;
				_open_position(crr_tick, ret_sig);
			}
			else if (!_obser_up && crr_tick.last <= _last_obser_tick.last - _obser_threshold){

				ret_sig = signal::BUY;
				_open_position(crr_tick, ret_sig);
			}
			else if (crr_tick.time >= _last_obser_tick.time + boost::posix_time::minutes(_obser_max)){

				_can_obser = true;

#ifdef MQL5_RELEASE
				if (!_is_log_off)
					LOG("canceled observe, no singal for the last " << _obser_max << " minutes, at tick " << crr_tick.time);
#endif MQL5_RELEASE
			}

			return ret_sig;
		}

		int _close_position_algo(const tick& crr_tick, position& close_pos, 
			const double stop_loss, const double take_profit) override{

			double profit = _calc_profit(crr_tick);
			bool is_stop_out = algo::_is_stop_out(profit, stop_loss);

			if (is_stop_out || profit > _exit_lev){

				_position.close_tick = crr_tick;
				close_pos = _position;
				_delete_position();

				_can_obser = true;
				_last_obser_tick = crr_tick;

				return 1;
			}

			return 0;
		}

	public:

#pragma region constructors

		algo_bollinger(const string symbol,
			size_t obser_max, double exit_lev, double initial_threshold, double obser_threshold) :
			algo_bar(symbol), _obser_max(obser_max), _exit_lev(exit_lev),
			_initial_threshold(initial_threshold), _obser_threshold(obser_threshold),
			_can_obser(true){

			_attach_watcher(bar_watcher(bar_interval::SEC_15, boost::bind(&algo_bollinger::_on_new_15sec_bar, this, _1, _2)));
			_attach_watcher(bar_watcher(bar_interval::MIN, boost::bind(&algo_bollinger::_on_new_min_bar, this, _1, _2)));
			_attach_watcher(bar_watcher(bar_interval::HOUR, boost::bind(&algo_bollinger::_on_new_hour_bar, this, _1, _2)));
		};


		/// Destructor
		~algo_bollinger(){};

#pragma endregion

		signal process_tick(const tick& crr_tick, position& close_pos, 
			double stop_loss = -1, const double take_profit = -1) override{

			_process_bar_tick(crr_tick);
			
			if (_crr_hour < _start_close_hour){

				//close all positions for previous day
				if (_crr_hour == 0 && has_open_position()){

					_position.close_tick = crr_tick;
					close_pos = _position;
					_delete_position();

					_can_obser = true;
					_last_obser_tick = crr_tick;
				}

				if (_can_obser){

					if (_check_if_obser(crr_tick)){

						_can_obser = false;

#ifdef MQL5_RELEASE
						if (!_is_log_off)
							LOG("observed at tick " << crr_tick.time << " " << crr_tick.last << "; last tick " << _last_obser_tick.last);
#endif MQL5_RELEASE
					}

					return signal::NONE;
				}
				else {

					if (has_open_position()){

						//this reset every 15 sec
						if (_can_close_pos){

							_close_position_algo(crr_tick, close_pos, stop_loss, take_profit);
							_can_close_pos = false;
						}

						return signal::NONE;
					}
					else if (_crr_hour <= _last_entry_hour){

						return _get_signal_algo(crr_tick);
					}
				}
			}
			else{

				// >= _start_close_hour, strat close positions and accept losses
				if (has_open_position()){

					_close_position_algo(crr_tick, close_pos, stop_loss, take_profit);
				}
			}
		}

		void reset_params(size_t obser_max, double exit_lev, double initial_threshold, double obser_threshold){

			_obser_max = obser_max;
			_exit_lev = exit_lev;
			_initial_threshold = initial_threshold;
			_obser_threshold = obser_threshold;

			LOG("reset param obser:" << _obser_max << " exit_lev:" << _exit_lev <<
				" ini_t:" << _initial_threshold << " obser_t:" << obser_threshold);
		}

#ifndef MQL5_RELEASE

#pragma region optimizable_algo members

#define OPTI_PARAMS_BOLLINGER size_t, double, double, double

		typedef std::pair<double, std::tuple<OPTI_PARAMS_BOLLINGER>> CITIZEN_TYPE;

		std::tuple<OPTI_PARAMS_BOLLINGER> get_random_citizen(){

			return std::make_tuple(
				_rand_from_range(1, 60),
				_rand_from_range(2, 50) * 0.00005,
				_rand_from_range(1, 100) * 0.00010, //0.00010
				_rand_from_range(1, 100) * 0.00010
				);
		}

		concurrency::concurrent_vector<CITIZEN_TYPE> init_optimization_population(int population_size) override{

			concurrency::concurrent_vector<CITIZEN_TYPE> population;

			//add current parameters
			population.push_back(std::make_pair(0,
				std::make_tuple(
				_obser_max,
				_exit_lev,
				_initial_threshold,
				_obser_threshold
				)));

			for (int i = 0; i < population_size - 1; i++){

				CITIZEN_TYPE citizen =
					std::make_pair<double, std::tuple<OPTI_PARAMS_BOLLINGER>>(0, get_random_citizen());
				population.push_back(citizen);
			}

			return population;
		}

		std::shared_ptr<algo> get_optimizable_algo(std::tuple<OPTI_PARAMS_BOLLINGER> params) override{

			algo_bollinger* ret_algo = new algo_bollinger(_symbol,
				std::get<0>(params), std::get<1>(params), std::get<2>(params), std::get<3>(params));
			
			std::shared_ptr<algo> casted_ret = std::make_shared<algo_bollinger>(*ret_algo);

			return casted_ret;
		}

		std::tuple<OPTI_PARAMS_BOLLINGER> mate(
			const std::tuple<OPTI_PARAMS_BOLLINGER>& i, const std::tuple<OPTI_PARAMS_BOLLINGER>& j) override{

			return std::make_tuple(
				std::get<0>(i),
				std::get<1>(j),
				std::get<2>(i),
				std::get<3>(j)
				);
		}

		std::tuple<OPTI_PARAMS_BOLLINGER> mutate(std::tuple<OPTI_PARAMS_BOLLINGER> params) override{

			std::tuple<OPTI_PARAMS_BOLLINGER> ret = get_random_citizen();

			int keep = rand() % 4;
			switch (keep){

			case 0:
				std::get<0>(ret) = std::get<0>(params);
				break;

			case 1:
				std::get<1>(ret) = std::get<1>(params);
				break;

			case 2:
				std::get<2>(ret) = std::get<2>(params);
				break;

			case 3:
				std::get<3>(ret) = std::get<3>(params);
				break;
			}

			return ret;
		}

		string print_params(std::tuple<OPTI_PARAMS_BOLLINGER> params) override{

			return static_cast<std::ostringstream&>(std::ostringstream().flush() <<
				std::get<0>(params) << "," << std::get<1>(params) <<
				"," << std::get<2>(params) << "," << std::get<3>(params)).str();
		}

#pragma endregion

#endif MQL5_RELEASE
	};
	}

#endif