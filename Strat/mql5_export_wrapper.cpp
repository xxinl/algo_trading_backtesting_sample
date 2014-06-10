
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
#include <future>
#include <exception>
#include <sstream>

#include <boost/date_time.hpp>

using std::string;

logger::callback logger::on_callback = nullptr;

#pragma region _private members

string convert_wchar_to_string(const wchar_t* wchar){

	std::wstring w_base_str = std::wstring(wchar);
	return string(w_base_str.begin(), w_base_str.end());
}

template<typename T>
void write_ini(string section, T val){

	util::write_ini("c:/strat_ini/strat.ini", section, val);
}

template<typename T>
T read_ini(string section){

	return util::read_ini<T>("c:/strat_ini/strat.ini", section);
}

void optimize(size_t algo_addr, const wchar_t* hist_tick_path, 
	boost::posix_time::ptime start_date, boost::posix_time::ptime end_date,
	size_t max_iteration, size_t population_size){

	srand((unsigned int)time(NULL));

	//typedef strat::optimizable_algo_genetic<size_t, size_t, double> OPTIMIZER_TYPE;
	typedef strat::event_long_short OPTIMIZER_TYPE;

	strat::event_long_short* algo_p = reinterpret_cast<strat::event_long_short*>(algo_addr);
	strat::optimizer_genetic<size_t, size_t, double> optimizer(
		convert_wchar_to_string(hist_tick_path), algo_p, 0.2f, 0.4f, max_iteration, population_size);
	std::pair<double, std::tuple<size_t, size_t, double>> opti_params = optimizer.optimize(start_date, end_date);

	write_ini("OPTI_PARAM.OBSERV", std::get<0>(opti_params.second));
	write_ini("OPTI_PARAM.HOLD", std::get<1>(opti_params.second));
	write_ini("OPTI_PARAM.SD", std::get<2>(opti_params.second));
}

#pragma endregion


// todo free strings
// base/quote has to be lower case
extern "C"	__declspec(dllexport)
size_t get_algo(const wchar_t* base, const wchar_t* quote, const wchar_t* path, 
									logger::callback callback_handler){

	logger::on_callback = callback_handler;
	strat::event_algo* ret_p = nullptr;

	try{

		ret_p = new strat::event_long_short(
			convert_wchar_to_string(base), convert_wchar_to_string(quote), convert_wchar_to_string(path),
			7, 45, 0.0003);
	
		//ret_p = new strat::event_algo_ma(
		//	convert_wchar_to_string(base), convert_wchar_to_string(quote), convert_wchar_to_string(path),
		//	7, 45, 0.0003, 25, 270, 0.7);
	}
	catch(std::exception& e){
		
		LOG_SEV("get_algo error: " << e.what(), logger::error);
	}

	size_t ret_add = reinterpret_cast<size_t>(ret_p);

	LOG_SEV("get_algo return pointer adreess:" << ret_add, logger::debug);

	return ret_add;
}

extern "C"	__declspec(dllexport)
int delete_algo(size_t algo_addr){

	strat::algo* algo_p = reinterpret_cast<strat::algo*>(algo_addr);
	delete(algo_p);

	return 1;
}

//time eg.2013-11-25 23:50:00.000
extern "C"	__declspec(dllexport)
int process_tick(size_t algo_addr, const wchar_t* time, double ask, double bid, double last, size_t volume,
									double stop_loss, bool* is_close_pos, logger::callback callback_handler){

	logger::on_callback = callback_handler;

	*is_close_pos = false;

	string time_str = convert_wchar_to_string(time);
	strat::algo* algo_p = reinterpret_cast<strat::algo*>(algo_addr);

	LOG_SEV("process_tick algo:" << algo_addr << " time:" << time_str
		<< " last:" << last << " sl:" << stop_loss, logger::debug);
	//LOG_TICK(time_str, ask, bid, last, volume);
	
	strat::signal sig = strat::signal::NONE;
	strat::position close_pos;

	try{

		strat::tick tick;
		tick.time_stamp = boost::posix_time::time_from_string(time_str);
		tick.last = last;
		tick.ask = ask;
		tick.bid = bid;
		tick.volume = volume;

		LOG_SEV("process_tick casted time:" << tick.time_stamp, logger::debug);
		
		sig = algo_p->process_tick(tick, close_pos, stop_loss);
	}
	catch (std::exception& e){

		LOG_SEV("process_tick error: " << e.what(), logger::error);
	}

	if (sig != strat::signal::NONE)
		LOG("return signal: " << sig);

	//close position signal
	if (close_pos.type != strat::signal::NONE)
	{
		LOG_POSITION(close_pos);
		*is_close_pos = true;
	}

	return sig;
}

extern "C"	__declspec(dllexport)
void optimize(size_t algo_addr, const wchar_t* hist_tick_path, const wchar_t* start_date, const wchar_t* end_date,
								size_t max_iteration, size_t population_size,
								logger::callback callback_handler){

	logger::on_callback = callback_handler;
	
	string start_time_str = convert_wchar_to_string(start_date);
	string end_time_str = convert_wchar_to_string(end_date);

	boost::posix_time::ptime start_time = boost::posix_time::time_from_string(start_time_str);
	boost::posix_time::ptime end_time = boost::posix_time::time_from_string(end_time_str);

	try{

		optimize(algo_addr, hist_tick_path, start_time, end_time, max_iteration, population_size);
	}
	catch (std::exception& e){

		LOG_SEV("optimize error: " << e.what(), logger::error);
	}
}

extern "C"	__declspec(dllexport)
void reset_algo_params(size_t algo_addr){

	strat::event_long_short* algo_p = reinterpret_cast<strat::event_long_short*>(algo_addr);

	algo_p->reset_params(
		read_ini<size_t>("OPTI_PARAM.OBSERV"),
		read_ini<size_t>("OPTI_PARAM.HOLD"),
		read_ini<double>("OPTI_PARAM.SD")
		);
}

extern "C"	__declspec(dllexport)
int process_end_day(size_t algo_addr, const wchar_t* hist_tick_path, size_t keep_days_no,
		logger::callback callback_handler, size_t max_iteration, size_t population_size){

	logger::on_callback = callback_handler;

	util::delete_hist_tick(convert_wchar_to_string(hist_tick_path), keep_days_no);

	boost::posix_time::ptime end_time = boost::posix_time::second_clock::local_time();
	boost::posix_time::ptime start_time = end_time - boost::posix_time::hours(keep_days_no * 24);

	optimize(algo_addr, hist_tick_path, start_time, end_time, max_iteration, population_size);

	reset_algo_params(algo_addr);

	return 1;
}

#endif
