

#include <oaidl.h>
#include <comutil.h>

#include "algo.h"
#include "algo\event\event_long_short.h"


extern "C"	__declspec(dllexport)
//strat::event_long_short* connect(wchar_t *algo_name){
//	if (algo_name == L"event_long_short"){
//		return new strat::event_long_short();
//	}
//}

extern "C"	__declspec(dllexport)
int disconnect(strat::algo *algo){
	delete(algo);

	return 1;
}

extern "C"	__declspec(dllexport)
int process_tick(strat::algo *algo, long time, double close){
	return 1;
}
