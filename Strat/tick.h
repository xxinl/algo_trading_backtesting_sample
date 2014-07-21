
#ifndef _STRAT_TICK
#define _STRAT_TICK

#include <boost/date_time.hpp>

namespace strat{

	struct tick {

		boost::posix_time::ptime time_stamp;
		double ask;//high
		double bid;//low
		double last;//close
		size_t volume;
	};
}

#endif