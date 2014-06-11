/*
idea principle: price tend to move dramatically after a ecnomic event and revert to mean after a cool down period
implementation: fixed observe period to observe the volatitly and fixed hold period to exit
*/

#ifndef _STRAT_EVENT_LONG_SHORT
#define _STRAT_EVENT_LONG_SHORT

#include "tick.h"
#include "position.h"
#include "event_algo.h"
#include "optimizer/optimizable_algo_genetic.h"

#include <vector>
#include <queue>

#include <boost/date_time.hpp>

#include <concurrent_vector.h>

using std::string;

namespace strat{

	class event_long_short : public event_algo,
		public optimizable_algo_genetic<size_t, size_t, double>{

	private:

		const bool _is_anti_trend;

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {

			signal ret_sig = signal::NONE;

			//assumption: only one event exist at any particular time(min), 
			//	therefore only one signal will be returned here. ie. not signal conflict
			if (!_obser_tick_q.empty()){

				//only one position allow to open at anytime i.e. no hedging
				if (_positions.size() > 0) return signal::NONE;

				tick front_tick = _obser_tick_q.front();
				if (crr_tick.time_stamp >= front_tick.time_stamp + boost::posix_time::minutes(_obser_win)){

					if (crr_tick.last >= front_tick.last + _run_sd){

						ret_sig = _is_anti_trend ? signal::SELL : signal::BUY;
						_add_position(crr_tick, ret_sig, front_tick);
					}
					else if (crr_tick.last <= front_tick.last - _run_sd){

						ret_sig = _is_anti_trend ? signal::BUY : signal::SELL;
						_add_position(crr_tick, ret_sig, front_tick);
					}

					_pop_obser_tick_queue();
				}
			}

			return ret_sig;
		}

		//duplicate code as event_algo_ma
		int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss) override{

			for (std::list<position>::iterator it = _positions.begin(); it != _positions.end();){

				bool is_stop_out = stop_loss != -1 && (it->open_tick.last - crr_tick.last) * it->type > stop_loss;

				if (is_stop_out || 
					crr_tick.time_stamp >= it->open_tick.time_stamp + boost::posix_time::minutes(_hold_win))
				{
					it->close_tick = crr_tick;
					close_pos = *it;
					it = _delete_position(it);
				}
				else
					++it;
			}

			return 1;
		}

	public:

#pragma region constructors

		event_long_short(const string s_base, const string s_quote,
			string event_f_path, size_t obser_win, size_t hold_win, double run_sd, 
			bool is_anti_trend = true) :
			event_algo(s_base, s_quote, event_f_path, obser_win, hold_win, run_sd),
			_is_anti_trend(is_anti_trend){		};


		/// Destructor
		~event_long_short(){};

#pragma endregion

		void reset_params(size_t obser_win, size_t hold_win, double run_sd){
		
			_obser_win = obser_win;
			_hold_win = hold_win;
			_run_sd = run_sd;

			LOG("reset param obser:" << _obser_win << " hold:" << _hold_win << " sd:" << _run_sd);
		}
		

#pragma region optimizable_algo members

		typedef std::pair<double, std::tuple<size_t, size_t, double>> CITIZEN_TYPE;

		std::tuple<size_t, size_t, double> get_random_citizen(){

			size_t obser = _rand_from_range(0, 60);
			return std::make_tuple(
				obser,
				_rand_from_range(obser, 360),
				_rand_from_range(1, 5) * 0.00015
				);
		}

		concurrency::concurrent_vector<CITIZEN_TYPE> init_optimization_population(int population_size) override{

			concurrency::concurrent_vector<CITIZEN_TYPE> population;

			//add current parameters
			population.push_back(std::make_pair(0, 
				std::make_tuple(
					_obser_win,
					_hold_win,
					_run_sd
				)));

			for (int i = 0; i < population_size - 1; i++){

				CITIZEN_TYPE citizen = 
					std::make_pair<double, std::tuple<size_t, size_t, double>>(0, get_random_citizen());
				population.push_back(citizen);
			}

			return population;
		}

		std::shared_ptr<algo> get_optimizable_algo(std::tuple<size_t, size_t, double> params) override{

			event_long_short* ret_algo = new event_long_short(_s_base, _s_quote,
				"", std::get<0>(params), std::get<1>(params), std::get<2>(params));

			std::queue<boost::posix_time::ptime> event_q(get_event_queue());

			//disable logging for open and close position/oberv
			ret_algo->toggle_log_switch();
			
			ret_algo->load_event(event_q);

			std::shared_ptr<algo> casted_ret = std::make_shared<event_long_short>(*ret_algo);

			return casted_ret;
		}

		std::tuple<size_t, size_t, double> mate(
			const std::tuple<size_t, size_t, double>& i, const std::tuple<size_t, size_t, double>& j) override{
		
			return std::make_tuple(
				std::get<0>(i),
				std::get<1>(j),
				std::get<2>(i)
				);
		}

		std::tuple<size_t, size_t, double> mutate(std::tuple<size_t, size_t, double> params) override{
		
			std::tuple<size_t, size_t, double> ret = get_random_citizen();

			int keep = rand() % 3 ;
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

		string print_params(std::tuple<size_t, size_t, double> params) override{
			
			return static_cast<std::ostringstream&>(std::ostringstream().flush() <<
				"obser:" << std::get<0>(params) << " hold:" << std::get<1>(params) << " sd:" << std::get<2>(params)
				).str();
		}

		void remove_non_important_ticks(std::vector<tick>& ticks) override{

			LOG("optimizer deleting non important sample ticks");

			std::queue<boost::posix_time::ptime> temp_event_q(get_event_queue());
			boost::posix_time::ptime next_event_t = temp_event_q.front();
			boost::posix_time::ptime last_event_t = temp_event_q.front();

			for (std::vector<tick>::iterator it = ticks.begin(); it != ticks.end();){

				if (next_event_t > it->time_stamp &&
					last_event_t + boost::posix_time::hours(7) < it->time_stamp){

					it = ticks.erase(it);
				}
				else {
					
					if (next_event_t < it->time_stamp){

						last_event_t = next_event_t;
						if (!temp_event_q.empty()){

							temp_event_q.pop();

							if(!temp_event_q.empty())
								next_event_t = temp_event_q.front();
						}
					}

					++it;
				}
			}
		}

#pragma endregion
	};
}

#endif