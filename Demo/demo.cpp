// Demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../Strat/algo/event/event_anti_long_short.h"

#include <vector>
#include <string>
#include <fstream>

#include <boost\tokenizer.hpp>
#include <boost\date_time.hpp>
#include <boost\lexical_cast.hpp>

#include <ppl.h>
#include <concurrent_vector.h>

using std::string;

void run_back_test(){

	std::ifstream file("../../back_test_files/EURUSD_min_2013.csv");
	//std::ifstream file("../../test_files/EURUSD_min_11-24-2013.csv");
	std::string line;
	//std::vector<int> cols{ 1, 2, 6 }; //1 date, 2 time, 6 close

	concurrency::concurrent_vector<strat::event_anti_long_short> algos;

	LOG("tester_begin algo constructor");
	strat::event_anti_long_short algo("eur", "usd", "../../back_test_files/Calendar-2013.csv", 15, 90, 0.0003);
	//strat::event_anti_long_short algo("eur", "usd", "../../test_files/Calendar-11-24-2013.csv", size_t(15), size_t(90), 0.0003);
	LOG("tester_end algo constructor");
	algos.push_back(algo);

	std::queue<boost::posix_time::ptime> event_q(algo.get_event_queue());

	concurrency::parallel_for(size_t(1), size_t(10), [&event_q, &algos](int o){

		for (size_t h = o + 1; h <= 120; h++){
			strat::event_anti_long_short algo2("eur", "usd", event_q, o, h, 0.0003);
			algos.push_back(algo2);
			LOG("push algo " << o << "-" << h);
		}
	});

	//skip header
	std::getline(file, line);

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

		boost::posix_time::ptime next_event_t = algo.get_event_queue().front();
		//skip if no event for the day
		if (next_event_t.date() > t.date()){

			if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){

				LOG("tester_skipping tick " << t << " no event for the day");
			}

			continue;
		}

		strat::tick tick1;
		tick1.time_stamp = t;
		tick1.close = boost::lexical_cast<double>(row_vec[6]);

		if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){

			LOG("tester_processing tick " << t);
		}

		concurrency::parallel_for(size_t(0), algos.size(), [&algos, &tick1](int i)
		{
			strat::position close_pos;
			algos[i].process_tick(tick1, close_pos);
		});

		if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){

			for (concurrency::concurrent_vector<strat::event_anti_long_short>::iterator it = algos.begin();
				it != algos.end(); ++it)	{

				std::list<strat::position> close_pos;
				close_pos = it->get_closed_position_history();
				if (!close_pos.empty()){

					LOG_POSITIONS(close_pos, it->get_obser_threshold(), it->get_hold_threshold());
					it->clear_closed_position_history();
				}
			}
		}
	}

	for (concurrency::concurrent_vector<strat::event_anti_long_short>::iterator it = algos.begin();
		it != algos.end(); ++it)	{

		std::list<strat::position> close_pos;
		close_pos = it->get_closed_position_history();
		if (!close_pos.empty())
			LOG_POSITIONS(close_pos, it->get_obser_threshold(), it->get_hold_threshold());
	}

	std::cout << "done" << std::endl;
	std::cin.get();
}

int _tmain(int argc, _TCHAR* argv[])
{
	run_back_test();

	return 0;
}

