/*
idea principle: day_range + bollinger where day_range has higher priority
implementation: 
*/


#ifndef _STRAT_HYBRID
#define _STRAT_HYBRID

#include "algo.h"

#include <vector>

using std::string;

namespace strat{

	class algo_hybrid : public algo {

	private:

		//algo with higher priority push in first
		std::vector<strat::algo*> _algos;
		int _has_pos_algo_index = -1;

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {

			return  signal::NONE;
		}

		int _close_position_algo(const tick& crr_tick, position& close_pos, 
			double stop_loss) override{

			return 0;
		}

	public:

#pragma region constructors

		algo_hybrid(const string symbol, std::vector<strat::algo*>& algos) :
			algo(symbol), _algos(algos){		};
		
		/// Destructor
		~algo_hybrid(){};

#pragma endregion

		signal process_tick(const tick& crr_tick, position& close_pos, double& risk_lev,
			double stop_loss = -1, const bool ignore = false) override{

			signal sig = signal::NONE;

			if (_has_pos_algo_index >= 0){
			
				for (int i = 0; i < _algos.size(); i++){

					if (_has_pos_algo_index == i && _algos[i]->has_open_position()){

						_algos[i]->process_tick(crr_tick, close_pos, risk_lev, stop_loss);
						if (close_pos.type != NONE)
							_has_pos_algo_index = -1;
					}
					else{
					
						_algos[i]->process_tick(crr_tick, close_pos, risk_lev, stop_loss, true);
					}
				}
			}
			else{
			
				for (int i = 0; i < _algos.size(); i++){

					if (_has_pos_algo_index == -1){

						sig = _algos[i]->process_tick(crr_tick, close_pos, risk_lev, stop_loss);

						if (sig != NONE)
							_has_pos_algo_index = i;
					}
					else{

						_algos[i]->process_tick(crr_tick, close_pos, risk_lev, stop_loss, true);
					}
				}
			}		

			return sig;
		}
	};
}

#endif