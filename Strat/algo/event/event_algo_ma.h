/*
idea principle: price tend to move dramatically after a ecnomic event and revert to mean after a cool down period
implementation: a. fixed observe period to observe the volatitly and fixed hold period to exit;
								b. only entry when there is a historical trend formed as price more likely rever 
										to mean where there is a anti trend movement after a ecnomic event annouced;
*/


#ifndef _STRAT_EVENT_ALGO_MA
#define _STRAT_EVENT_ALGO_MA

#include "event_algo.h"
#include "indicator\sma.h"
#include "trend.h"

#include <string>
#include <deque>

using std::string;

namespace strat{

	class event_algo_ma : public event_algo {
	private:

		std::deque<double> _ma_lookback_q;
		const int _ma_lookback;
		sma _sma;
		trend _trend;
		trend_type _trend_type;

		void _process_ma(const tick& crr_tick){
		
			_ma_lookback_q.push_back(_sma.push(crr_tick.last));
			if (_ma_lookback_q.size() > _ma_lookback)
				_ma_lookback_q.pop_front();
			
			double slope;
			std::vector<double> x(_ma_lookback_q.begin(), _ma_lookback_q.end());
			_trend_type = _trend.get_trend(x, slope);
		}

	protected:

		signal _get_signal_algo(const tick& crr_tick) override {

			signal ret_sig = signal::NONE;

			//assumption: only one event exist at any particular time(min), therefore only one signal will be returned here. ie. not signal conflict
			if (!_obser_tick_q.empty()){

				tick front_tick = _obser_tick_q.front();
				if (crr_tick.time >= front_tick.time + boost::posix_time::minutes(_obser_win)){

					if (crr_tick.last >= front_tick.last + _run_sd && _trend_type == trend_type::DOWN){

						ret_sig = signal::SELL;
						_add_position(crr_tick, ret_sig, front_tick);
					}
					else if (crr_tick.last <= front_tick.last - _run_sd && _trend_type == trend_type::UP){

						ret_sig = signal::BUY;
						_add_position(crr_tick, ret_sig, front_tick);
					}

					_pop_obser_tick_queue();
				}
			}

			return ret_sig;
		}

		//duplicate code as event_algo
		int _close_position_algo(const tick& crr_tick, position& close_pos, 
			double stop_loss, const double take_profit) override {

			bool is_stop_out = stop_loss != -1 && (_position.open_tick.last - crr_tick.last) * _position.type > stop_loss;

			if (is_stop_out ||
				crr_tick.time >= _position.open_tick.time + boost::posix_time::minutes(_hold_win))
			{
				_position.close_tick = crr_tick;
				close_pos = _position;
				_delete_position();
				return 1;
			}

			return 0;
		}

	public:

#pragma region constructors

		event_algo_ma(const std::string symbol,
			string event_f_path, size_t obser_win, size_t hold_win, double run_sd,
			int ma_period, int ma_lookback, double trend_slope_t = 0.7) :
			event_algo(symbol, event_f_path, obser_win, hold_win, run_sd),
			_ma_lookback(ma_lookback), _sma(ma_period), _trend(ma_lookback, trend_slope_t){
		}

		~event_algo_ma(){};

#pragma endregion

		signal process_tick(const tick& crr_tick, position& close_pos, double& risk_lev,
			double stop_loss = -1, const double take_profit = -1) override{

			_process_ma(crr_tick);

			signal ret_sig = event_algo::process_tick(crr_tick, close_pos, risk, stop_loss, take_profit);

			return ret_sig;
		}
	};
}

#endif