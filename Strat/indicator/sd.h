

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
								
				_seriers.erase(_seriers.begin());
			}

			_seriers.push_back(v);
		}

		/// -1 invalid value, not enough notes to calculate
		double get_value() override {
			
			if (_seriers.size() < _look_back_n) return -1;

			double mean = 0.0;
			double sum_d = 0.0;
			
			for (int i = 0; i < _look_back_n; ++i)
			{
				mean += _seriers[i];
			}
			mean = mean / _look_back_n;

			for (int i = 0; i<_look_back_n; ++i)
				sum_d += (_seriers[i] - mean) * (_seriers[i] - mean);

			return std::sqrt(sum_d / _look_back_n);
		}

		void reset(){
		
			_seriers.clear();
		}
	};
}

#endif