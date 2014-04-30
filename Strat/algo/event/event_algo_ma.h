

#ifndef _STRAT_EVENT_ALGO_MA
#define _STRAT_EVENT_ALGO_MA

#include "event_algo.h"
#include "indicator\sma.h"
#include "trend.h"

#include <string>
#include <deque>

using std::string;

namespace strat{

	class event_algo_ma : public event_algo{
	private:

		std::deque<double> _ma_lookback_q;
		const int _ma_lookback;
		sma _sma;
		trend _trend;
		trend_type _trend_type;

		void _process_ma(const tick& crr_tick){
		
			_ma_lookback_q.push_back(_sma.push(crr_tick.close));
			if (_ma_lookback_q.size() > _ma_lookback)
				_ma_lookback_q.pop_front();
			
			double slope;
			std::vector<double> x(_ma_lookback_q.begin(), _ma_lookback_q.end());
			_trend_type = _trend.get_trend(x, slope);
		}

	protected:

		void _set_algo_name() override{

			_name = "algo" + std::to_string(_sma.get_lookback()) + "-" + std::to_string(_ma_lookback);
		}

		signal _get_signal_algo(const tick& crr_tick) override {

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
					LOG(_name << ":remove obser. curr close: " << crr_tick.close << " obser close: " << front_tick.close << " sd: " << _run_sd
						<< " trend: " << _trend_type);
				}
			}

			return ret_sig;
		}

		//duplicate code as event_algo
		int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss) override		{

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

#pragma region constructors

		event_algo_ma(const std::string symbol_base, const std::string symbol_target,
			string event_f_path, size_t obser_win, size_t hold_win, double run_sd,
			int ma_period, int ma_lookback, double trend_slope_t = 1) :
			event_algo(symbol_base, symbol_target, event_f_path, obser_win, hold_win, run_sd),
			_ma_lookback(ma_lookback), _sma(ma_period), _trend(ma_lookback, trend_slope_t){

			_set_algo_name();
			LOG(_name << ": " << get_event_queue().size() << " events enqueued");
		}

		event_algo_ma(const std::string symbol_base, const std::string symbol_target,
			std::queue<boost::posix_time::ptime> event_queue, size_t obser_win, size_t hold_win,
			double run_sd, int ma_period, int ma_lookback, double trend_slope_t = 1) :
			event_algo(symbol_base, symbol_target, event_queue, obser_win, hold_win, run_sd),
			_ma_lookback(ma_lookback), _sma(ma_period), _trend(ma_lookback, trend_slope_t){
			
			_set_algo_name();
			LOG(_name << ": " << get_event_queue().size() << " events enqueued");
		}

		~event_algo_ma(){};

#pragma endregion

		signal process_tick(const tick& crr_tick, position& close_pos, double stop_loss = -1) override{

			LOG_SEV("processing tick " << crr_tick.time_stamp, logger::debug);

			_process_ma(crr_tick);

			signal ret_sig = event_algo::process_tick(crr_tick, close_pos, stop_loss);

			return ret_sig;
		}

		void init_ma_queue(const std::vector<int>& close_s){
		
			for (int i : close_s){
			
				_ma_lookback_q.push_back(_sma.push(i));
				if (_ma_lookback_q.size() > _ma_lookback)
					_ma_lookback_q.pop_front();
			}
		}

		int get_threshold1() const override{
			return _sma.get_lookback();
		}

		int get_threshold2() const override{
			return _ma_lookback;
		}
	};
}

#endif