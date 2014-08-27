
#ifndef _STRAT_BAR
#define _STRAT_BAR

#include <boost/date_time.hpp>

namespace strat{
	
	//in seconds
	enum bar_interval{

		SEC_15 = 15,
		SEC_20 = 20,
		MIN = 60,
		HOUR = 3600,
		DAY = 86400
	};

	struct bar {

		boost::posix_time::ptime time_stamp;

		double open;
		double close;
		double high;
		double low;
		size_t volume;

		bar(){
		
			open = -1;
			close = -1;
			high = -1;
			low = 9999;
			volume = 0;
			time_stamp = boost::posix_time::min_date_time;
		}
	};
}

#endif