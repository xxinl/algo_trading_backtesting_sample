
#ifndef _STRAT_OPTIMIZER_GENETIC
#define _STRAT_OPTIMIZER_GENETIC


#include "optimizable_algo_genetic.h"

#include <string>
#include <utility>
#include <tuple>   
#include <vector>
#include <algorithm>

#include <ppl.h>

#include <boost/date_time.hpp>

using std::string;

namespace strat{

	template<typename ...Params>
	class optimizer_genetic{
	
	private:
		optimizable_algo_genetic<Params...>* _opti_algo;
		const int _elit_size;
		const double _mutation_rate;
		const int _max_iteration;
		const int _population_size;
		const string _hist_ticks_f_path;

		typedef std::pair<double, std::tuple<Params...>> CITIZEN_TYPE;

		std::vector<CITIZEN_TYPE> _population;
		std::vector<CITIZEN_TYPE> _next_generation;

		std::vector<tick> read_sample_ticks(string hist_ticks_f_path, 
			boost::posix_time::ptime start_date, boost::posix_time::ptime end_date){
		
			std::vector<tick> ticks;
			std::vector<int> cols_v{ 0, 3 };

			util::read_tick_csv(hist_ticks_f_path, ticks, start_date, end_date, "%Y.%m.%d %H:%M", cols_v);
			return ticks;
		}

		void init_population(){
			
			_population = _opti_algo->init_optimization_population(_population_size);
			_next_generation = _population;
		}

		void sort_population(){

			std::sort(_population.begin(), _population.end(),
				[](CITIZEN_TYPE i, CITIZEN_TYPE j){
				return i.first > j.first;
			});
		}

		void calc_population_fitness(size_t start_i, const std::vector<tick>& ticks,
			std::vector<CITIZEN_TYPE>& population, optimizable_algo_genetic<Params...>* opti_algo){
			
			concurrency::parallel_for(size_t(start_i), size_t(_population_size) - 1,
				[&ticks, &population, &opti_algo](int i){
				
				population[i].first = opti_algo->calculate_fitness(ticks, population[i].second);
			});
		}

		void mate(){

			//keep elit
			for (int i = 0; i < _elit_size; ++i){
			
				_next_generation[i] = _population[i];
			}

			//mate/mutate the rest
			for (int i = _elit_size; i < _population_size; ++i){

				int i1 = rand() % _population_size;
				int i2 = rand() % _population_size;
								
				std::tuple<Params...> mated = _opti_algo->mate(_population[i1].second, _population[i2].second);
				//_next_generation[i] = std::make_pair<double, std::tuple<Params...>>(0, mated);
				_next_generation[i].first = 0;
				_next_generation[i].second = mated;

				if (rand() % 100 < _mutation_rate){

					_next_generation[i].second = _opti_algo->mutate(_next_generation[i].second);
				}
			}
		}

	public:
		optimizer_genetic(string hist_ticks_f_path, optimizable_algo_genetic<Params...>* opti_algo,
			double elit_rate = 0.20f, double mutation_rate = 0.40f, int max_iteration = 128, int population_size = 128) :
			_opti_algo(opti_algo), _elit_size(elit_rate * population_size), _mutation_rate(mutation_rate * 100), 
			_max_iteration(max_iteration), _population_size(population_size), _hist_ticks_f_path(hist_ticks_f_path){}

		CITIZEN_TYPE optimize(boost::posix_time::ptime start_date, boost::posix_time::ptime end_date){

			std::vector<tick> ticks = read_sample_ticks(_hist_ticks_f_path, start_date, end_date);

			init_population();
			calc_population_fitness(0, ticks, _population, _opti_algo);
			sort_population();

			LOG("initialised population with top fitness " 
				<< _population.front().first << " params: " << _opti_algo->print_params(_population.front().second));

			for (int i = 0; i < _max_iteration; i++){

				LOG("starting optimization iteration " << i);

				mate();

				_population.swap(_next_generation);

				calc_population_fitness(_elit_size, ticks, _population, _opti_algo);
				sort_population();

				LOG("completed optimization iteration " << i << " with top fitness " 
					<< _population.front().first << " params: " << _opti_algo->print_params(_population.front().second));
			}

			return _population[0];
		}
	};
}

#endif