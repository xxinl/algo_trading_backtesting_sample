

#ifndef _STRAT_INDICATOR
#define _STRAT_INDICATOR

#include <string>

using std::string;

namespace strat{

	template <class T>
	class indicator {

	public:

		virtual T get_value() = 0;
	};
}

#endif