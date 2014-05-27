
#ifdef STRAT_EXPORTS

#include <oaidl.h>
#include <comutil.h>

#include "algo.h"
#include "algo\event\event_algo_ma.h"
#include "algo\event\event_long_short.h"
#include "logger.h"
#include "optimizer\optimizable_algo_genetic.h"
#include "optimizer\optimizer_genetic.h"

#include <string>
#include <boost/date_time.hpp>

#include <exception>

using std::string;


string convert_wchar_to_string(wchar_t* wchar){

	std::wstring w_base_str = std::wstring(wchar);
	return string(w_base_str.begin(), w_base_str.end());
}


extern "C"	__declspec(dllexport)
size_t get_algo(wchar_t* base, wchar_t* quote, wchar_t* path){

	strat::event_algo* ret_p = nullptr;

	try{
	
		//ret_p = new strat::event_algo_ma(
		//	convert_wchar_to_string(base), convert_wchar_to_string(quote), convert_wchar_to_string(path),
		//	7, 45, 0.0003, 25, 270, 0.7);

		ret_p = new strat::event_long_short(
			convert_wchar_to_string(base), convert_wchar_to_string(quote), convert_wchar_to_string(path),
			7, 45, 0.0003);
	}
	catch(std::exception& e){
		
		LOG_SEV("get_algo error: " << e.what(), logger::error);
	}

	size_t ret_add = reinterpret_cast<size_t>(ret_p);

	LOG_SEV("get_algo return pointer adreess:" << ret_add, logger::debug);

	return ret_add;
}

extern "C"	__declspec(dllexport)
int delete_algo(size_t algo_add){

	strat::event_algo_ma* algo_p = reinterpret_cast<strat::event_algo_ma*>(algo_add);
	delete(algo_p);

	return 1;
}

//time eg.2013-11-25 23:50:00.000
extern "C"	__declspec(dllexport)
int process_tick(size_t algo_addr, wchar_t* time, double ask, double bid, double close, double stop_loss){

	string time_str = convert_wchar_to_string(time);

	LOG_SEV("process_tick algo:" << algo_addr << " time:" << time_str
		<< " close:" << close << " sl:" << stop_loss, logger::debug);

	strat::signal sig = strat::signal::NONE;

	try{

		strat::tick tick;
		tick.time_stamp = boost::posix_time::time_from_string(time_str);
		tick.close = close;

		LOG_SEV("process_tick casted time:" << tick.time_stamp, logger::debug);

		strat::position close_pos;
		strat::algo* algo_p = reinterpret_cast<strat::algo*>(algo_addr);
		sig = algo_p->process_tick(tick, close_pos, stop_loss);
	}
	catch (std::exception& e){

		LOG_SEV("process_tick error: " << e.what(), logger::error);
	}

	if (sig != strat::signal::NONE)
		LOG("return signal:" << sig);

	return sig;
}

__declspec(dllexport)
void optimize(size_t algo_addr){

	typedef strat::optimizable_algo_genetic<size_t, size_t, double> OPTIMIZER_TYPE;

	OPTIMIZER_TYPE* algo_p =reinterpret_cast<OPTIMIZER_TYPE*>(algo_addr);
	strat::optimizer_genetic<size_t, size_t, double> optimizer(algo_p);
	std::pair<double, std::tuple<size_t, size_t, double>> opti_params = optimizer.optimize();

	//todo save opti_params
}

#endif
