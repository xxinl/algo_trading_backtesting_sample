// Demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../Strat/algo/event/event_anti_long_short.h"

#include <vector>
#include <string>
#include <fstream>

#include<boost\tokenizer.hpp>
#include<boost\date_time.hpp>
#include<boost\lexical_cast.hpp>

#include <ppl.h>

int _tmain(int argc, _TCHAR* argv[])
{
	//std::ifstream file("../back_test_files/EURUSD_min_2013.csv");
	std::ifstream file("../test_files/EURUSD_min_11-24-2013.csv");
	std::string line;
	//std::vector<int> cols{ 1, 2, 6 }; //1 date, 2 time, 6 close

	std::vector<strat::event_anti_long_short> algos;

	LOG("tester_begin algo constructor");
	//strat::event_anti_long_short algo("usd", "eur", "../back_test_files/Calendar-2013.csv", 15, 90, 0.0003);
	strat::event_anti_long_short algo("usd", "eur", "../test_files/Calendar-11-24-2013.csv", 15, 90, 0.0003);
	LOG("tester_end algo constructor");
	algos.push_back(algo);

	std::queue<boost::posix_time::ptime> event_q(algo.get_event_queue());

	//strat::event_anti_long_short algo1("usd", "eur", event_q, 10, 60, 0.0003);
	//algos.push_back(algo1);

	//strat::event_anti_long_short algo2("usd", "eur", event_q, 15, 120, 0.0003);
	//algos.push_back(algo2);

	for (int o = 1; o <= 10; o++){
		for (int h = o + 1; h <= 120; h++){
			strat::event_anti_long_short algo2("usd", "eur", event_q, o, h, 0.0003);
			algos.push_back(algo2);
			LOG("push algo " << o << "-" << h);
		}
	}

	//skip header
	std::getline(file, line);

	boost::posix_time::ptime t;
	while (std::getline(file, line)) {

		boost::tokenizer<boost::escaped_list_separator<char> > tk(
			line, boost::escaped_list_separator<char>('\\', ',', '\"'));

		std::vector<std::string> row_vec;
		for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator it(tk.begin());
			it != tk.end(); ++it)	{
			row_vec.push_back(*it);
		}

		t = util::convert_to_dt(row_vec[1] + row_vec[2], "%Y%m%d%H%M%S");

		strat::tick tick1;
		tick1.time_stamp = t;
		tick1.close = boost::lexical_cast<double>(row_vec[6]);

		if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){

			LOG("tester_processing tick " << t);
		}


		//for (std::vector<strat::event_anti_long_short>::iterator it = algos.begin();
		//	it != algos.end(); ++it)
		concurrency::parallel_for(size_t(0), algos.size(), [&algos, &tick1](int i)
		{
			std::vector<strat::position> close_pos;
			//it->process_tick(tick1, close_pos);
			algos[i].process_tick(tick1, close_pos);
		});

		//if (!close_pos.empty())
		//	LOG_POSITIONS(close_pos);
	}

	for (std::vector<strat::event_anti_long_short>::iterator it = algos.begin();
		it != algos.end(); ++it)
	{
		std::vector<strat::position> close_pos;
		close_pos = it->get_closed_position_history();
		if (!close_pos.empty())
			LOG_POSITIONS(close_pos, it->get_obser_threshold(), it->get_hold_threshold());
	}

	std::cout << "done" << std::endl;
	std::cin.get();

	return 0;
}

