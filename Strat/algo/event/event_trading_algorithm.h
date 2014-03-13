

#ifndef _STRAT_EVENT_TRADING_ALGORITHM
#define _STRAT_EVENT_TRADING_ALGORITHM

#include "../../trading_algorithm.h"
#include "../../util.h"

#include <string>
#include <queue> 
#include <ctime>
#include <vector>

#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp> 

namespace strat{

	class event_trading_algorithm : public trading_algorithm {
	private:
		std::string _event_f_path;
		std::queue<boost::posix_time::ptime> _event_q;

		std::vector<strat::position> _closed_pos_hist;

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

		int _push_obser_queue_if_event(const tick &crr_tick){
			if (_event_q.empty()) return 0;

			boost::posix_time::ptime next_event_t = _event_q.front();

			if (next_event_t <= crr_tick.time_stamp){
				_obser_tick_q.push(crr_tick);

				LOG(_name << ":observed at tick " << crr_tick.time_stamp);

				_event_q.pop();
				return _event_q.size();
			}

			return -1;
		}

	protected:
		int _obser_win;
		int _hold_win;
		
		//int _sd_diff_back;
		double _run_sd;		

		std::queue<tick> _obser_tick_q;

		virtual signal _get_signal_algo(const tick &crr_tick) = 0;
		virtual int _close_position_algo(const tick &crr_tick, std::vector<position> &close_pos) = 0;

		void _pop_obser_tick_queue(){

			if (!_obser_tick_q.empty()){

				LOG(_name << ":removing observe " << _obser_tick_q.front().time_stamp);
				_obser_tick_q.pop();
			}
		}

	public:
		/// Constructor 
		event_trading_algorithm(const std::string symbol_base, const std::string symbol_target, 
			const char *event_f_path, int obser_win, int hold_win, double run_sd) :
			trading_algorithm(symbol_base, symbol_target), _event_f_path(event_f_path), _obser_win(obser_win), _hold_win(hold_win), _run_sd(run_sd){
			
			std::vector<std::vector<std::string>> event_v;
			std::vector<int> cols_v{ 0, 1, 3, 5 };
			util::read_csv(event_f_path, event_v, cols_v);
			
			boost::posix_time::ptime t;
			// assume event_v is in ascending order
			for (std::vector<std::vector<std::string>>::iterator it = event_v.begin(); it != event_v.end(); ++it){	

				//add 0 if date is single digit
				std::string date = (*it)[0];
				if (date.length() == (size_t)9)
					date = date.insert(8, "0");

				t = util::convert_to_dt("2013 " + date + " " + (*it)[1]);

				if (t.is_not_a_date_time()){
					LOG_SEV("failed to convert " << "2013 " + (*it)[0] + " " + (*it)[1], logger::warning);

					continue;
				}

				if (_event_q.empty() || _event_q.back() != t)	{
					std::string symbol = (*it)[2];
					boost::algorithm::to_lower(symbol);
					std::string importance = (*it)[3];
					boost::algorithm::to_lower(importance);

					if ((symbol == _symbol_base || symbol == _symbol_target)
						&& (importance == "high" || importance == "medium"))
					_event_q.push(t);
				}
			}

			LOG(_event_q.size() << " events enqueued");

			_name = "algo" + std::to_string(_obser_win) + "-" + std::to_string(_hold_win);
		};

		event_trading_algorithm(const std::string symbol_base, const std::string symbol_target,
			std::queue<boost::posix_time::ptime> event_queue, int obser_win, int hold_win, double run_sd) :
			trading_algorithm(symbol_base, symbol_target),_obser_win(obser_win), _hold_win(hold_win), _run_sd(run_sd){

			_event_q = event_queue;

			LOG(_event_q.size() << " events enqueued");

			_name = "algo" + std::to_string(_obser_win) + "-" + std::to_string(_hold_win);
		};

		///// Destructor
		//virtual ~event_trading_algorithm();
		
		signal process_tick(const tick &crr_tick, std::vector<position> &close_pos){
			
			_push_obser_queue_if_event(crr_tick);
			//_update_run_sd(crr_tick);

			if (_obser_tick_q.empty() || _positions.empty()){
			
				LOG_SEV("empty obser and position queue, abord process tick at tick " << crr_tick.time_stamp, logger::debug);
			}

			//TODO maybe new thread/parallel run
			//process existing observe queue
			signal ret_sig = _get_signal_algo(crr_tick);
			LOG_SEV(ret_sig << " signal at tick " << crr_tick.time_stamp, logger::debug);

			//TODO maybe new thread/parallel run
			//process open positions
			_close_position_algo(crr_tick, close_pos);
			LOG_SEV(close_pos.size() << " close positions returned at tick " << crr_tick.time_stamp, logger::debug);

			if (!close_pos.empty())
				_closed_pos_hist.insert(_closed_pos_hist.end(), close_pos.begin(), close_pos.end());

			//_push_hist_tick_queue(crr_tick);

			return ret_sig;
		}

		std::queue<boost::posix_time::ptime> get_event_queue() const{
			return _event_q;
		}

		std::queue<tick> get_obser_tick_queue() const{
			return _obser_tick_q;
		}

		std::vector<position> get_closed_position_history(){
			return _closed_pos_hist;
		}

		int get_obser_threshold(){
			return _obser_win;
		}

		int get_hold_threshold(){
			return _hold_win;
		}
	};
}

#endif