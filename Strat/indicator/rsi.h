

#ifndef _STRAT_RSI
#define _STRAT_RSI

#include "indicator\indicator.h"
#include "indicator\ema.h"

#include <string>
#include <queue>

using std::string;

namespace strat{

	/// online sma
	class rsi : indicator<double> {

	private:

		const int _look_back_n;
		ema _ema_U;
		ema _ema_D;
		double _last_p;

	public:

		rsi(int look_back_n) : _look_back_n(look_back_n), _last_p(-1),
			_ema_U(look_back_n), _ema_D(look_back_n){}

		double push(double p){
			
			if (_last_p != -1){
			
				double diff = p - _last_p;

				if (diff > 0)
					_ema_U.push(diff);
				else
					_ema_D.push(0 - diff);
			}

			_last_p = p;

			return get_value();
		}

		double get_value() override {
			
			double ema_U_v = _ema_U.get_value();
			double ema_D_v = _ema_D.get_value();

			if (ema_U_v == -1 || ema_D_v == -1) return -1;

			return 100 - 100 / (1 + ema_U_v / ema_D_v);
		}

		int get_lookback() const{
			
			return _look_back_n;
		}
	};
}

#endif