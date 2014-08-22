
#ifndef _STRAT_ALGO_BAR
#define _STRAT_ALGO_BAR

#include "algo.h"
#include "bar.h"
#include "bar_watcher.h"

#include <string>

#include <boost/date_time.hpp>

//#include <ppl.h>
//#include <concurrent_vector.h>

using std::string;

namespace strat{
	
	class algo_bar : public algo {

	private:

		//concurrency::concurrent_vector<bar_watcher> _bar_watchers;
		std::vector<bar_watcher> _bar_watchers;

	protected:

		void _process_bar_tick(const tick& crr_tick) {

			//concurrency::parallel_for_each(std::begin(_bar_watchers), std::end(_bar_watchers),
			//	[&](bar_watcher& watcher){

			//	watcher.on_tick(crr_tick);
			//});

			for (std::vector<bar_watcher>::iterator it = _bar_watchers.begin(); it != _bar_watchers.end(); ++it){

				it->on_tick(crr_tick);
			}
		};

		void _attach_watcher(const bar_watcher& watcher){
		
			_bar_watchers.push_back(watcher);
		}

		bool _is_on_new_bar_tick(bar_interval type) {
		
			//for (concurrency::concurrent_vector<bar_watcher>::iterator it = _bar_watchers.begin(); 
			//	it != _bar_watchers.end(); ++it){

			//	if (it->interval == type && it->is_on_new_bar_tick())
			//		return true;
			//}

			for (std::vector<bar_watcher>::iterator it = _bar_watchers.begin();
				it != _bar_watchers.end(); ++it){

				if (it->interval == type && it->is_on_new_bar_tick())
					return true;
			}

			return false;
		}

	public:

		virtual ~algo_bar() {}

		algo_bar(const string symbol) :
			algo(symbol){	};
	};
}

#endif