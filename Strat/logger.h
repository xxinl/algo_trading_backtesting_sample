

#ifndef _LOGGER
#define _LOGGER

#include "util.h"
#include "position.h"

#include <string>
#include <mutex>

#include <boost\log\common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/logger.hpp>

using std::string;


//https://github.com/boostorg/log/blob/master/example/basic_usage/main.cpp
class logger{

public:

	//TODO free char*
	typedef void(__stdcall * callback)(const char* msg, int severity);
	static callback on_callback;

	enum severity_level
	{
		debug,
		normal,
		notification,
		warning,
		error,
		critical,
		order,
		tick
	};

private:
	logger(){
	}

	std::once_flag flag;

	void init(){

		std::call_once(flag, [](){

#ifdef MQL5_RELEASE
			string file_name = "C:/strat_live_logs/" + util::get_current_dt_str();
			auto severity = normal;
#else
			string file_name = "C:/strat_logs/" + util::get_current_dt_str();
			auto severity = notification;
#endif MQL5_RELEASE

			//boost::log::add_console_log(std::clog, boost::log::keywords::format = "%TimeStamp%: %Message%"
			//	, boost::log::keywords::filter = boost::log::expressions::attr< severity_level >("Severity") >= severity);

			boost::log::add_file_log
				(
				file_name + ".log",
				boost::log::keywords::filter = boost::log::expressions::attr< severity_level >("Severity") >= severity
				&& boost::log::expressions::attr< severity_level >("Severity") < order,
				boost::log::keywords::format = boost::log::expressions::stream
				<< boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
				<< " <" << boost::log::expressions::attr< severity_level >("Severity")
				<< "> " << boost::log::expressions::message
				);

			boost::log::add_file_log
				(
				file_name + ".order",
				boost::log::keywords::filter = boost::log::expressions::attr< severity_level >("Severity") == order
				);

			boost::log::add_file_log
				(
				file_name + "_TICKS.tk",
				boost::log::keywords::filter = boost::log::expressions::attr< severity_level >("Severity") == tick
				);

			boost::log::add_common_attributes();

			BOOST_LOG_FUNCTION();
		});
	}

public:
	
	~logger(){};

	static void log_sev(string const& msg, severity_level lev){
		static logger l; 
		l.init();

		static boost::log::sources::severity_logger<severity_level> slg;

		BOOST_LOG_SEV(slg, lev) << msg;
		
#ifdef MQL5_RELEASE
		if (on_callback != nullptr && lev > debug && lev != 7)
			on_callback(msg.c_str(), lev);
#else
		if (on_callback != nullptr && lev >= notification && lev != 7)
			on_callback(msg.c_str(), lev);
#endif MQL5_RELEASE
	}

	static void log_position(const strat::position& position){
		std::stringstream s;

		double open_rate = position.type == strat::signal::BUY ? position.open_tick.ask : position.open_tick.bid;
		double close_rate = position.type == strat::signal::SELL ? position.close_tick.ask : position.close_tick.bid;

		s << position.open_tick.time << "," << open_rate
			<< "," << position.close_tick.time << "," << close_rate
			<< "," << position.type;

		log_sev(s.str(), order);
	}

	static void log_positions(const std::list<strat::position>& positions){
		
		std::for_each(positions.begin(), positions.end(), [=](const strat::position p){

			log_position(p);
		});
	}

	static void log_tick(string time, double ask, double bid, double lask, size_t volume){
		std::stringstream s;

		s << time << "," << ask << "," << bid << "," << lask << "," << volume;

		log_sev(s.str(), tick);
	}
};

#define LOG_SEV(msg, lev) logger::log_sev(static_cast<std::ostringstream&>( \
																				std::ostringstream().flush() << msg \
																				).str(), lev);

#define LOG_POSITION(pos) logger::log_position(pos);
#define LOG_POSITIONS(pos) logger::log_positions(pos);

#define LOG(msg) logger::log_sev(static_cast<std::ostringstream&>( \
															std::ostringstream().flush() << msg  \
															).str(), logger::normal);

#define LOG_TICK(time, ask, bid, last, volume) logger::log_tick(time, ask, bid, last, volume);

#endif
