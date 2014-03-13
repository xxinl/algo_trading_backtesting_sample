
#include "event_anti_long_short.h"

namespace strat{

	signal event_anti_long_short::_get_signal_algo(const tick& crr_tick){
		
		signal ret_sig = signal::NONE;

		//assumption: only one event exist at any particular time(min), therefore only one signal will be returned here. ie. not signal conflict
		if (!_obser_tick_q.empty()){

			tick front_tick = _obser_tick_q.front();
			if (crr_tick.time_stamp >= front_tick.time_stamp + boost::posix_time::minutes(_obser_win)){

				if (crr_tick.close >= front_tick.close + _run_sd){

					ret_sig = signal::SELL;
					_add_position(crr_tick, ret_sig, front_tick);
				}
				else if (crr_tick.close <= front_tick.close - _run_sd){

					ret_sig = signal::BUY;
					_add_position(crr_tick, ret_sig, front_tick);
				}

				_pop_obser_tick_queue();
			}
		}

		return ret_sig;
	}

	int event_anti_long_short::_close_position_algo(const tick& crr_tick, position& close_pos){

		for (std::list<position>::iterator it = _positions.begin(); it != _positions.end();){

			if (crr_tick.time_stamp >= it->open_tick.time_stamp + boost::posix_time::minutes(_hold_win))
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
}