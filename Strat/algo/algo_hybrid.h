/*
idea principle: day_range + bollinger where day_range has higher priority
implementation: 
*/


#ifndef _STRAT_HYBRID
#define _STRAT_HYBRID

#include "algo.h"
#include "algo\algo_dayrange.h"

#include <vector>

using std::string;

namespace strat{

	class algo_hybrid : public algo {

	private:

		std::vector<std::shared_ptr<strat::algo>> _algos;

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {

			return  signal::NONE;
		}

		int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss) override{

			return 0;
		}

	public:

#pragma region constructors

		algo_hybrid(const string s_base, const string s_quote,			
			int complete_hour, double entry_lev, double exit_lev,
			int complete_hour2, double entry_lev2, double exit_lev2) :
			algo(s_base, s_quote){

			//algo with higher priority push in first
			_algos.push_back(std::make_shared<strat::algo_dayrange>(s_base, s_quote,
				complete_hour, entry_lev, exit_lev));
		};
		
		/// Destructor
		~algo_hybrid(){};

#pragma endregion

		signal process_tick(const tick& crr_tick, position& close_pos, double stop_loss = -1) override{

			signal sig = signal::NONE;

			bool has_pos = false;
			for (int i = 0; i < _algos.size(); i++){
			
				if (_algos[i]->has_open_position()){

					has_pos = true;
					return _algos[i]->process_tick(crr_tick, close_pos, stop_loss);
				}
			}

			for (std::vector<std::shared_ptr<strat::algo>>::iterator it = _algos.begin();
				it != _algos.end(); ++it){
			
				sig = (*it)->process_tick(crr_tick, close_pos, stop_loss);
				if (sig != signal::NONE)
					return sig;
			}			
		}
	};
}

#endif