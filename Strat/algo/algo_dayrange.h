/*
idea principle: daily volatility range complete before the end of the day. 
									trade pullback after the complete
implementation: 
*/


#ifndef _STRAT_DAYRANGE
#define _STRAT_DAYRANGE

#include "tick.h"
#include "position.h"
#include "algo.h"
#include "optimizer/optimizable_algo_genetic.h"

#include <vector>
#include <cmath>

#include <boost/date_time.hpp>

#include <concurrent_vector.h>

using std::string;

namespace strat{

	class algo_dayrange : public algo,
		public optimizable_algo_genetic<int, double, double>{

	private:

		double _high;
		double _low;
		boost::posix_time::ptime::date_type _current_day;
		int _complete_hour;
		double _entry_lev;
		double _exit_lev;

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {

			signal ret_sig = signal::NONE;
			
			if (crr_tick.last >= _high + _entry_lev){

				ret_sig = signal::SELL;
				_add_position(crr_tick, ret_sig);
			}

			if (crr_tick.last <= _low - _entry_lev){

				ret_sig = signal::BUY;
				_add_position(crr_tick, ret_sig);
			}

			return ret_sig;
		}

		int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss) override{

			double closeRate = _position.type == signal::SELL ? crr_tick.ask : crr_tick.bid;

			bool is_stop_out = stop_loss != -1 && (_position.open_rate - closeRate) * _position.type > stop_loss;

			if (is_stop_out || 
				(_position.type == signal::SELL && crr_tick.ask <= (_position.open_rate - _exit_lev))
				|| (_position.type == signal::BUY && crr_tick.bid >= (_position.open_rate + _exit_lev))){

				_position.close_tick = crr_tick;
				close_pos = _position;
				_delete_position();

				return 1;
			}

			return 0;
		}

	public:

#pragma region constructors

		algo_dayrange(const string s_base, const string s_quote, 
			int complete_hour, double entry_lev, double exit_lev) :
			algo(s_base, s_quote), 
			_complete_hour(complete_hour), _entry_lev(entry_lev), _exit_lev(exit_lev){
		
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
				_high = 0;
				_low = 999999;
				_current_day = crr_day;
			}
					
			if (crr_tick.last > _high){
				
				_high = crr_tick.last;
			}
			else if (crr_tick.last < _low){
				
				_low = crr_tick.last;
			}

			if (crr_hour > _complete_hour && crr_hour < 22){
			
				if (has_open_position()){

					_close_position_algo(crr_tick, close_pos, stop_loss);
				}
				else{
				
					return _get_signal_algo(crr_tick);
				}
			}		
			else{
			
				//close all positions after 2200 hours
				if (has_open_position()){

					_position.close_tick = crr_tick;
					close_pos = _position;
					_delete_position();
				}
			}

			return signal::NONE;
		}
	
#pragma region optimizable_algo members

		typedef std::pair<double, std::tuple<int, double, double>> CITIZEN_TYPE;

		std::tuple<int, double, double> get_random_citizen(){

			return std::make_tuple(
				_rand_from_range(12, 20),
				_rand_from_range(0, 10) * 0.00010, //0.00010
				_rand_from_range(2, 50) * 0.00010
				);
		}

		concurrency::concurrent_vector<CITIZEN_TYPE> init_optimization_population(int population_size) override{

			concurrency::concurrent_vector<CITIZEN_TYPE> population;

			//add current parameters
			population.push_back(std::make_pair(0,
				std::make_tuple(
				_complete_hour,
				_entry_lev,
				_exit_lev
				)));

			for (int i = 0; i < population_size - 1; i++){

				CITIZEN_TYPE citizen =
					std::make_pair<double, std::tuple<int, double, double>>(0, get_random_citizen());
				population.push_back(citizen);
			}

			return population;
		}

		std::shared_ptr<algo> get_optimizable_algo(std::tuple<int, double, double> params) override{

			algo_dayrange* ret_algo = new algo_dayrange(_s_base, _s_quote,
				std::get<0>(params), std::get<1>(params), std::get<2>(params));

			//disable logging for open and close position/oberv
			ret_algo->toggle_log_switch();

			std::shared_ptr<algo> casted_ret = std::make_shared<algo_dayrange>(*ret_algo);

			return casted_ret;
		}

		std::tuple<int, double, double> mate(
			const std::tuple<int, double, double>& i, const std::tuple<int, double, double>& j) override{

			return std::make_tuple(
				std::get<0>(i),
				std::get<1>(j),
				std::get<2>(i)
				);
		}

		std::tuple<int, double, double> mutate(std::tuple<int, double, double> params) override{

			std::tuple<int, double, double> ret = get_random_citizen();

			int keep = rand() % 3;
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
			}

			return ret;
		}

		string print_params(std::tuple<int, double, double> params) override{

			return static_cast<std::ostringstream&>(std::ostringstream().flush() <<
				std::get<0>(params) << "," << std::get<1>(params) <<
				"," << std::get<2>(params)).str();
		}

#pragma endregion
	};
}

#endif