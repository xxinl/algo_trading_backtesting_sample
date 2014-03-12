

#ifndef _STRAT_POSITION
#define _STRAT_POSITION

#include "tick.h"

namespace strat{

	enum signal{
		SELL = -1,
		NONE = 0,
		BUY =1
	};

	class position{
	private:
	
	public:
		int id;
		tick open_tick;
		tick close_tick;
		strat::signal type;

		/// Constructor 
		position(){};

		/// Destructor
		~position(){};

	};

}

#endif
