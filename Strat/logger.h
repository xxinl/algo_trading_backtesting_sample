

#ifndef _LOGGER
#define _LOGGER

#include "util.h"
#include "position.h"

#include <string>
#include <mutex>

#include "boost\log\common.hpp"
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/logger.hpp>

//https://github.com/boostorg/log/blob/master/example/basic_usage/main.cpp
class logger{
public:
	enum severity_level
	{
		debug,
		normal,
		notification,
		warning,
		error,
		critical,
		order
	};

private:

	logger(){
	}

	std::once_flag flag;

	void init(){

		std::call_once(flag, [](){
			boost::log::add_console_log(std::clog, boost::log::keywords::format = "%TimeStamp%: %Message%",
			boost::log::keywords::filter = boost::log::expressions::attr< severity_level >("Severity") > debug);
			
			std::string file_name = util::get_current_dt_str();

			boost::log::add_file_log
				(
				file_name + ".log",
				boost::log::keywords::filter = boost::log::expressions::attr< severity_level >("Severity") > debug
				&& boost::log::expressions::attr< severity_level >("Severity") != order,
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

			boost::log::add_common_attributes();

			BOOST_LOG_FUNCTION();
		});
	}

public:
	
	~logger(){};

	static void log_sev(std::string const& msg, severity_level lev){
		static logger l; 
		l.init();

		static boost::log::sources::severity_logger< severity_level > slg;

		BOOST_LOG_SEV(slg, lev) << msg;
	}

	static void log_position(const strat::position &position, int obser, int hold){
		std::stringstream s;

		s << obser << "," << hold << ","
			<< position.open_tick.time_stamp << "," << position.open_tick.close 
			<< "," << position.close_tick.time_stamp << "," << position.close_tick.close 
			<< "," << position.type;

		log_sev(s.str(), severity_level::order);
	}

	static void log_positions(const std::vector<strat::position> &positions, int obser, int hold){
		
		for (std::vector<strat::position>::const_iterator it = positions.begin();
			it != positions.end(); ++it){
		
			log_position(*it, obser, hold);
		}
	}
};

#define LOG_SEV(msg, lev) logger::log_sev(static_cast<std::ostringstream&>( \
	std::ostringstream().flush() << msg \
	).str(), lev);

#define LOG_POSITIONS(pos, obser, hold) logger::log_positions(pos, obser, hold);

#define LOG(msg) logger::log_sev(static_cast<std::ostringstream&>( \
	std::ostringstream().flush() << msg \
	).str(), logger::normal);

#endif
