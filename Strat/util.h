
#ifndef _UTIL
#define _UTIL

#include "tick.h"

#include<vector>
#include<list>
#include<string>
#include<fstream>
#include<sstream>

#include<boost\tokenizer.hpp>
#include<boost\date_time.hpp>
#include<boost\lexical_cast.hpp>

using std::string;

class util{
public:
	static int read_csv(string path, std::vector<std::vector<std::string>>& vec, const std::vector<int>& cols, bool with_header = true){
		
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

		const std::locale f = std::locale(std::locale::classic(), new  boost::posix_time::time_input_facet(format));

		boost::posix_time::ptime pt;
		std::istringstream is(s);
		is.imbue(f);
		is >> pt;

		return pt;
	}

	static int read_tick_csv(string path, std::vector<strat::tick>& tick_vec){
		
		std::vector<std::vector<std::string>> csv_vec;
		std::vector<int> cols_v{ 1, 2, 6 };
		util::read_csv(path, csv_vec, cols_v);

		boost::posix_time::ptime t;
		for (std::vector<std::vector<std::string>>::iterator it = csv_vec.begin(); it != csv_vec.end(); ++it){

			t = util::convert_to_dt((*it)[0] + (*it)[1], "%Y%m%d%H%M%S");
			strat::tick tick1;
			tick1.time_stamp = t;
			tick1.close = boost::lexical_cast<double>((*it)[2]);
			tick_vec.push_back(tick1);
		}

		return 1;
	}

	static std::string get_current_dt_str(){
	
		std::stringstream s;

		const boost::posix_time::ptime now =
			boost::posix_time::second_clock::local_time();

		boost::posix_time::time_facet*const f =
			new boost::posix_time::time_facet("%Y%m%d.%H%M%S");
		s.imbue(std::locale(s.getloc(), f));
		s << now;

		return s.str();
	}

	static std::string dt_to_string(boost::posix_time::ptime pt){

		return boost::posix_time::to_simple_string(pt);
	}
};

#endif