

#ifndef _STRAT_EMA
#define _STRAT_EMA

#include "indicator\indicator.h"

#include <string>
#include <queue>

using std::string;

namespace strat{

	class ema : indicator<double> {

	private:

		const int _look_back_n;
		double _alpha;
		double _crrEma;

	public:

		ema(int look_back_n) : _look_back_n(look_back_n), _crrEma(-1),
			_alpha((double)2 / (1 + _look_back_n)){}

		double push(double p){
			
			if (_crrEma == -1){
			
				_crrEma = p;
			}
			else{
			
				_crrEma = _alpha * p + (1 - _alpha) * _crrEma;
			}

			return get_value();
		}

		double get_value() override {
			
			return _crrEma;
		}

		int get_lookback() const{
			
			return _look_back_n;
		}
	};
}

#endif