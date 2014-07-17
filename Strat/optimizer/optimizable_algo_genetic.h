
#ifndef _STRAT_OPTIMIZABLE_ALGO_GENETIC
#define _STRAT_OPTIMIZABLE_ALGO_GENETIC

#include <string>
#include <vector>
#include <tuple>  

#include <concurrent_vector.h>

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

		virtual concurrency::concurrent_vector<std::pair<double, std::tuple<Params...>>> init_optimization_population(int population_size) = 0;

		virtual std::shared_ptr<algo> get_optimizable_algo(std::tuple<Params...>) = 0;

		virtual std::tuple<Params...> mate(const std::tuple<Params...>& i, const std::tuple<Params...>& j) = 0;

		virtual std::tuple<Params...> mutate(std::tuple<Params...> params) = 0;

		virtual string print_params(std::tuple<Params...> params) = 0;

		//virtual void remove_non_important_ticks(std::vector<tick>& ticks) = 0;

		double calculate_fitness(const std::vector<tick>& ticks, std::tuple<Params...> params){
			
			std::shared_ptr<algo> algo_p = get_optimizable_algo(params);

			double ttl_ret = 0;
			int no_win = 0;
			int no_loss = 0;
			double max_dd = 0;
			double crr_dd = 0;
			double crr_max_ttl_ret = 0;

			for (tick t : ticks){
				
				position close_pos;
				algo_p->process_tick(t, close_pos);

				if (close_pos.type != signal::NONE){

					bool is_buy = close_pos.type == signal::BUY;
					double closeRate = !is_buy ? close_pos.close_tick.ask : close_pos.close_tick.bid;
					double ret = (is_buy ? 1 : -1) * (closeRate - close_pos.open_rate);

					if (ret >= 0){

						no_win++;
					}
					else {
						
						no_loss++;
					}

					ttl_ret += ret;

					if (ttl_ret >= crr_max_ttl_ret){
					
						crr_max_ttl_ret = ttl_ret;
						crr_dd = 0;
					}
					else{
					
						crr_dd = crr_max_ttl_ret - ttl_ret;
						if (crr_dd > max_dd)
							max_dd = crr_dd;
					}
				}
			}
			
			if (no_win < 231) return 0; //462

			if (no_loss < 1) no_loss = 1;
			if (no_win < 1) no_win = 1;
			
			return (ttl_ret * no_win / no_loss) - max_dd;
		}
	};
}

#endif