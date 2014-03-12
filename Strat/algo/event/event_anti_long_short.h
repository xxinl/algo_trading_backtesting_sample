

#ifndef _STRAT_EVENT_LONG_SHORT
#define _STRAT_EVENT_LONG_SHORT

#include "../../tick.h"
#include "../../position.h"
#include "event_trading_algorithm.h"

#include <vector>
#include <queue>

#include <boost/date_time.hpp>

namespace strat{

	class event_anti_long_short : public event_trading_algorithm{
	private:

	protected:
		signal _get_signal_algo(const tick &crr_tick);
		int _close_position_algo(const tick &crr_tick, std::vector<position> &close_pos);

	public:
		/// Constructor 
		event_anti_long_short(const std::string symbol_base, const std::string symbol_target,
			const char *event_f_path, int obser_win, int hold_win, double run_sd) :
			event_trading_algorithm(symbol_base, symbol_target, event_f_path, obser_win, hold_win, run_sd){};

		/// Destructor
		~event_anti_long_short(){};

		//TODO copy constructor
	};
}

#endif