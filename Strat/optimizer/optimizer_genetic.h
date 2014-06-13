
#ifndef _STRAT_OPTIMIZER_GENETIC
#define _STRAT_OPTIMIZER_GENETIC


#include "optimizable_algo_genetic.h"

#include <string>
#include <utility>
#include <tuple>   
#include <vector>
#include <algorithm>

#include <ppl.h>
#include <concurrent_vector.h>

#include <boost/date_time.hpp>

using std::string;

namespace strat{

	template<typename ...Params>
	class optimizer_genetic{
	
	private:
		optimizable_algo_genetic<Params...>* _opti_algo;
		const int _elite_size;
		const double _mutation_rate;
		const int _max_iteration;
		const int _population_size;
		const string _hist_ticks_f_path;

		typedef std::pair<double, std::tuple<Params...>> CITIZEN_TYPE;

		concurrency::concurrent_vector<CITIZEN_TYPE> _population;
		concurrency::concurrent_vector<CITIZEN_TYPE> _next_generation;

		std::vector<tick> _read_sample_ticks(string hist_ticks_f_path, 
			boost::posix_time::ptime start_date, boost::posix_time::ptime end_date){
		
			LOG("optimizer reading sample ticks");

			std::vector<tick> ticks;
			std::vector<int> cols_v{ 0, 3 };

			util::read_tick_csv(hist_ticks_f_path, ticks, start_date, end_date, "%Y.%m.%d %H:%M", cols_v);
			return ticks;
		}

		void _init_population(){
			
			_population = _opti_algo->init_optimization_population(_population_size);
			_next_generation = _population;
		}

		void _sort_population(){

			std::sort(_population.begin(), _population.end(),
				[](CITIZEN_TYPE i, CITIZEN_TYPE j){
				return i.first > j.first;
			});
		}

		void _calc_population_fitness(size_t start_i, const std::vector<tick>& ticks,
			concurrency::concurrent_vector<CITIZEN_TYPE>& population, optimizable_algo_genetic<Params...>* opti_algo){
			
			concurrency::parallel_for(size_t(start_i), size_t(_population_size) - 1,
				[&ticks, &population, &opti_algo](int i){
				
				population[i].first = opti_algo->calculate_fitness(ticks, population[i].second);
			});
		}

		void _mate(){

			//keep elit
			for (int i = 0; i < _elite_size; ++i){
			
				_next_generation[i] = _population[i];
			}

			//mate/mutate the rest
			for (int i = _elite_size; i < _population_size; ++i){

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

		void _print_elite(){
		
			for (int i = 0; i < _elite_size; ++i){

				LOG("elite..." << _opti_algo->print_params(_population[i].second) << " fitness:" << _population[i].first);
			}
		}

	public:
		optimizer_genetic(string hist_ticks_f_path, optimizable_algo_genetic<Params...>* opti_algo,
			double elite_rate = 0.20f, double mutation_rate = 0.40f, int max_iteration = 16, int population_size = 64) :
			_opti_algo(opti_algo), _elite_size(elite_rate * population_size), _mutation_rate(mutation_rate * 100), 
			_max_iteration(max_iteration), _population_size(population_size), _hist_ticks_f_path(hist_ticks_f_path){		}

		CITIZEN_TYPE optimize(boost::posix_time::ptime start_date, boost::posix_time::ptime end_date){

			LOG("optimizer starting iteration:" << _max_iteration << " population:" << _population_size <<
				" start:" << start_date << " end:" << end_date);

			std::vector<tick> ticks = _read_sample_ticks(_hist_ticks_f_path, start_date, end_date);
			//_opti_algo->remove_non_important_ticks(ticks);

			LOG("initialising population");
			_init_population();
			_calc_population_fitness(0, ticks, _population, _opti_algo);
			_sort_population();

			LOG("initialised population with top fitness " 
				<< _population.front().first << " params: " << _opti_algo->print_params(_population.front().second));
			_print_elite();

			for (int i = 0; i < _max_iteration; i++){

				LOG("starting optimization iteration " << i);

				_mate();

				_population.swap(_next_generation);

				_calc_population_fitness(_elite_size, ticks, _population, _opti_algo);
				_sort_population();

				LOG("completed optimization iteration " << i << " with top fitness " 
					<< _population.front().first << " params: " << _opti_algo->print_params(_population.front().second));
				_print_elite();
			}

			return _population[0];
		}
	};
}

#endif