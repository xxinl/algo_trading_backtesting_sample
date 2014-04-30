
#ifdef STRAT_EXPORTS

#include <oaidl.h>
#include <comutil.h>

#include "algo.h"
#include "algo\event\event_algo_ma.h"
#include "logger.h"

#include <boost/date_time.hpp>

#include <exception>


extern "C"	__declspec(dllexport)
size_t get_algo(wchar_t* base, wchar_t* quote, wchar_t* path){

	std::wstring w_base_str = std::wstring(base);
	std::wstring w_quote_str = std::wstring(quote);
	std::wstring w_path_str = std::wstring(path);

	strat::event_algo_ma* ret_p = nullptr;

	try{
	
		ret_p = new strat::event_algo_ma(string(w_base_str.begin(), w_base_str.end()),
			string(w_quote_str.begin(), w_quote_str.end()),
			string(w_path_str.begin(), w_path_str.end()),
			7, 45, 0.0003, 25, 270, 0.7);
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
int process_tick(size_t algo_add, wchar_t* time, double close, double stop_loss){

	std::wstring w_time_str = std::wstring(time);

	LOG_SEV("process_tick algo:" << algo_add << " time:" << string(w_time_str.begin(), w_time_str.end())
		<< " close:" << close << " sl:" << stop_loss, logger::debug);

	strat::signal sig = strat::signal::NONE;

	try{

		strat::tick tick;
		tick.time_stamp = boost::posix_time::time_from_string(string(w_time_str.begin(), w_time_str.end()));
		tick.close = close;

		LOG_SEV("process_tick casted time:" << tick.time_stamp, logger::debug);

		strat::position close_pos;
		strat::event_algo_ma* algo_p = reinterpret_cast<strat::event_algo_ma*>(algo_add);
		strat::signal sig = algo_p->process_tick(tick, close_pos, stop_loss);
	}
	catch (std::exception& e){

		LOG_SEV("process_tick error: " << e.what(), logger::error);
	}

	return sig;
}

#endif
