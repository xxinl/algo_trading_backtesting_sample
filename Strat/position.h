

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

		double open_rate;

		position(){

			type = signal::NONE;
		}

		void open(const tick& _open_tick, const signal _type){

			open_tick = _open_tick;
			type = _type;
			open_rate = signal::BUY ? open_tick.ask : open_tick.bid;
		}

		void clear(){
		
			type = signal::NONE;
		}

		bool is_empty() const{
		
			return type == signal::NONE;
		}
	};

	//below is only used for event based algo which none are live
	struct event_position : position{

		tick obser_tick;
	};
}

#endif
