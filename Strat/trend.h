

#ifndef _STRAT_TREND
#define _STRAT_TREND

#include <string>
#include <numeric> 

using std::string;

namespace strat{

	enum trend_type{

		DOWN = -1,
		SIDEWAYS = 0,
		UP = 1
	};

	class trend {

	private:

		const double _threshold;
		const int _size;
		std::vector<int> _x;

	public:

		trend(int size, double threshold = 0.5) : _size(size), _threshold(threshold){
		
			for (int i = 1; i <= size; i++){
				
				_x.push_back(i++);
			}
		};

		//http ://stackoverflow.com/questions/18939869/how-to-get-the-slope-of-a-linear-regression-line-using-c
		template <typename Container>
		trend_type get_trend(Container const& y, double& slope){

			const auto n = _x.size();
			const auto s_x = std::accumulate(_x.begin(), _x.end(), 0.0);
			const auto s_y = std::accumulate(y.begin(), y.end(), 0.0);
			const auto s_xx = std::inner_product(_x.begin(), _x.end(), _x.begin(), 0.0);
			const auto s_xy = std::inner_product(_x.begin(), _x.end(), y.begin(), 0.0);
			slope = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
			
			if (slope > _threshold)
				return trend_type::UP;
			else if (slope < 0 - _threshold)
				return trend_type::DOWN;

			return trend_type::SIDEWAYS;
		}
	};
}

#endif