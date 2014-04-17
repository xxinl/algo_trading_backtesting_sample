
//TODO test idear that follow when > 2*sd and anti when > 1*sd
//TODO consider/test tp price, maybe 3*sd?
//TODO test idear that during the abserve period, the close price should continuously rising
//TODO test trailing stop
//TODO sd for the current day/or maybe runing last 24 hours needs to be considered. 
//				i.e. the current day's volatility is too hight then close all positions


// Demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "algo/event/event_long_short.h"
#include "algo\event\event_algo_ma.h"

#include <vector>
#include <string>
#include <fstream>

#include <boost\tokenizer.hpp>
#include <boost\date_time.hpp>
#include <boost\lexical_cast.hpp>

#include <ppl.h>
#include <concurrent_vector.h>

using std::string;

void algo_factory_get(const string& algo_name, concurrency::concurrent_vector<std::unique_ptr<strat::event_algo>>& algos){
	
	//std::vector<int> cols{ 1, 2, 6 }; //1 date, 2 time, 6 close

	const string path = "C:/workspace/Strat/back_test_files/Calendar-2013.csv";//"../../back_test_files/Calendar-2013.csv";

	if (algo_name == "event_long_short"){		

		LOG("tester_begin event_long_short algo constructor");
		std::unique_ptr<strat::event_algo> algo(new strat::event_long_short(
			"eur", "usd", path, size_t(15), size_t(90), 0.0003));
		LOG("tester_end event_long_short algo constructor");
		algos.push_back(std::move(algo));

		std::queue<boost::posix_time::ptime> event_q(algos[0]->get_event_queue());

		concurrency::parallel_for(size_t(1), size_t(10), [&event_q, &algos](int o){

			for (size_t h = o + 1; h <= 120; h++){
				std::unique_ptr<strat::event_algo> algo2(new strat::event_long_short(
					"eur", "usd", event_q, o, h, 0.0003));
				algos.push_back(std::move(algo2));
				LOG("push algo " << o << "-" << h);
			}
		});
	} else if (algo_name == "event_algo_ma"){
		
		LOG("tester_begin event_algo_ma algo constructor");
		std::unique_ptr<strat::event_algo> algo(new strat::event_algo_ma(
			"eur", "usd", path,
			7, 45, 0.0003, 50, 510));
		LOG("tester_end event_algo_ma algo constructor");
		algos.push_back(std::move(algo));

		std::queue<boost::posix_time::ptime> event_q(algos[0]->get_event_queue());

		for (size_t sma_peroid = 5; sma_peroid <= 125; sma_peroid += 10){

			for (size_t look_back = 20; look_back <= 1000; look_back += 50){
				std::unique_ptr<strat::event_algo> algo2(new strat::event_algo_ma(
					"eur", "usd", event_q,
					7, 45, 0.0003, sma_peroid, look_back));
				algos.push_back(std::move(algo2));
				LOG("push algo " << sma_peroid << "-" << look_back);
			}
		}
	}
}

void run_back_test(boost::posix_time::ptime start_t, boost::posix_time::ptime end_t, 
	concurrency::concurrent_vector<std::unique_ptr<strat::event_algo>>& algos){

	const string path = "C:/workspace/Strat/back_test_files/EURUSD_min_2013.csv";
	std::ifstream file(path); //("../../back_test_files/EURUSD_min_2013.csv");
	if (!file.is_open())
	{
		LOG_SEV("file doesn't exist at " << path, logger::error);
	}

	std::string line;
	//skip header
	std::getline(file, line);
	const double sp = 0.01;
	boost::posix_time::ptime skip_date = 
		boost::posix_time::time_from_string(std::string("1900-01-01 00:00:00.000"));

	while (std::getline(file, line)) {

		boost::tokenizer<boost::escaped_list_separator<char> > tk(
			line, boost::escaped_list_separator<char>('\\', ',', '\"'));

		std::vector<std::string> row_vec;
		for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator it(tk.begin());
			it != tk.end(); ++it)	{
			row_vec.push_back(*it);
		}

		boost::posix_time::ptime t = util::convert_to_dt(row_vec[1] + row_vec[2], "%Y%m%d%H%M%S");

		if (t.is_not_a_date_time()){

			LOG_SEV("tester_not a date time, skiped tick " << t, logger::error);
			continue;
		}

		//skip if not in test date range
		if (t < start_t){

			if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){

				LOG("tester_skipping tick " << t << " not in test date range");
			}

			continue;
		}

		if (t > end_t)
		{
			if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){

				LOG("tester_reached the end_t at tick " << t);
			}

			break;
		}

		boost::posix_time::ptime next_event_t = algos[0]->get_event_queue().front();
		//skip if no event for the day
		if (next_event_t.date() > t.date()
			&& t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){

			skip_date = t;
			LOG("tester_skipping tick " << t << " no event for the day");
		}

		if (skip_date.date() == t.date())
			continue;

		strat::tick tick1;
		tick1.time_stamp = t;
		tick1.close = boost::lexical_cast<double>(row_vec[6]);

		if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){

			LOG("tester_processing tick " << t);
		}

		concurrency::parallel_for(size_t(0), algos.size(), [&algos, &tick1, &sp](int i)
		{
			strat::position close_pos;
			algos[i]->process_tick(tick1, close_pos, sp);
		});

		if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){

			for (concurrency::concurrent_vector<std::unique_ptr<strat::event_algo>>::iterator it = algos.begin();
				it != algos.end(); ++it)	{

				std::list<strat::position> close_pos;
				close_pos = (*it)->get_closed_position_history();
				if (!close_pos.empty()){

					LOG_POSITIONS(close_pos, (*it)->get_threshold1(), (*it)->get_threshold2());
					(*it)->clear_closed_position_history();
				}
			}
		}
	}

	for (concurrency::concurrent_vector<std::unique_ptr<strat::event_algo>>::iterator it = algos.begin();
		it != algos.end(); ++it)	{

		std::list<strat::position> close_pos;
		close_pos = (*it)->get_closed_position_history();
		if (!close_pos.empty())
			LOG_POSITIONS(close_pos, (*it)->get_threshold1(), (*it)->get_threshold2());
	}

	std::cout << "done" << std::endl;
	std::cin.get();
}

/// argv[0] n months start from Jan
int _tmain(int argc, _TCHAR* argv[])
{
	boost::posix_time::ptime start_t, end_t;
	start_t = boost::posix_time::time_from_string(std::string("2013-01-01 00:00:00.000"));

	int c;
	if (argc > 1)
	{
		c = boost::lexical_cast<double>(argv[1]);
	}
	else
	{
		std::cout << "test first n weeks start from 01Jan13. n=" << std::endl;
		std::cin >> c;
		std::cin.get();
	}

	end_t = start_t + boost::gregorian::weeks(c);
	std::cout << "test start from " << start_t << " end on " << end_t << ". enter to continue" << std::endl;
	std::cin.get();

	concurrency::concurrent_vector<std::unique_ptr<strat::event_algo>> algos;
	algo_factory_get("event_algo_ma", algos);
	run_back_test(start_t, end_t, algos);

	return 0;
}

