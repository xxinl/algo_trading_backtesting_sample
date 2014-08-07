//0-hybrid
//1-dayrange
//2-bollinger

#include "stdafx.h"

#ifdef STRAT_EXPORTS

#include <oaidl.h>
#include <comutil.h>

#include "algo.h"
//#include "algo\event\event_algo_ma.h"
//#include "algo\event\event_long_short.h"
#include "algo\algo_bollinger.h"
#include "algo\algo_dayrange.h"
#include "algo\algo_hybrid.h"
#include "logger.h"

#ifndef MQL5_RELEASE
#include "optimizer\optimizable_algo_genetic.h"
#include "optimizer\optimizer_genetic.h"
//#include "optimizer\optimizer_genetic_day_research.h"
#endif MQL5_RELEASE

#include <string>
#include <future>
#include <exception>
#include <sstream>

#include <boost/date_time.hpp>

using std::string;

logger::callback logger::on_callback = nullptr;

//typedef strat::algo_bollinger ALGO_TYPE;
//#define OPTIMIZER_PARAMS size_t, size_t, double, double

typedef strat::algo_dayrange ALGO_TYPE;
#ifndef OPTIMIZER_PARAMS
#define OPTIMIZER_PARAMS int, double, double, double
#endif

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

#ifndef MQL5_RELEASE

void optimize(size_t algo_addr, const wchar_t* hist_tick_path, 
	boost::posix_time::ptime start_date, boost::posix_time::ptime end_date,
	size_t max_iteration, size_t population_size){

	srand((unsigned int)time(NULL));
	
	ALGO_TYPE* algo_p = reinterpret_cast<ALGO_TYPE*>(algo_addr);

	strat::optimizer_genetic<OPTIMIZER_PARAMS> optimizer(
		convert_wchar_to_string(hist_tick_path), algo_p, 0.2f, 0.3f, max_iteration, population_size);
	std::pair<double, std::tuple<OPTIMIZER_PARAMS>> opti_params = optimizer.optimize(start_date, end_date);

	//write_ini("OPTI_PARAM.OBSERV", std::get<0>(opti_params.second));
	//write_ini("OPTI_PARAM.HOLD", std::get<1>(opti_params.second));
	//write_ini("OPTI_PARAM.INI_T", std::get<2>(opti_params.second));
	//write_ini("OPTI_PARAM.OBSER_T", std::get<3>(opti_params.second));

	//strat::optimizer_genetic_day_research<OPTIMIZER_PARAMS> optimizer(
	//	convert_wchar_to_string(hist_tick_path), algo_p, 0.2f, 0.3f, max_iteration, population_size);
	//optimizer.run_day_opti(start_date, end_date);
}

#endif MQL5_RELEASE

#pragma endregion

extern "C"	__declspec(dllexport)
size_t get_dayrange_algo(const wchar_t* base, const wchar_t* quote,
							size_t complete_hour, double entry_lev, double exit_lev, double extend_factor,	
							logger::callback callback_handler){

	logger::on_callback = callback_handler;
	strat::algo* ret_p = nullptr;

	string base_str = convert_wchar_to_string(base);
	string quote_str = convert_wchar_to_string(quote);

	try{

		ret_p = new strat::algo_dayrange(base_str, quote_str, complete_hour, entry_lev, exit_lev);			
	}
	catch (std::exception& e){

		LOG_SEV("get_dayrange_algo error: " << e.what(), logger::error);
	}

	size_t ret_addr = reinterpret_cast<size_t>(ret_p);

	LOG("get_dayrange_algo instantiated. base:" << base_str << " quote:" << quote_str <<
		" complete_hour: " << complete_hour << " entry_lev:" << entry_lev << " exit_lev:" << exit_lev <<
		". return pointer adreess:" << ret_addr);

	return ret_addr;
}

extern "C"	__declspec(dllexport)
size_t get_bollinger_algo(const wchar_t* base, const wchar_t* quote,
						size_t obser_win, size_t hold_win, double ini_t, double obser_t,
						logger::callback callback_handler){

	logger::on_callback = callback_handler;
	strat::algo* ret_p = nullptr;

	string base_str = convert_wchar_to_string(base);
	string quote_str = convert_wchar_to_string(quote);

	try{		

		ret_p = new strat::algo_bollinger(base_str, quote_str,
			obser_win, hold_win, ini_t, obser_t);
	}
	catch (std::exception& e){

		LOG_SEV("get_bollinger_algo error: " << e.what(), logger::error);
	}

	size_t ret_addr = reinterpret_cast<size_t>(ret_p);

	LOG("get_bollinger_algo instantiated. base:" << base_str << " quote:" << quote_str <<
		" obser_win: " << obser_win << " hold_win:" << hold_win << " ini_t:" << ini_t << " obser_t:" << obser_t <<
		". return pointer adreess:" << ret_addr);

	return ret_addr;
}

extern "C"	__declspec(dllexport)
int delete_algo(size_t algo_addr){

	strat::algo* algo_p = reinterpret_cast<strat::algo*>(algo_addr);
	delete(algo_p);

	return 1;
}

//time eg.2013-11-25 23:50:00.000
//TODO  time pass in long
extern "C"	__declspec(dllexport)
int process_tick(size_t algo_addr, const wchar_t* time, double ask, double bid, double last, size_t volume,
									double stop_loss, bool* is_close_pos, logger::callback callback_handler){

	logger::on_callback = callback_handler;

	*is_close_pos = false;

	string time_str = convert_wchar_to_string(time);
	strat::algo* algo_p = reinterpret_cast<strat::algo*>(algo_addr);

	LOG_SEV("process_tick algo:" << algo_addr << " time:" << time_str
		<< " last:" << last << " sl:" << stop_loss, logger::debug);
	
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

#ifdef MQL5_RELEASE
	if (sig != strat::signal::NONE)
		LOG("return signal: " << sig);
#endif MQL5_RELEASE

	//close position signal
	if (close_pos.type != strat::signal::NONE)
	{
		LOG_POSITION(close_pos);
		*is_close_pos = true;
	}

	return sig;
}

#ifndef MQL5_RELEASE

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

#endif MQL5_RELEASE

extern "C"	__declspec(dllexport)
void reset_algo_params(size_t algo_addr){

	//ALGO_TYPE* algo_p = reinterpret_cast<ALGO_TYPE*>(algo_addr);

	//algo_p->reset_params(
	//	read_ini<size_t>("OPTI_PARAM.OBSERV"),
	//	read_ini<size_t>("OPTI_PARAM.HOLD"),
	//	read_ini<double>("OPTI_PARAM.INI_T"),
	//	read_ini<double>("OPTI_PARAM.OBSER_T")
	//	);
}

extern "C"	__declspec(dllexport)
void log_tick(const wchar_t* time, double ask, double bid, double last, size_t volume){

	string time_str = convert_wchar_to_string(time);
	LOG_TICK(time_str, ask, bid, last, volume);
}


//extern "C"	__declspec(dllexport)
//int process_end_day(size_t algo_addr, const wchar_t* hist_tick_path, size_t keep_days_no,
//		logger::callback callback_handler, size_t max_iteration, size_t population_size){
//
//	logger::on_callback = callback_handler;
//
//	util::delete_hist_tick(convert_wchar_to_string(hist_tick_path), keep_days_no);
//
//	boost::posix_time::ptime end_time = boost::posix_time::second_clock::local_time();
//	boost::posix_time::ptime start_time = end_time - boost::posix_time::hours(keep_days_no * 24);
//
//	optimize(algo_addr, hist_tick_path, start_time, end_time, max_iteration, population_size);
//
//	reset_algo_params(algo_addr);
//
//	return 1;
//}

#endif
