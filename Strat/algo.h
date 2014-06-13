
#ifndef _STRAT_ALGO
#define _STRAT_ALGO

#include "tick.h"
#include "position.h"
#include "logger.h"

#include <string>

using std::string;

namespace strat{

	class algo {
		
	protected:
		string _name;
		const string _s_base;
		const string _s_quote;

		bool _is_log_off = false;

		position _position;

		virtual void _add_position(tick t, signal type){

			_position.open_tick = t;
			_position.type = type;

			if (!_is_log_off)
				LOG("opened position at tick " << t.time_stamp);			
		}

		void _delete_position(){

			if (!_is_log_off)
				LOG("closing position at tick " << _position.open_tick.time_stamp);

			_position.type = signal::NONE;
		}

		virtual signal _get_signal_algo(const tick& crr_tick) = 0;
		virtual int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss) = 0;

	public:

		virtual ~algo() {}

		algo(string s_base, string s_quote) :
			_s_base(s_base), _s_quote(s_quote){
		
			_position.type = signal::NONE;
		};

		virtual signal process_tick(const tick&, position& close_pos, double stop_loss = -1) = 0;

		position get_position() const{

			return _position;
		}

		bool toggle_log_switch(){

			_is_log_off = !_is_log_off;

			return _is_log_off;
		}

		bool has_open_position(){

			return _position.type != signal::NONE;
		}
	};
}

#endif