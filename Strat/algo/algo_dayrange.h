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

#include <vector>
#include <algorithm>

#include <boost/date_time.hpp>

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
		public optimizable_algo_genetic < int, double, double, double > {
#endif MQL5_RELEASE

	private:

		double _high;
		double _low;

		boost::posix_time::ptime::date_type _current_day;
		bool _is_skip_day;
		
		const int _complete_hour;
		const double _entry_lev;
		const double _exit_lev;
		
		const int _last_entry_hour = 21;
		const int _start_close_hour = 22; //this as to be at least _last_entry_hour + 2 to allow collect deviation
		
		const double _extend_exit_lev_factor;

		sd _run_sd;
		tick _last_m_tick;
		double _dev_factor;

		//return 1. sd changed, 0 unchanged
		int _push_run_sd(const tick& crr_tick){
		
			if (_last_m_tick.last == -1)
				_last_m_tick = crr_tick;
			else{

				if (crr_tick.time_stamp.time_of_day().minutes() !=
					_last_m_tick.time_stamp.time_of_day().minutes()){

					_run_sd.push(crr_tick.last - _last_m_tick.last);
					_last_m_tick = crr_tick;

					return 1;
				}
			}

			return 0;
		}

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {

			signal ret_sig = signal::NONE;
			
			if (crr_tick.bid >= _high + _entry_lev){

				ret_sig = signal::SELL;
				_add_position(crr_tick, ret_sig);
			}

			if (crr_tick.ask <= _low - _entry_lev){

				ret_sig = signal::BUY;
				_add_position(crr_tick, ret_sig);
			}

			return ret_sig;
		}

		int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss) override{

			double closeRate = _position.type == signal::SELL ? crr_tick.ask : crr_tick.bid;

			bool is_stop_out = stop_loss != -1 && (_position.open_rate - closeRate) * _position.type > stop_loss;

			if (is_stop_out || 
				(_position.type == signal::SELL && crr_tick.ask <= (_high - _exit_lev))
				|| (_position.type == signal::BUY && crr_tick.bid >= (_low + _exit_lev))){

				_position.close_tick = crr_tick;
				close_pos = _position;
				_delete_position();

				//extend entry level each time after a buy/sell signal
				_low -= _exit_lev * _extend_exit_lev_factor;
				_high += _exit_lev * _extend_exit_lev_factor;

				if (is_stop_out){

					_is_skip_day = true;

#ifdef MQL5_RELEASE
					LOG("stoped out at " << crr_tick.time_stamp << ". no entry for the rest of the day.")
#endif MQL5_RELEASE
				}

				return 1;
			}

			return 0;
		}

		void _on_bar_tick(const tick& crr_tick, position& close_pos, double stop_loss = -1) override{

			_close_position_algo(crr_tick, close_pos, stop_loss);
		}

	public:

#pragma region constructors

		algo_dayrange(const string s_base, const string s_quote, 
			int complete_hour, double entry_lev, double exit_lev, double extend_factor = 1.5) :
			algo_bar(s_base, s_quote, bar_interval::SEC_15),
			_complete_hour(complete_hour), _entry_lev(entry_lev), _exit_lev(exit_lev),
			_run_sd(60), _extend_exit_lev_factor(extend_factor){
		
			boost::posix_time::ptime day = boost::posix_time::min_date_time;
			_current_day = day.date();
		};
		
		/// Destructor
		~algo_dayrange(){};

#pragma endregion

		signal process_tick(const tick& crr_tick, position& close_pos, double stop_loss = -1) override{

			int crr_hour = crr_tick.time_stamp.time_of_day().hours();
			boost::posix_time::ptime::date_type crr_day = crr_tick.time_stamp.date();

			if (crr_day > _current_day)
			{
				//close all positions for previous day
				if (has_open_position()){

					_position.close_tick = crr_tick;
					close_pos = _position;
					_delete_position();
				}

				_high = 0;
				_low = 999999;
				_current_day = crr_day;

				_run_sd.reset();
				_last_m_tick.last = -1;
				_dev_factor = 5;

				_is_skip_day = false;
			}
				
			if (crr_hour < _complete_hour){

				if (crr_tick.last > _high){

					_high = crr_tick.last;
				}
				else if (crr_tick.last < _low){

					_low = crr_tick.last;
				}
			}
			else if (crr_hour < _start_close_hour){
			
				if (has_open_position()){

					if (crr_hour >= _start_close_hour - 1){

						_push_run_sd(crr_tick);
					}

					//_close_position_algo(crr_tick, close_pos, stop_loss);

					_process_bar_tick(crr_tick, close_pos, stop_loss);
				}
				else if (crr_hour <= _last_entry_hour && !_is_skip_day){
				
					return _get_signal_algo(crr_tick);
				}
			}	
			else{

				// >= _start_close_hour, strat close positions and accept losses
				if (has_open_position()){
					
					//update new _high/_low every minute
					if (_push_run_sd(crr_tick)){

						double dev = _run_sd.get_value();

						if (dev != -1){

							//update new _high/_low to keep it as deviation range of last_m_tick.last
							//	+-_exit_lev to adjust the _close_position_algo(i.e. ignore _exit_lev)
							_high = (std::max)(_high, _last_m_tick.last - dev * _dev_factor + _exit_lev);
							_low = (std::min)(_low, _last_m_tick.last + dev * _dev_factor - _exit_lev);
						}

						//reduce sd range every minute to close position faster
						_dev_factor = _dev_factor * 0.98;
					}

					_close_position_algo(crr_tick, close_pos, stop_loss);
				}
			}

			return signal::NONE;
		}
	
#ifndef MQL5_RELEASE

#pragma region optimizable_algo members

#ifndef OPTI_PARAMS
#define OPTI_PARAMS int, double, double, double
#endif

		typedef std::pair<double, std::tuple<OPTI_PARAMS>> CITIZEN_TYPE;

		std::tuple<OPTI_PARAMS> get_random_citizen(){

			return std::make_tuple(
				_rand_from_range(12, 20),
				_rand_from_range(0, 10) * 0.00010, //0.00010
				_rand_from_range(2, 50) * 0.00010,
				_rand_from_range(2, 10) * 0.5
				);
		}

		concurrency::concurrent_vector<CITIZEN_TYPE> init_optimization_population(int population_size) override{

			concurrency::concurrent_vector<CITIZEN_TYPE> population;

			//add current parameters
			population.push_back(std::make_pair(0,
				std::make_tuple(
				_complete_hour,
				_entry_lev,
				_exit_lev,
				_extend_exit_lev_factor
				)));

			for (int i = 0; i < population_size - 1; i++){

				CITIZEN_TYPE citizen =
					std::make_pair<double, std::tuple<OPTI_PARAMS>>(0, get_random_citizen());
				population.push_back(citizen);
			}

			return population;
		}

		std::shared_ptr<algo> get_optimizable_algo(std::tuple<OPTI_PARAMS> params) override{

			algo_dayrange* ret_algo = new algo_dayrange(_s_base, _s_quote,
				std::get<0>(params), std::get<1>(params), std::get<2>(params), std::get<3>(params));

			//disable logging for open and close position/oberv
			ret_algo->toggle_log_switch();

			std::shared_ptr<algo> casted_ret = std::make_shared<algo_dayrange>(*ret_algo);

			return casted_ret;
		}

		std::tuple<OPTI_PARAMS> mate(
			const std::tuple<OPTI_PARAMS>& i, const std::tuple<OPTI_PARAMS>& j) override{

			return std::make_tuple(
				std::get<0>(i),
				std::get<1>(i),
				std::get<2>(j),
				std::get<3>(j)
				);
		}

		std::tuple<OPTI_PARAMS> mutate(std::tuple<OPTI_PARAMS> params) override{

			std::tuple<OPTI_PARAMS> ret = get_random_citizen();

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

		string print_params(std::tuple<OPTI_PARAMS> params) override{

			return static_cast<std::ostringstream&>(std::ostringstream().flush() <<
				std::get<0>(params) << "," << std::get<1>(params) <<
				"," << std::get<2>(params) << "," << std::get<3>(params)).str();
		}

#pragma endregion

#endif MQL5_RELEASE
	};
}

#endif