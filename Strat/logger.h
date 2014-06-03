

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

			boost::log::add_console_log(std::clog, boost::log::keywords::format = "%TimeStamp%: %Message%"
			,boost::log::keywords::filter = boost::log::expressions::attr< severity_level >("Severity") > debug);
			
			string file_name = "C:/strat_logs/" + util::get_current_dt_str();

			boost::log::add_file_log
				(
				file_name + ".log",
				boost::log::keywords::filter = boost::log::expressions::attr< severity_level >("Severity") > debug
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
				file_name + "C:/strat_logs/tick.tk",
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

		if (on_callback != nullptr && lev > debug)
			on_callback(msg.c_str(), lev);
	}

	static void log_position(const strat::position& position, size_t obser, size_t hold){
		std::stringstream s;

		s << obser << "," << hold << ","
			//<< position.obser_tick.time_stamp << "," << position.obser_tick.close
			<< "," << position.open_tick.time_stamp << "," << position.open_tick.close
			<< "," << position.close_tick.time_stamp << "," << position.close_tick.close 
			<< "," << position.type;

		log_sev(s.str(), order);
	}

	static void log_positions(const std::list<strat::position>& positions, size_t obser, size_t hold){
		
		std::for_each(positions.begin(), positions.end(), [=, &obser, &hold](const strat::position p){

			log_position(p, obser, hold);
		});
	}

	static void log_tick(string time, double bid, double ask, double lask, size_t volume){
		std::stringstream s;

		s << time << "," << bid << "," << ask << "," << lask << "," << volume;

		log_sev(s.str(), tick);
	}
};

#define LOG_SEV(msg, lev) logger::log_sev(static_cast<std::ostringstream&>( \
																				std::ostringstream().flush() << msg \
																				).str(), lev);

#define LOG_POSITIONS(pos, obser, hold) logger::log_positions(pos, obser, hold);

#define LOG(msg) logger::log_sev(static_cast<std::ostringstream&>( \
															std::ostringstream().flush() << msg  \
															).str(), logger::normal);

#define LOG_TICK(time, bid, ask, last, volume) logger::log_tick(time, bid, ask, last, volume);

#endif
