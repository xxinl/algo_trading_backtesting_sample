
#ifndef _STRAT_OPTIMIZER_GENETIC_RESEARCH
#define _STRAT_OPTIMIZER_GENETIC_RESEARCH

#include "tick.h"
#include "optimizer_genetic.h"


namespace strat{

	template<typename ...Params>
	class optimizer_genetic_day_research : public optimizer_genetic<Params...>{
	
	private:

		double _calc_ret_sd(const std::vector<double> day_min_rates){
		
			double sum = 0;
			double ave;

			std::vector<double> min_ret;

			min_ret.push_back(0);
			for (int i = 1; i < day_min_rates.size(); ++i){
			
				auto ret = day_min_rates[i] - day_min_rates[i - 1];
				sum += ret;
				min_ret.push_back(ret);
			}

			ave = sum / min_ret.size();
			sum = 0;

			for (int i = 0; i < min_ret.size(); ++i){

				sum += std::pow(min_ret[i] - ave, 2);
			}

			return std::sqrt(sum / min_ret.size());
		}

	public:
		optimizer_genetic_day_research(string hist_ticks_f_path, optimizable_algo_genetic<Params...>* opti_algo,
			double elite_rate = 0.20f, double mutation_rate = 0.30f, int max_iteration = 16, int population_size = 64) :
			optimizer_genetic<Params...>(hist_ticks_f_path, opti_algo, elite_rate, mutation_rate, max_iteration, population_size){}

		void run_day_opti(boost::posix_time::ptime start_date, boost::posix_time::ptime end_date){

			std::vector<tick> ticks = _read_sample_ticks(_hist_ticks_f_path, start_date, end_date);

			std::vector<tick> day_ticks;
			std::vector<double> day_min_rates;
			tick last_t = ticks.front();
			boost::posix_time::ptime cur_day = ticks.front().time;

			for (tick t : ticks)
			{
				if (t.time.date() <= cur_day.date()){

					day_ticks.push_back(t);
					if (last_t.time.time_of_day().minutes() != t.time.time_of_day().minutes()){
					
						day_min_rates.push_back(last_t.last);
					}
				}
				else{
				
					double sd = _calc_ret_sd(day_min_rates);
					optimizer_genetic<Params...>::CITIZEN_TYPE params = optimizer_genetic<Params...>::_optimize(day_ticks);

					LOG("DAY_OPTI," << cur_day << "," << sd << ","
						<< optimizer_genetic<Params...>::_opti_algo->print_params(params.second) 
						<< "," << params.first);

					day_ticks.clear();
					day_ticks.push_back(t);
					cur_day = t.time;

					day_min_rates.clear();
				}

				last_t = t;
			}

			LOG("DAY_OPTI completed");
		}
	};
}

#endif _STRAT_OPTIMIZER_GENETIC_RESEARCH