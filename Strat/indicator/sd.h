//
//
//#ifndef _STRAT_SD
//#define _STRAT_SD
//
//#include "indicator\indicator.h"
//
//#include <string>
//#include <queue>
//
//using std::string;
//
//namespace strat{
//
//	class sd : indicator<double> {
//
//	private:
//
//		double _sum;
//		const int _look_back_n;
//		std::queue<double> _seriers_q;
//
//	public:
//
//		sd(int look_back_n) : _look_back_n(look_back_n), _sum(0){
//		}
//
//		double push(double v){
//			
//			if (!_seriers_q.empty() && _seriers_q.size() == _look_back_n){
//
//				_sum = _sum - _seriers_q.front() + v;
//				_seriers_q.pop();
//			}
//			else{
//
//				_sum += v;
//			}
//
//			_seriers_q.push(v);
//
//			return get_value();
//		}
//
//		/// -1 invalid value, not enough notes to calculate
//		double get_value() override {
//			
//			if (_seriers_q.size() < _look_back_n) return -1;
//
//			return _sum / _look_back_n;
//		}
//
//		int get_lookback() const{
//			
//			return _look_back_n;
//		}
//	};
//}
//
//#endif