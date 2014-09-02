/*
idea principle: daily volatility range complete before the end of the day. 
									trade pullback after the complete
implementation: 
*/


#ifndef _STRAT_DAYRANGE
#define _STRAT_DAYRANGE

#include "tick.h"
#include "position.h"
#include "algo_bar.h"
#include "indicator/sd.h"
#include "risk.h"

#include <vector>
#include <algorithm>

#include <boost/date_time.hpp>

//#include <ppl.h>

#ifndef MQL5_RELEASE

#include "optimizer/optimizable_algo_genetic.h"
#include <concurrent_vector.h>

#endif MQL5_RELEASE

using std::string;

namespace strat{

#ifdef MQL5_RELEASE
	class algo_dayrange : public algo_bar{
#else
	class algo_dayrange : public algo_bar,
		public optimizable_algo_genetic < int, double > {
#endif MQL5_RELEASE

	private:

#pragma region variables

		//input params---
		const int _complete_hour;
		const double _init_exit_lev;
		//input params end---

		const int _last_entry_hour = 21;
		const int _start_close_hour = 22;

		double _high;
		double _low;
		double _entry_lev;
		double _exit_lev;
		bool _is_skip_day;
		int _crr_hour;

		sd _run_sd_min;
		double _sd_multiplier;

		risk _risk;

		//concurrency::critical_section _cs;

#pragma endregion variables

#pragma region bar_warcher handlers

		void _stub(const tick& crr_tick, const bar& last_bar){

			return;
		}

		void _on_new_min_bar(const tick& crr_tick, const bar& last_bar){

			//lock for _run_sd_min & _sd_multiplier
			//concurrency::critical_section::scoped_lock::scoped_lock(_cs);

			if (_crr_hour >= _start_close_hour - 1)
				_run_sd_min.push(crr_tick.last - last_bar.open);

			if (_crr_hour >= _start_close_hour){

				// >= _start_close_hour, strat close positions and accept losses
				if (has_open_position()){
					
					double dev = _run_sd_min.get_value();
					if (dev != -1){

						double adjusted_dev = dev * _sd_multiplier;
						//update new _high/_low to keep it as deviation range of last_m_tick.last
						//	+-_exit_lev to adjust the _close_position_algo(i.e. ignore _exit_lev)
						if (_position.type == signal::SELL)
							_exit_lev = (std::min)((double)0, (_high + _entry_lev) - (last_bar.close - adjusted_dev));
						else
							_exit_lev = (std::min)((double)0, (last_bar.close + adjusted_dev) - (_low - _entry_lev));
					}

					//reduce sd range every minute to close position faster
					_sd_multiplier *= 0.98;
				}
			}
		}

		void _on_new_hour_bar(const tick& crr_tick, const bar& last_bar){

			_risk.push_return(crr_tick.last - last_bar.open);

			_crr_hour = crr_tick.time.time_of_day().hours();

			//new day
			if (_crr_hour == 0){
			
				//reset parameter at begining of the day
				_high = 0;
				_low = 999999;
				_entry_lev = 0;
				_exit_lev = _init_exit_lev;

				//lock for _run_sd_min & _sd_multiplier
				//concurrency::critical_section::scoped_lock::scoped_lock(_cs);

				_run_sd_min.reset();
				_sd_multiplier = 3;

				_risk.reset();

				_is_skip_day = false;
			}
		}

#pragma endregion bar_warcher handlers

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {
		
			if (crr_tick.bid >= _high + _entry_lev){

				_open_position(crr_tick, signal::SELL);
				return signal::SELL;
			}
			
			if (crr_tick.ask <= _low - _entry_lev){

				_open_position(crr_tick, signal::BUY);
				return signal::BUY;
			}

			return signal::NONE;
		}

		int _close_position_algo(const tick& crr_tick, position& close_pos, 
			const double stop_loss) override{

			bool is_stop_out = algo::_is_stop_out(crr_tick, stop_loss);

			if (is_stop_out ||
				(_position.type == signal::SELL && crr_tick.ask <= (_high + _entry_lev - _exit_lev))
				|| (_position.type == signal::BUY && crr_tick.bid >= (_low - _entry_lev + _exit_lev))){

				_position.close_tick = crr_tick;
				close_pos = _position;
				_delete_position();

				//extend entry level each time after a buy/sell signal
				//_entry_lev += _risk.get_risk();
				_entry_lev += _init_exit_lev * 1.5;

				if (is_stop_out){

					_is_skip_day = true;

					LOG("stoped out at " << crr_tick.time << ". no entry for the rest of the day.")
				}

				return 1;
			}

			return 0;
		}

	public:

#pragma region constructors

		algo_dayrange(const string symbol, int complete_hour, double exit_lev) : algo_bar(symbol),
			_complete_hour(complete_hour), _init_exit_lev(exit_lev),
			_run_sd_min(60), _risk(10){

			_attach_watcher(bar_watcher(bar_interval::HOUR, boost::bind(&algo_dayrange::_on_new_hour_bar, this, _1, _2)));
			_attach_watcher(bar_watcher(bar_interval::MIN, boost::bind(&algo_dayrange::_on_new_min_bar, this, _1, _2)));
			_attach_watcher(bar_watcher(bar_interval::SEC_20, boost::bind(&algo_dayrange::_stub, this, _1, _2)));
		};
		
		/// Destructor
		~algo_dayrange(){};

#pragma endregion

		signal process_tick(const tick& crr_tick, position& close_pos, double& risk_lev,
			const double stop_loss = -1, const bool ignore = false) override{
				
			_process_bar_tick(crr_tick);

			risk_lev = _risk.get_risk();

			if (_crr_hour < _complete_hour){

				//close all positions for previous day
				if (has_open_position() && _crr_hour == 0){

					_position.close_tick = crr_tick;
					close_pos = _position;
					_delete_position();
				}

				if (crr_tick.last > _high){

					_high = crr_tick.last;
				}
				else if (crr_tick.last < _low){

					_low = crr_tick.last;
				}
			}
			else if (_crr_hour < _start_close_hour){
			
				if (has_open_position()){
					
					//check close every 20 sec
					if (_is_on_new_bar_tick(bar_interval::SEC_20)){

						_close_position_algo(crr_tick, close_pos, stop_loss);
					}
				}
				else if (!ignore && _crr_hour <= _last_entry_hour && !_is_skip_day){

					return _get_signal_algo(crr_tick);
				}
			}	
			else{
				
				//every minute the exit_lev is reset based on current deviation in method _on_new_min_bar
				if (has_open_position()){

					_close_position_algo(crr_tick, close_pos, stop_loss);
				}
			}

			return signal::NONE;
		}
	
#ifndef MQL5_RELEASE

#pragma region optimizable_algo members

#define OPTI_PARAMS_DAYRANGE int, double

		typedef std::pair<double, std::tuple<OPTI_PARAMS_DAYRANGE>> CITIZEN_TYPE;

		std::tuple<OPTI_PARAMS_DAYRANGE> get_random_citizen(){

			return std::make_tuple(
				_rand_from_range(12, 20),
				_rand_from_range(2, 50) * 0.00010
				);
		}

		concurrency::concurrent_vector<CITIZEN_TYPE> init_optimization_population(int population_size) override{

			concurrency::concurrent_vector<CITIZEN_TYPE> population;

			//add current parameters
			population.push_back(std::make_pair(0,
				std::make_tuple(
				_complete_hour,
				_exit_lev
				)));

			for (int i = 0; i < population_size - 1; i++){

				CITIZEN_TYPE citizen =
					std::make_pair<double, std::tuple<OPTI_PARAMS_DAYRANGE>>(0, get_random_citizen());
				population.push_back(citizen);
			}

			return population;
		}

		std::shared_ptr<algo> get_optimizable_algo(std::tuple<OPTI_PARAMS_DAYRANGE> params) override{

			std::shared_ptr<algo> casted_ret = std::make_shared<algo_dayrange>(
				_symbol, std::get<0>(params), std::get<1>(params));

			return casted_ret;
		}

		std::tuple<OPTI_PARAMS_DAYRANGE> mate(
			const std::tuple<OPTI_PARAMS_DAYRANGE>& i, const std::tuple<OPTI_PARAMS_DAYRANGE>& j) override{

			return std::make_tuple(
				std::get<0>(i),
				std::get<1>(j)
				);
		}

		std::tuple<OPTI_PARAMS_DAYRANGE> mutate(std::tuple<OPTI_PARAMS_DAYRANGE> params) override{

			std::tuple<OPTI_PARAMS_DAYRANGE> ret = get_random_citizen();

			int keep = rand() % 3;
			switch (keep){

			case 0:
				std::get<0>(ret) = std::get<0>(params);
				break;

			case 1:
				std::get<1>(ret) = std::get<1>(params);
				break;
			}

			return ret;
		}

		string print_params(std::tuple<OPTI_PARAMS_DAYRANGE> params) override{

			return static_cast<std::ostringstream&>(std::ostringstream().flush() <<
				std::get<0>(params) << "," << std::get<1>(params)).str();
		}

#pragma endregion

#endif MQL5_RELEASE
	};
}

#endif