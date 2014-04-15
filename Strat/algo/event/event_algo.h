

#ifndef _STRAT_EVENT_ALGO
#define _STRAT_EVENT_ALGO

#include "algo.h"
#include "util.h"

#include <string>
#include <queue> 
#include <vector>
#include <list>
#include <future>

#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp> 

using std::string;

namespace strat{

	class event_algo : public algo {
	private:
		std::queue<boost::posix_time::ptime> _event_q;

		std::list<strat::position> _closed_pos_hist;
		
#pragma region "online run sd - commentted out"
		
		//double _run_mean;
		//double _run_M2;

		//std::queue<tick> _hist_tick_q;

		//int _push_hist_tick_queue(const tick &crr_tick){
		//
		//	_hist_tick_q.push(crr_tick);
		//	if (_hist_tick_q.size() > _sd_lookback)
		//		_hist_tick_q.pop();

		//	return _hist_tick_q.size();
		//}

		//double _update_run_sd(const tick &crr_tick){

		//	if (_hist_tick_q.size() >= _sd_lookback)
		//	{
		//		_run_sd = util::get_run_sd(
		//			_run_mean, crr_tick.close - _hist_tick_q.front().close, _sd_lookback, _run_M2);
		//	}

		//	return _run_sd;
		//}

#pragma endregion

		///return size of the event_q
		int _push_obser_queue_if_event(tick crr_tick){

			//delete any event in queue that has passed
			while (!_event_q.empty() && _event_q.front() < crr_tick.time_stamp)
			{
				_event_q.pop();
			}

			if (!_event_q.empty() && _event_q.front() == crr_tick.time_stamp){
				_obser_tick_q.push(crr_tick);

				LOG(_name << ":observed at tick " << crr_tick.time_stamp);

				_event_q.pop();
			}

			return _event_q.size();
		}

	protected:

		const size_t _obser_win;
		const size_t _hold_win;
		
		//int _sd_diff_back;
		double _run_sd;		

		std::queue<tick> _obser_tick_q;

		virtual void _set_algo_name() = 0;

		virtual signal _get_signal_algo(const tick& crr_tick) = 0;
		virtual int _close_position_algo(const tick& crr_tick, position& close_pos, double stop_loss) = 0;

		void _pop_obser_tick_queue(){

			if (!_obser_tick_q.empty()){

				LOG(_name << ":removing observe " << _obser_tick_q.front().time_stamp);
				_obser_tick_q.pop();
			}
		}

	public:

		event_algo(string symbol_base, string symbol_quote, 
			string event_f_path, size_t obser_win, size_t hold_win, double run_sd = 0.0003) :
			algo(symbol_base, symbol_quote), _obser_win(obser_win), _hold_win(hold_win), _run_sd(run_sd){
			
			std::vector<std::vector<std::string>> event_v;
			std::vector<int> cols_v{ 0, 1, 3, 5 };
			util::read_csv(event_f_path, event_v, cols_v);
			LOG_SEV("read event csv file. event count: " << event_v.size(), logger::debug);

			boost::posix_time::ptime t;
			// assume event_v is in ascending order
			for (std::vector<std::vector<string>>::iterator it = event_v.begin(); it != event_v.end(); ++it){

				//add 0 if date is single digit
				string date = (*it)[0];
				if (date.length() == (size_t)9)
					date = date.insert(8, "0");

				t = util::convert_to_dt("2013 " + date + " " + (*it)[1]);

				if (t.is_not_a_date_time()){

					LOG_SEV("failed to convert " << "2013 " + (*it)[0] + " " + (*it)[1], logger::warning);
					continue;
				}

				if (_event_q.empty() || _event_q.back() != t)	{

					string symbol = (*it)[2];
					boost::algorithm::to_lower(symbol);
					string importance = (*it)[3];
					boost::algorithm::to_lower(importance);

					if ((symbol == _symbol_base || symbol == _symbol_quote)
						&& (importance == "high" || importance == "medium")){

						_event_q.push(t);
					}
				}
			}
		};

		event_algo(const string symbol_base, const string symbol_target,
			std::queue<boost::posix_time::ptime> event_queue, int obser_win, int hold_win, double run_sd) :
			algo(symbol_base, symbol_target),_obser_win(obser_win), _hold_win(hold_win), _run_sd(run_sd){

			_event_q = event_queue;
		};
				
		signal process_tick(const tick& crr_tick, position& close_pos, double stop_loss = -1){

			LOG_SEV(_name << ": processing tick " << crr_tick.time_stamp, logger::debug);
			
			_push_obser_queue_if_event(crr_tick);

			if (_obser_tick_q.empty() && _positions.empty()){
			
				LOG_SEV("empty obser and position queue, abord process tick at tick " << crr_tick.time_stamp, logger::debug);
				return signal::NONE;
			}

			//process existing observe queue
			auto fut_get_sig = std::async([&]{

				return _get_signal_algo(crr_tick);
			});

			//process open positions
			auto fut_close_pos = std::async([&]{

				_close_position_algo(crr_tick, close_pos, stop_loss);
			});

			fut_get_sig.wait();
			fut_close_pos.wait();

			signal ret_sig = fut_get_sig.get();
			if (ret_sig){

				LOG_SEV(_name << ": " << ret_sig << " signal at tick " << crr_tick.time_stamp, logger::debug);
			}

			if (close_pos.type != signal::NONE)	{

				_closed_pos_hist.push_back(close_pos);

				LOG_SEV(_name << ": " << " position(" << close_pos.open_tick.time_stamp << ") closed at tick "
					<< crr_tick.time_stamp, logger::debug);
			}

			LOG_SEV(_name << ": processed " << crr_tick.time_stamp, logger::debug);

			return ret_sig;
		}

#pragma region properties gets

		std::queue<boost::posix_time::ptime> get_event_queue() const{
			return _event_q;
		}

		std::queue<tick> get_obser_tick_queue() const{
			return _obser_tick_q;
		}

		std::list<position> get_closed_position_history() const{
			return _closed_pos_hist;
		}

		void clear_closed_position_history() {
			_closed_pos_hist.clear();
		}

		virtual int get_obser_threshold() const{
			return _obser_win;
		}

		virtual int get_hold_threshold() const{
			return _hold_win;
		}

#pragma endregion
	};
}

#endif