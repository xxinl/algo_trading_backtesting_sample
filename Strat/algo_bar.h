
#ifndef _STRAT_ALGO_BAR
#define _STRAT_ALGO_BAR

#include "algo.h"
#include "bar.h"

#include <string>
#include <functional>

#include <boost/date_time.hpp>

using std::string;

namespace strat{

	class algo_bar : public algo {
		
	private:

		boost::posix_time::ptime _epoch;

	protected:

		bar_interval _bar_type;
		bar _crr_bar;
		size_t _bar_id;

		virtual void _on_bar_tick(const tick& crr_tick, position& close_pos, double stop_loss) = 0;

		void _process_bar_tick(const tick& crr_tick, position& close_pos, double stop_loss){

			_crr_bar.close = crr_tick.last;

			boost::posix_time::time_duration duration = crr_tick.time_stamp - _epoch;
			long total_sec = duration.total_seconds();
			size_t new_bar_id = total_sec / _bar_type;

			if (new_bar_id != _bar_id){

				_bar_id = new_bar_id;

				_crr_bar.high = crr_tick.last;
				_crr_bar.low = crr_tick.last;
				_crr_bar.volume = crr_tick.volume;

				_on_bar_tick(crr_tick, close_pos, stop_loss);
			}
			else{

				if (crr_tick.last > _crr_bar.high) _crr_bar.high = crr_tick.last;
				if (crr_tick.last < _crr_bar.low) _crr_bar.low = crr_tick.last;
				_crr_bar.volume += crr_tick.volume;
			}
		}

	public:

		virtual ~algo_bar() {}

		algo_bar(string s_base, string s_quote, bar_interval bar_type) :
			algo(s_base, s_quote){
		
			_position.type = signal::NONE;
			_bar_type = bar_type;
			_epoch = boost::posix_time::ptime(boost::gregorian::date(2000, 1, 1));
			_bar_id = 0;
		};
	};
}

#endif