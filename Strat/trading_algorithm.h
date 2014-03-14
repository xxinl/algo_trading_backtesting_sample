

#ifndef _STRAT_TRADING_ALGORITHM
#define _STRAT_TRADING_ALGORITHM

#include "tick.h"
#include "position.h"
#include "logger.h"

#include <string>

using std::string;

namespace strat{

	class trading_algorithm {
	private:

	protected:
		string _name;
		const string _symbol_base;
		const string _symbol_quote;

		std::list<position> _positions;

		int _add_position(tick t, signal type, tick obser_t){

			position pos;
			pos.open_tick = t;
			pos.obser_tick = obser_t;
			pos.type = type;
			_positions.push_back(pos);

			LOG(_name << ":opened position at tick " << t.time_stamp);

			return _positions.size();
		}

		std::list<position>::iterator _delete_position(std::list<position>::iterator it){

			LOG(_name << ":closing position at tick " << it->open_tick.time_stamp);
			return _positions.erase(it);
		}

	public:

		/// Constructor 
		trading_algorithm(string symbol_base, string symbol_quote) :
			_symbol_base(symbol_base), _symbol_quote(symbol_quote){};

		virtual signal process_tick(const tick&, position& close_pos, double stop_loss = -1) = 0;

		std::list<position> get_positions() const{

			return _positions;
		}
	};
}

#endif