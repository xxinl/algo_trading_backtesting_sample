

#ifndef _STRAT_TREND
#define _STRAT_TREND

#include <string>

#include <dlib/svm.h>

using std::string;

namespace strat{

	enum trend_type{

		DOWN = -1,
		SIDEWAYS = 0,
		UP = 1
	};

	class trend {

		typedef dlib::matrix<double, 1, 1> sample_type;

	private:

		const double _threshold;
		const int _size;
		std::vector<sample_type> _x;

	public:

		trend(int size, double threshold = 1) : _size(size), _threshold(threshold){
		
			sample_type m;
			for (int i = 1; i <= size; i++){
				
				m(0) = i;
				_x.push_back(m);
			}
		};

		//http://dlib.net/krr_regression_ex.cpp.html
		trend_type get_trend(const std::vector<double>& y, double& slope){

			// not enough points
			if (y.size() != _size) {
				return SIDEWAYS;
			}

			typedef dlib::linear_kernel<sample_type> kernel_type;
			dlib::rr_trainer<kernel_type> trainer;

			dlib::decision_function<kernel_type> test = trainer.train(_x, y);

			sample_type m;
			m(0) = 1;
			double y1 = test(m);
			m(0) = 10;
			double y10 = test(m);
			slope = (y10 - y1) * 1000000 / 9;

			if (slope > _threshold)
				return UP;
			else if (slope < 0 - _threshold)
				return DOWN;

			return SIDEWAYS;
		}

		double get_slope_threshod() const{
			
			return _threshold;
		}
	};
}

#endif