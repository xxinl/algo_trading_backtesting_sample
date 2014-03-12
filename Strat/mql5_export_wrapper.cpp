

#include <oaidl.h>
#include <comutil.h>

#include "trading_algorithm.h"
#include "algo\event\event_anti_long_short.h"


extern "C"	__declspec(dllexport)
//strat::event_anti_long_short* connect(wchar_t *algo_name){
//	if (algo_name == L"event_anti_long_short"){
//		return new strat::event_anti_long_short();
//	}
//}

extern "C"	__declspec(dllexport)
int disconnect(strat::trading_algorithm *algo){
	delete(algo);

	return 1;
}

extern "C"	__declspec(dllexport)
int process_tick(strat::trading_algorithm *algo, long time, double close){
	return 1;
}
