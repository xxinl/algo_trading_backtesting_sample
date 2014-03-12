

#ifndef _STRAT_TRADING_ALGORITHM
#define _STRAT_TRADING_ALGORITHM

#include "tick.h"
#include "position.h"
#include "logger.h"

#include <string>

namespace strat{

	class trading_algorithm {
	private:

	protected:
		std::string _symbol_base;
		std::string _symbol_target;
		std::vector<position> _positions;

		int _add_position(const tick &t, strat::signal type){

			position pos;
			pos.open_tick = t;
			pos.type = type;
			_positions.push_back(pos);

			LOG("opened position at tick " << t.time_stamp);

			return _positions.size();
		}

		std::vector<position>::iterator _delete_position(std::vector<position>::iterator it){
			LOG("closing position at tick " << it->open_tick.time_stamp);
			return _positions.erase(it);
		}

	public:

		/// Constructor 
		trading_algorithm(const std::string symbol_base, const std::string symbol_target) :
			_symbol_base(symbol_base), _symbol_target(symbol_target){		};

		///// Destructor
		//virtual ~trading_algorithm() = 0;

		virtual signal process_tick(const tick&, std::vector<position>&) = 0;

		std::vector<position> get_positions() const{
			return _positions;
		}
	};
}

#endif