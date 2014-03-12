
#ifndef _STRAT_TICK
#define _STRAT_TICK

#include <boost/date_time.hpp>

namespace strat{

	class tick {
	private:

	public:
		boost::posix_time::ptime time_stamp;
		double close;

		/// Constructor 
		tick(){};

		/// Destructor
		~tick(){};

		//TODO copy constructor
	};
}

#endif