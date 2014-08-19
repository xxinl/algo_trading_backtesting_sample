
#ifndef _STRAT_TICK
#define _STRAT_TICK

#include <boost/date_time.hpp>

namespace strat{

	struct tick {

		boost::posix_time::ptime time;
		double ask;
		double bid;
		double last;
		size_t volume;

		tick(){}

		tick(const boost::posix_time::ptime _time,
			const double _ask, const double _bid, const double _last,
			const size_t _volume)
			:time(_time), ask(_ask), bid(_bid), last(_last), volume(_volume){}
	};
}

#endif