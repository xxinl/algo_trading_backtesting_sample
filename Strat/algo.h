

#ifndef _STRAT_ALGO
#define _STRAT_ALGO

#include "tick.h"
#include "position.h"
#include "logger.h"

#include <string>

#include <vld.h>

using std::string;

namespace strat{

	class algo {
		
	protected:
		string _name;
		const string _s_base;
		const string _s_quote;

		bool _is_log_off = false;

		std::list<position> _positions;

		virtual size_t _add_position(tick t, signal type){

			position pos;
			pos.open_tick = t;
			pos.type = type;
			_positions.push_back(pos);

			if (!_is_log_off)
				LOG("opened position at tick " << t.time_stamp);

			return _positions.size();
		}

		std::list<position>::iterator _delete_position(std::list<position>::iterator it){

			if (!_is_log_off)
				LOG("closing position at tick " << it->open_tick.time_stamp);

			return _positions.erase(it);
		}

		virtual signal _get_signal_algo(const tick& crr_tick) = 0;
		virtual int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss) = 0;

	public:

		virtual ~algo() {}

		algo(string s_base, string s_quote) :
			_s_base(s_base), _s_quote(s_quote){};

		virtual signal process_tick(const tick&, position& close_pos, double stop_loss = -1) = 0;

		std::list<position> get_positions() const{

			return _positions;
		}

		bool toggle_log_switch(){

			_is_log_off = !_is_log_off;

			return _is_log_off;
		}
	};
}

#endif