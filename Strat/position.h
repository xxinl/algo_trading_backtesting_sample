

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

		tick open_tick;
		tick close_tick;
		signal type;

		position(){
			type = signal::NONE;
		}

		~position(){}
	};

	struct event_position : position{

		tick obser_tick;
	};
}

#endif
