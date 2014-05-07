

#ifndef _STRAT_EVENT_LONG_SHORT
#define _STRAT_EVENT_LONG_SHORT

#include "tick.h"
#include "position.h"
#include "event_algo.h"

#include <vector>
#include <queue>

#include <boost/date_time.hpp>

using std::string;

namespace strat{

	class event_long_short : public event_algo{

	private:

		const bool _is_anti_trend;

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {

			signal ret_sig = signal::NONE;

			//assumption: only one event exist at any particular time(min), 
			//	therefore only one signal will be returned here. ie. not signal conflict
			if (!_obser_tick_q.empty()){

				tick front_tick = _obser_tick_q.front();
				if (crr_tick.time_stamp >= front_tick.time_stamp + boost::posix_time::minutes(_obser_win)){

					if (crr_tick.close >= front_tick.close + _run_sd){

						ret_sig = _is_anti_trend ? signal::SELL : signal::BUY;
						_add_position(crr_tick, ret_sig, front_tick);
					}
					else if (crr_tick.close <= front_tick.close - _run_sd){

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

				bool is_stop_out = stop_loss != -1 && (it->open_tick.close - crr_tick.close) * it->type > stop_loss;

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

		event_long_short(const string s_base, const string symbol_target,
			string event_f_path, size_t obser_win, size_t hold_win, double run_sd, 
			bool is_anti_trend = true) :
			event_algo(s_base, symbol_target, event_f_path, obser_win, hold_win, run_sd),
			_is_anti_trend(is_anti_trend){		};

#pragma endregion

		/// Destructor
		~event_long_short(){};
	};
}

#endif