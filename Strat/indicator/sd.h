

#ifndef _STRAT_SD
#define _STRAT_SD

#include "indicator\indicator.h"

#include <string>
#include <vector>

using std::string;

namespace strat{

	class sd : indicator<double> {

	private:

		double _sum;
		const int _look_back_n;
		std::vector<double> _seriers;

	public:

		sd(int look_back_n) : _look_back_n(look_back_n), _sum(0){
		}

		void push(double v){

			if (!_seriers.empty() && _seriers.size() == _look_back_n){

				_sum = _sum - _seriers[0] + v;

				_seriers.erase(_seriers.begin());
			}
			else{

				_sum += v;
			}

			_seriers.push_back(v);
		}

		/// -1 invalid value, not enough notes to calculate
		double get_value() override {

			if (_seriers.size() != _look_back_n) return -1;

			double mean = _sum / _look_back_n;
			double sum_d = 0.0;

			for (int i = 0; i<_look_back_n; ++i)
				sum_d += (_seriers[i] - mean) * (_seriers[i] - mean);

			return std::sqrt(sum_d / _look_back_n);
		}

		/// -1 invalid value, not enough notes to calculate
		double get_mean() const {

			if (_seriers.size() != _look_back_n) return -1;

			return _sum / _look_back_n;
		}

		void reset(){

			_seriers.clear();
			_sum = 0;
		}
	};
}

#endif