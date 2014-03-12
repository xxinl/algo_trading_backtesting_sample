
#ifndef _UTIL
#define _UTIL

#include "tick.h"

#include<vector>
#include<string>
#include<fstream>
#include<sstream>

#include<boost\tokenizer.hpp>
#include<boost\date_time.hpp>
#include<boost\lexical_cast.hpp>

class util{
public:
	static int read_csv(const char *path, std::vector<std::vector<std::string>> &vec, const std::vector<int> &cols, bool with_header = true){
		
		try {

			std::ifstream file(path);
			std::string line;
			int cols_len = cols.size();

			if (with_header) std::getline(file, line);

			while (std::getline(file, line)) {
				
				boost::tokenizer<boost::escaped_list_separator<char> > tk(
					line, boost::escaped_list_separator<char>('\\', ',', '\"'));
				int cols_i = 0;
				int i = 0;
				std::vector<std::string> row_vec;

				for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator it(tk.begin());
					it != tk.end(); ++it)
				{
					if (cols_i >= cols_len) break;

					if (cols[cols_i] == i++){
						row_vec.push_back(*it);
						cols_i++;
					}
				}

				vec.push_back(row_vec);
			}

			return 1;
		}
		catch (std::ifstream::failure e) {
			//std::cerr << "Exception opening/reading/closing file\n";
			throw;
		}
		catch (...) { 
			throw;
		}
	}

	static boost::posix_time::ptime convert_to_dt(const std::string s, const std::string format = "%Y %a %b %d %H:%M")	{

		const std::locale formats[] = {
			std::locale(std::locale::classic(), new  boost::posix_time::time_input_facet(format)) };
		const size_t formats_n = sizeof(formats) / sizeof(formats[0]);

		boost::posix_time::ptime pt;
		for (size_t i = 0; i<formats_n; ++i)
		{
			std::istringstream is(s);
			is.imbue(formats[i]);
			is >> pt;
			if (pt != boost::posix_time::ptime()) break;
		}

		return pt;

		//boost::posix_time::ptime timet_start(boost::gregorian::date(1970, 1, 1));
		//boost::posix_time::time_duration diff = pt - timet_start;
		//return diff.ticks() / boost::posix_time::time_duration::rep_type::ticks_per_second;
	}

	//// http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
	//static double get_run_sd(double &run_mean, double new_d, int n, double &run_M2){

	//	double delta = new_d - run_mean;
	//	run_mean += delta / n;
	//	run_M2 += delta*(new_d - run_mean);
	//	return sqrt(run_M2 / (n - 1));
	//}

	static int read_tick_csv(const char *path, std::vector<strat::tick> &tick_vec){
		
		std::vector<std::vector<std::string>> csv_vec;
		std::vector<int> cols_v{ 1, 2, 6 };
		util::read_csv(path, csv_vec, cols_v);

		boost::posix_time::ptime t;
		for (std::vector<std::vector<std::string>>::iterator it = csv_vec.begin();
			it != csv_vec.end(); ++it){

			t = util::convert_to_dt((*it)[0] + (*it)[1], "%Y%m%d%H%M%S");
			strat::tick tick1;
			tick1.time_stamp = t;
			tick1.close = boost::lexical_cast<double>((*it)[2]);
			tick_vec.push_back(tick1);
		}

		return 1;
	}

	static std::string get_current_dt_str(const char *format = "%Y%m%d.%H%M%S"){
	
		std::stringstream s;

		const boost::posix_time::ptime now =
			boost::posix_time::second_clock::local_time();

		boost::posix_time::time_facet*const f =
			new boost::posix_time::time_facet(format);
		s.imbue(std::locale(s.getloc(), f));
		s << now;

		return s.str();
	}

	static std::string dt_to_string(boost::posix_time::ptime pt){
		return boost::posix_time::to_simple_string(pt);
	}
};

#endif