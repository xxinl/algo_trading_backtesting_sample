

#include <oaidl.h>
#include <comutil.h>

#include "algo.h"
#include "algo\event\event_algo_ma.h"

#include <boost/date_time.hpp>


extern "C"	__declspec(dllexport)
strat::event_algo_ma* get_algo(char* base, char* quote, char* path){

	return new strat::event_algo_ma(base, quote, path,
		7, 45, 0.0003, 25, 270, 0.7);
}

extern "C"	__declspec(dllexport)
int delete_algo(strat::algo *algo){

	delete(algo);

	return 1;
}

//time eg.2013-11-25 23:50:00.000
extern "C"	__declspec(dllexport)
int process_tick(strat::event_algo_ma* algo, char* time, double close, double stop_loss){

	strat::tick tick;
	tick.time_stamp = boost::posix_time::time_from_string(time);
	tick.close = close;

	strat::position close_pos;
	strat::signal sig = algo->process_tick(tick, close_pos, stop_loss);

	return sig;
}
