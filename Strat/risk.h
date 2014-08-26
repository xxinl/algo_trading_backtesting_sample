

#ifndef _STRAT_RISK
#define _STRAT_RISK

#include "indicator/sd.h"

using std::string;

namespace strat{

	class risk {

	private:

		sd _sd;

	public:

		risk(int sd_look_back_n) : _sd(sd_look_back_n){
		}

		void push_return(double v){

			_sd.push(v);
		}

		/// -1 invalid value, not enough notes to calculate
		double get_risk() {

			return 2 * _sd.get_value();
		}

		void reset(){

			_sd.reset();
		}
	};
}

#endif