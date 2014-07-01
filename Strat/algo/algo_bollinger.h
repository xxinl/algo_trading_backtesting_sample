/*
idea principle: price rever to mean after a dramatic movement
implementation: a. bollinger band + anti trend, regression mean price;
								if price jump up or down, oberve a period to confirm the sudden jump, then hold until it reverse
*/


#ifndef _STRAT_BOLLINGER
#define _STRAT_BOLLINGER

#include "tick.h"
#include "position.h"
#include "algo.h"
#include "optimizer/optimizable_algo_genetic.h"

#include <vector>
#include <queue>
#include <cmath>

#include <boost/date_time.hpp>

#include <concurrent_vector.h>

using std::string;

namespace strat{

	class algo_bollinger : public algo,
		public optimizable_algo_genetic<size_t, size_t, double, double>{

	private:

		size_t _obser_win;
		size_t _hold_win;
		double _initial_threshold;
		double _obser_threshold;

		tick _last_tick;
		bool _can_obser;
		bool _obser_up;

		bool _check_if_obser(const tick& crr_tick){

			_obser_up = crr_tick.last - _last_tick.last > 0;
			return std::abs(crr_tick.last - _last_tick.last) > _initial_threshold;
		}

		void _update_last_tick(const tick& crr_tick){

			if (crr_tick.time_stamp >= _last_tick.time_stamp + boost::posix_time::minutes(1)){
			
				_last_tick = crr_tick;
			}
		}

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {

			signal ret_sig = signal::NONE;
			
			if (crr_tick.time_stamp >= _last_tick.time_stamp + boost::posix_time::minutes(_obser_win)){

				if (_obser_up && crr_tick.last >= _last_tick.last + _obser_threshold){

					ret_sig = signal::SELL;
					_add_position(crr_tick, ret_sig);
				}
				else if (!_obser_up && crr_tick.last <= _last_tick.last - _obser_threshold){

					ret_sig = signal::BUY;
					_add_position(crr_tick, ret_sig);
				}
				else if (crr_tick.time_stamp >= _last_tick.time_stamp + boost::posix_time::minutes(60)){

					_can_obser = true;
					_update_last_tick(crr_tick);

#ifdef MQL5_RELEASE
					if (!_is_log_off)
						LOG("canceled observe, no singal for the last 60 minutes, at tick " << crr_tick.time_stamp);
#endif MQL5_RELEASE
				}
			}

			return ret_sig;
		}

		int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss) override{
			
			bool is_stop_out = stop_loss != -1 && (_position.open_tick.last - crr_tick.last) * _position.type > stop_loss;

			if (is_stop_out ||
				crr_tick.time_stamp >= _position.open_tick.time_stamp + boost::posix_time::minutes(_hold_win))
			{
				_position.close_tick = crr_tick;
				close_pos = _position;
				_delete_position();

				return 1;
			}

			return 0;
		}

	public:

#pragma region constructors

		algo_bollinger(const string s_base, const string s_quote,
			size_t obser_win, size_t hold_win, double initial_threshold,
			double obser_threshold, bool is_anti_trend = true) :
			algo(s_base, s_quote), _obser_win(obser_win), _hold_win(hold_win),
			_initial_threshold(initial_threshold), _obser_threshold(obser_threshold),
			_can_obser(true){
		
			_last_tick.last = -1;
		};


		/// Destructor
		~algo_bollinger(){};

#pragma endregion

		signal process_tick(const tick& crr_tick, position& close_pos, double stop_loss = -1) override{

			//first tick
			if (_last_tick.last == -1)
				_last_tick = crr_tick;

			if (_can_obser){

				if (_check_if_obser(crr_tick)){

					_can_obser = false;

#ifdef MQL5_RELEASE
					if (!_is_log_off)
						LOG("observed at tick " << crr_tick.time_stamp << " " << crr_tick.last << "; last tick " << _last_tick.last);
#endif MQL5_RELEASE
				}

				_update_last_tick(crr_tick);

				return signal::NONE;
			}
			else {

				if (has_open_position()){

					if (_close_position_algo(crr_tick, close_pos, stop_loss))	{

						_can_obser = true;
						_update_last_tick(crr_tick);
					}

					return signal::NONE;
				}
				else{

					return _get_signal_algo(crr_tick);
				}
			}
		}

		void reset_params(size_t obser_win, size_t hold_win, double initial_threshold, double obser_threshold){

			_obser_win = obser_win;
			_hold_win = hold_win;
			_initial_threshold = initial_threshold;
			_obser_threshold = obser_threshold;

			LOG("reset param obser:" << _obser_win << " hold:" << _hold_win << 
				" ini_t:" << _initial_threshold << " obser_t:" << obser_threshold);
		}


#pragma region optimizable_algo members

		typedef std::pair<double, std::tuple<size_t, size_t, double, double>> CITIZEN_TYPE;

		std::tuple<size_t, size_t, double, double> get_random_citizen(){

			return std::make_tuple(
				_rand_from_range(1, 10),
				_rand_from_range(1, 360),
				_rand_from_range(1, 100) * 0.00010,
				_rand_from_range(1, 100) * 0.00010
				);
		}

		concurrency::concurrent_vector<CITIZEN_TYPE> init_optimization_population(int population_size) override{

			concurrency::concurrent_vector<CITIZEN_TYPE> population;

			//add current parameters
			population.push_back(std::make_pair(0,
				std::make_tuple(
				_obser_win,
				_hold_win,
				_initial_threshold,
				_obser_threshold
				)));

			for (int i = 0; i < population_size - 1; i++){

				CITIZEN_TYPE citizen =
					std::make_pair<double, std::tuple<size_t, size_t, double, double>>(0, get_random_citizen());
				population.push_back(citizen);
			}

			return population;
		}

		std::shared_ptr<algo> get_optimizable_algo(std::tuple<size_t, size_t, double, double> params) override{

			algo_bollinger* ret_algo = new algo_bollinger(_s_base, _s_quote,
				std::get<0>(params), std::get<1>(params), std::get<2>(params), std::get<3>(params));
			
			//disable logging for open and close position/oberv
			ret_algo->toggle_log_switch();

			std::shared_ptr<algo> casted_ret = std::make_shared<algo_bollinger>(*ret_algo);

			return casted_ret;
		}

		std::tuple<size_t, size_t, double, double> mate(
			const std::tuple<size_t, size_t, double, double>& i, const std::tuple<size_t, size_t, double, double>& j) override{

			return std::make_tuple(
				std::get<0>(i),
				std::get<1>(j),
				std::get<2>(i),
				std::get<3>(j)
				);
		}

		std::tuple<size_t, size_t, double, double> mutate(std::tuple<size_t, size_t, double, double> params) override{

			std::tuple<size_t, size_t, double, double> ret = get_random_citizen();

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

		string print_params(std::tuple<size_t, size_t, double, double> params) override{

			return static_cast<std::ostringstream&>(std::ostringstream().flush() <<
				"obser:" << std::get<0>(params) << " hold:" << std::get<1>(params) << 
				" ini_t:" << std::get<2>(params) << " obser_t:" << std::get<3>(params)).str();
		}

#pragma endregion
	};
}

#endif