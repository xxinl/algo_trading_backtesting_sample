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

int _tmain(int argc, _TCHAR* argv[])
{
	std::ifstream file("../back_test_files/EURUSD_min_2013.csv");
	std::string line;
	//std::vector<int> cols{ 1, 2, 6 }; //1 date, 2 time, 6 close

	LOG("tester_begin algo constructor")
		strat::event_anti_long_short algo("usd", "eur", "../back_test_files/Calendar-2013.csv"
		, 5, 30, 0.0003);
	LOG("tester_end algo constructor")

	//skip header
	std::getline(file, line);

	boost::posix_time::ptime t;
	std::vector<strat::position> close_pos;
	while (std::getline(file, line)) {
		close_pos.clear();

		boost::tokenizer<boost::escaped_list_separator<char> > tk(
			line, boost::escaped_list_separator<char>('\\', ',', '\"'));

		std::vector<std::string> row_vec;
		for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator it(tk.begin());
			it != tk.end(); ++it)	{
			row_vec.push_back(*it);
		}

		t = util::convert_to_dt(row_vec[1] + row_vec[2], "%Y%m%d%H%M%S");

		if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0){
		
			LOG("tester_processing tick for " << t)
		}

		strat::tick tick1;
		tick1.time_stamp = t;
		tick1.close = boost::lexical_cast<double>(row_vec[6]);

		algo.process_tick(tick1, close_pos);

		//if (!close_pos.empty())
		//	LOG_POSITIONS(close_pos);
	}

	std::cout << "done" << std::endl;
	std::cin.get();

	return 0;
}

