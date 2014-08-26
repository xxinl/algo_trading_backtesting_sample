
#ifndef _STRAT_BAR_WATCHER
#define _STRAT_BAR_WATCHER

#include "bar.h"

#include <string>

#include <boost/date_time.hpp>
#include <boost/signals2.hpp>

using std::string;

namespace strat{

	class bar_watcher{
		
	private:

		typedef boost::signals2::signal<void(const tick&, const bar& last_bar)> SIGNAL_TYPE;

		boost::posix_time::ptime _epoch;

		size_t _crr_bar_id;
		bool _is_on_new_bar_tick;
		std::shared_ptr<SIGNAL_TYPE> _on_new_bar_handler_ptr;

	public:

		size_t interval;
		bar crr_bar;
		bar last_bar;

		bar_watcher(bar_interval interval, const SIGNAL_TYPE::slot_type& on_new_bar_handler)
			:interval(interval){
		
			_on_new_bar_handler_ptr = std::make_shared<SIGNAL_TYPE>();
			_on_new_bar_handler_ptr->connect(on_new_bar_handler);

			_epoch = boost::posix_time::ptime(boost::gregorian::date(2000, 1, 1));
			_crr_bar_id = 0;
			_is_on_new_bar_tick = false;
		}

		void on_tick(const tick& crr_tick){

			crr_bar.close = crr_tick.last;

			boost::posix_time::time_duration duration = crr_tick.time - _epoch;
			long total_sec = duration.total_seconds();
			size_t new_bar_id = total_sec / interval;

			if (new_bar_id != _crr_bar_id){

				last_bar = crr_bar;

				_crr_bar_id = new_bar_id;
				_is_on_new_bar_tick = true;

				crr_bar.time_stamp = crr_tick.time;
				crr_bar.high = crr_tick.last;
				crr_bar.low = crr_tick.last;
				crr_bar.volume = crr_tick.volume;
				crr_bar.open = crr_tick.last;

				if (!_on_new_bar_handler_ptr->empty())
					(*_on_new_bar_handler_ptr)(crr_tick, last_bar);
			}
			else{

				_is_on_new_bar_tick = false;

				if (crr_tick.last > crr_bar.high) crr_bar.high = crr_tick.last;
				if (crr_tick.last < crr_bar.low) crr_bar.low = crr_tick.last;

				crr_bar.volume += crr_tick.volume;
			}
		}

		bool is_on_new_bar_tick() const{
		
			return _is_on_new_bar_tick;
		}
	};
}

#endif