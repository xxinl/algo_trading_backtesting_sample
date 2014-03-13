

#ifndef _STRAT_POSITION
#define _STRAT_POSITION

#include "tick.h"

namespace strat{

	enum signal{

		SELL = -1,
		NONE = 0,
		BUY =1
	};

	struct position{

		tick obser_tick;
		tick open_tick;
		tick close_tick;
		signal type;

		position(){
			type = signal::NONE;
		}
	};

}

#endif
