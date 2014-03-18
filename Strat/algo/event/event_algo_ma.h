

#ifndef _STRAT_EVENT_ALGO_MA
#define _STRAT_EVENT_ALGO_MA

#include "event_algo.h"
#include "indicator\sma.h"
#include "trend.h"

#include <string>
#include <queue>

using std::string;

namespace strat{

	class event_algo_ma : public event_algo{
	private:

		std::queue<double> _ma_lookback_q;
		const int _ma_lookback;
		sma _sma;
		trend _trend;
		trend_type _trend_type;

		void _process_ma(const tick& crr_tick, double ma){
		
			_ma_lookback_q.push(_sma.push(crr_tick.close));
			if (_ma_lookback_q.size() > _ma_lookback)
				_ma_lookback_q.pop();
				
			_trend_type = _trend.get_trend(_ma_lookback_q);
		}

	protected:

		signal _get_signal_algo(const tick& crr_tick){

			signal ret_sig = signal::NONE;

			//assumption: only one event exist at any particular time(min), therefore only one signal will be returned here. ie. not signal conflict
			if (!_obser_tick_q.empty()){

				tick front_tick = _obser_tick_q.front();
				if (crr_tick.time_stamp >= front_tick.time_stamp + boost::posix_time::minutes(_obser_win)){

					if (crr_tick.close >= front_tick.close + _run_sd && _trend_type == trend_type::DOWN){

						ret_sig = signal::SELL;
						_add_position(crr_tick, ret_sig, front_tick);
					}
					else if (crr_tick.close <= front_tick.close - _run_sd && _trend_type == trend_type::UP){

						ret_sig = signal::BUY;
						_add_position(crr_tick, ret_sig, front_tick);
					}

					_pop_obser_tick_queue();
				}
			}

			return ret_sig;
		}

		//duplicate code as event_algo
		int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss){

			for (std::list<position>::iterator it = _positions.begin(); it != _positions.end();){

				bool is_stop_out = stop_loss != -1 && (it->open_tick.close - crr_tick.close) * it->type > stop_loss;

				if (is_stop_out ||
					crr_tick.time_stamp >= it->open_tick.time_stamp + boost::posix_time::minutes(_hold_win))
				{
					it->close_tick = crr_tick;
					close_pos = *it;
					it = _delete_position(it);
				}
				else
					++it;
			}

			return 1;
		}

	public:

#pragma regino constructors

		event_algo_ma(const std::string symbol_base, const std::string symbol_target,
			string event_f_path, size_t obser_win, size_t hold_win, double run_sd) :
			event_algo(symbol_base, symbol_target, event_f_path, obser_win, hold_win, run_sd){

			_sma(7200);
			_trend(_ma_lookback);
		};

		event_algo_ma(const std::string symbol_base, const std::string symbol_target,
			std::queue<boost::posix_time::ptime> event_queue, size_t obser_win, size_t hold_win,
			double run_sd) :
			event_algo(symbol_base, symbol_target, event_queue, obser_win, hold_win, run_sd){
		
			_sma(7200);
			_trend(_ma_lookback);
		};

#pragma endregion

		signal process_tick(const tick& crr_tick, position& close_pos, double stop_loss = -1, double ma = -1){

			_process_ma(ma);

			signal ret_sig = event_algo::process_tick(crr_tick, close_pos, stop_loss);

			return ret_sig;
		}

		/// Destructor
		~event_algo_ma(){};
	};
}

#endif