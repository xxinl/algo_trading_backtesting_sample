
#ifndef _STRAT_OPTIMIZABLE_ALGO_GENETIC
#define _STRAT_OPTIMIZABLE_ALGO_GENETIC

#include <string>
#include <vector>
#include <tuple>  

using std::string;

namespace strat{

	template<typename ...Params>
	class optimizable_algo_genetic{
		
	protected:
		int _rand_from_range(size_t low, size_t high){

			return rand() % (high - low + 1) + low;
		}

	public:
		optimizable_algo_genetic(){}
		virtual ~optimizable_algo_genetic(){};

		virtual std::vector<std::pair<double, std::tuple<Params...>>> init_optimization_population(int population_size) = 0;

		virtual std::shared_ptr<algo> get_optimizable_algo(std::tuple<Params...>) = 0;

		virtual std::tuple<Params...> mate(const std::tuple<Params...>& i, const std::tuple<Params...>& j) = 0;

		virtual std::tuple<Params...> mutate(std::tuple<Params...> params) = 0;

		double calculate_fitness(const std::vector<tick>& ticks, std::tuple<Params...> params){
			
			std::shared_ptr<algo> algo_p = get_optimizable_algo(params);

			double ttl_ret = 0;
			for (tick t : ticks){
				
				position close_pos;
				algo_p->process_tick(t, close_pos);

				if (close_pos.type != signal::NONE){
					
					ttl_ret += close_pos.close_tick.last - close_pos.open_tick.last;
				}
			}

			return ttl_ret;
		}
	};
}

#endif