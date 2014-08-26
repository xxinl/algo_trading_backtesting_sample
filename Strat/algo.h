
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

		const string _symbol;

		position _position;

		virtual void _open_position(const tick& t, const signal type){

			_position.open(t, type);

			LOG("opened position at tick " << t.time);		
		}

		void _delete_position(){

			LOG("closing position at tick " << _position.open_tick.time);

			_position.clear();
		}

		double _calc_profit(const tick& crr_tick) const{
		
			if (has_open_position()){

				double closeRate = _position.type == signal::SELL ? crr_tick.ask : crr_tick.bid;
				return (closeRate - _position.open_rate) * _position.type;
			}

			return 0;
		}

		bool _is_stop_out(const double profit, const double stop_loss) const{
					
			return stop_loss != -1 && (0 - profit) > stop_loss;
		}

		bool _is_stop_out(const tick& crr_tick, const double stop_loss) const{
			
			double profit = _calc_profit(crr_tick);
			return _is_stop_out(profit, stop_loss);
		}

		virtual signal _get_signal_algo(const tick& crr_tick) = 0;
		virtual int _close_position_algo(const tick& crr_tick, position& close_pos, 
			double stop_loss, const double take_profit) = 0;

	public:

		virtual ~algo() {}

		algo(const string symbol) :	_symbol(symbol){};

		//stop_loss and take_profit are pips level. 
		//  -1: ignore
		virtual signal process_tick(const tick&, position& close_pos, double& risk_lev,
			const double stop_loss = -1, const double take_profit = -1) = 0;

		position get_position() const{

			return _position;
		}

		bool has_open_position() const{

			return !_position.is_empty();
		}
	};
}

#endif