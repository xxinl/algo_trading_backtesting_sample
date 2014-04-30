
#ifdef _DEBUG

#include "stdafx.h"
#include "CppUnitTest.h"

#include "util.h"
#include "algo\event\event_long_short.h"
#include "algo\event\event_algo_ma.h"
#include "logger.h"
#include "indicator\sma.h"
#include "trend.h"

#include <vector>
#include <string>
#include <iostream>
#include <thread>

#include "boost\date_time.hpp"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Strat
{
	TEST_CLASS(unit_test)
	{
	public:
		
		TEST_METHOD(util_read_csv)
		{
			std::vector<std::vector<std::string>> vec;
			std::vector<int> cols_v{ 0, 1 };
			util::read_csv("../../test_files/Calendar-11-24-2013.csv", vec, cols_v);
			
			size_t size = 28;
			Assert::AreEqual(size, vec.size());
			size = 2;
			Assert::AreEqual(size, vec[0].size());

			Assert::AreEqual(std::string("Mon Nov 25"), vec[9][0]);
			Assert::AreEqual(std::string("23:50"), vec[9][1]);
		}

		TEST_METHOD(util_convert_to_dt)
		{
			boost::posix_time::ptime t1 = 
				boost::posix_time::time_from_string(std::string("2013-11-25 23:50:00.000"));
			boost::posix_time::ptime t2 = util::convert_to_dt(std::string("2013 Mon Nov 25 23:50"));
			Assert::IsTrue(t1 == t2);
		}
		
#pragma region "online run sd test - commentted out"

		//TEST_METHOD(util_get_run_sd)
		//{
		//	std::vector<double> data = { 10.0073665, 0.3597628, 3.3309777, 11.2367244,
		//		6.8589224, 4.9200999, 11.0411543, 11.6657897, 9.2686672, 12.0793347 };

		//	int look_back = 5;

		//	//sd for first 5 in data
		//	double sd = 4.541819;
		//	double mean = 6.358751;
		//	double M2 = sd * sd * (look_back - 1);

		//	//sd for first 6 in data
		//	double sd_6 = 4.064902;
		//	//sd for first 7 in data
		//	double sd_7 = 3.568893;
		//	//sd for first 8 in data
		//	double sd_8 = 3.057819;
		//	//sd for first 9 in data
		//	double sd_9 = 2.840094;
		//	//sd for first 10 in data
		//	double sd_10 = 2.928849;

		//	sd = util::get_run_sd(mean, data[5], look_back, M2);
		//	Assert::AreEqual(sd_6, sd);

		//	sd = util::get_run_sd(mean, data[6], look_back, M2);
		//	Assert::AreEqual(sd_7, sd);

		//	sd = util::get_run_sd(mean, data[7], look_back, M2);
		//	Assert::AreEqual(sd_8, sd);

		//	sd = util::get_run_sd(mean, data[8], look_back, M2);
		//	Assert::AreEqual(sd_9, sd);

		//	sd = util::get_run_sd(mean, data[9], look_back, M2);
		//	Assert::AreEqual(sd_10, sd);
		//}

#pragma endregion

		TEST_METHOD(event_long_short_constructor)
		{
			strat::event_long_short algo("eur", "usd", "../../test_files/Calendar-11-24-2013.csv"
				, 5, 15, 0.0003);

			std::queue<boost::posix_time::ptime> event_q = algo.get_event_queue();

			boost::posix_time::ptime t =
				boost::posix_time::time_from_string(std::string("2013-11-25 04:00:00.000"));
			Assert::IsTrue(t == event_q.front());

			event_q.pop();
			event_q.pop();
			t = boost::posix_time::time_from_string(std::string("2013-11-25 15:30:00.000"));
			Assert::IsTrue(t == event_q.front());
		}

		TEST_METHOD(util_read_tick_csv)
		{
			std::vector<strat::tick> tick_vec;
			util::read_tick_csv("../../test_files/EURUSD_min_11-24-2013.csv", tick_vec);

			size_t size = 2938;
			Assert::AreEqual(tick_vec.size(), size);
			boost::posix_time::ptime t =
				boost::posix_time::time_from_string(std::string("2013-11-24 23:10:00.000"));
			Assert::IsTrue(tick_vec[9].time_stamp == t);
			Assert::AreEqual(tick_vec[9].close, 1.3549);

		}

		TEST_METHOD(event_long_short_process_tick)
		{
			strat::event_long_short algo("eur", "usd", "../../test_files/Calendar-11-24-2013.csv"
				, 5, 15, 0.0003, true);

			std::vector<strat::tick> tick_vec;
			util::read_tick_csv("../../test_files/EURUSD_min_11-24-2013.csv", tick_vec);

			strat::position close_pos;
			std::queue<strat::tick> obser_q;
			strat::signal sig = strat::signal::NONE;

			for (int i = 0; i < 10; i++){
				algo.process_tick(tick_vec[i], close_pos);
				obser_q = algo.get_obser_tick_queue();
				Assert::IsTrue(obser_q.empty());
				Assert::IsTrue(strat::signal::NONE == sig);
			}

			//index no here = 'row index in file' - 2 (exclude header and c++ is 0 start index)
			sig = algo.process_tick(tick_vec[298], close_pos);
			obser_q = algo.get_obser_tick_queue();
			Assert::IsTrue(obser_q.empty());
			Assert::IsTrue(close_pos.type == strat::signal::NONE);
			Assert::IsTrue(strat::signal::NONE == sig);

			sig = algo.process_tick(tick_vec[299], close_pos);
			obser_q = algo.get_obser_tick_queue();
			size_t size = 1;
			Assert::AreEqual(obser_q.size(), size);
			Assert::IsTrue(close_pos.type == strat::signal::NONE);
			Assert::IsTrue(strat::signal::NONE == sig);

			sig = algo.process_tick(tick_vec[304], close_pos);
			obser_q = algo.get_obser_tick_queue();
			Assert::IsTrue(close_pos.type == strat::signal::NONE);
			Assert::IsTrue(strat::signal::BUY == sig);
			std::list<strat::position> pos = algo.get_positions();
			Assert::IsTrue(pos.size() == size);
			Assert::IsTrue(pos.front().open_tick.close == 1.3540);

			sig = algo.process_tick(tick_vec[319], close_pos);
			obser_q = algo.get_obser_tick_queue();
			Assert::IsTrue(obser_q.empty());
			Assert::IsTrue(strat::signal::NONE == sig);
			Assert::IsTrue(close_pos.type != strat::signal::NONE);
			pos = algo.get_positions();
			Assert::IsTrue(pos.empty());
		}

#pragma region test log

		static void test_log(std::string t){

			const boost::posix_time::ptime now =
				boost::posix_time::second_clock::local_time();

			LOG(t << " A normal severity message, will not pass to the file at " << now);
			LOG_SEV(t << " A warning severity message, will pass to the file at " << now,
				logger::warning);
			LOG_SEV(t << " A error severity message, will pass to the file at " << now,
				logger::error);

			std::list<strat::position> pos;
			strat::position p;
			
			strat::tick tick1;
			tick1.time_stamp = now;
			tick1.close = 1.006;

			p.open_tick = tick1;
			p.close_tick = tick1;

			pos.push_back(p);
			pos.push_back(p);

			LOG_POSITIONS(pos, 3, 3);
		}

		//https://github.com/boostorg/log/blob/master/example/basic_usage/main.cpp
		TEST_METHOD(logging){

			std::thread t1(&Strat::unit_test::test_log, "thread1 ");
			std::thread t2(&Strat::unit_test::test_log, "thread2 ");

			t1.join();
			t2.join();
		}

#pragma endregion

		TEST_METHOD(event_long_short_process_tick_with_stoploss)
		{
			strat::event_long_short algo("eur", "usd", "../../test_files/Calendar-11-24-2013.csv"
				, 5, 15, 0.0003, true);

			std::vector<strat::tick> tick_vec;
			util::read_tick_csv("../../test_files/EURUSD_min_11-24-2013.csv", tick_vec);

			strat::position close_pos;
			std::queue<strat::tick> obser_q;
			strat::signal sig = strat::signal::NONE;

			for (int i = 0; i < 10; i++){
				algo.process_tick(tick_vec[i], close_pos);
				obser_q = algo.get_obser_tick_queue();
				Assert::IsTrue(obser_q.empty());
				Assert::IsTrue(strat::signal::NONE == sig);
			}

			//index no here = 'row index in file' - 2 (exclude header and c++ is 0 start index)
			sig = algo.process_tick(tick_vec[298], close_pos);
			obser_q = algo.get_obser_tick_queue();
			Assert::IsTrue(obser_q.empty());
			Assert::IsTrue(close_pos.type == strat::signal::NONE);
			Assert::IsTrue(strat::signal::NONE == sig);

			sig = algo.process_tick(tick_vec[299], close_pos);
			obser_q = algo.get_obser_tick_queue();
			size_t size = 1;
			Assert::AreEqual(obser_q.size(), size);
			Assert::IsTrue(close_pos.type == strat::signal::NONE);
			Assert::IsTrue(strat::signal::NONE == sig);

			sig = algo.process_tick(tick_vec[304], close_pos);
			obser_q = algo.get_obser_tick_queue();
			Assert::IsTrue(close_pos.type == strat::signal::NONE);
			Assert::IsTrue(strat::signal::BUY == sig);
			std::list<strat::position> pos = algo.get_positions();
			Assert::IsTrue(pos.size() == size);
			Assert::IsTrue(pos.front().open_tick.close == 1.3540);

			sig = algo.process_tick(tick_vec[317], close_pos, 0.0003);
			obser_q = algo.get_obser_tick_queue();
			Assert::IsTrue(obser_q.empty());
			Assert::IsTrue(strat::signal::NONE == sig);
			Assert::IsTrue(close_pos.type != strat::signal::NONE);
			pos = algo.get_positions();
			Assert::IsTrue(pos.empty());
		}

		TEST_METHOD(sma){ 

			strat::sma ma(5);

			//mean of last 5 = 0.654106
			std::vector<double> t = { 0.1538291, 0.7083242, 0.9330954, 0.7555677, 0.1090589, 1.2543208, 0.7245816, 0.3999856, 0.4196101, 0.4720318 };
			for (std::vector<double>::iterator it = t.begin(); it != t.end(); ++it){
			
				ma.push(*it);
			}

			Assert::IsTrue(ma.get_value() - 0.654106 < 0.000001);
		}

		TEST_METHOD(trend){
		
			strat::trend trd(10);

			//slope = -0.004669211
			//std::vector<double> t = { 0.1538291, 0.7083242, 0.9330954, 0.7555677, 0.1090589, 1.2543208, 0.7245816, 0.3999856, 0.4196101, 0.4720318 };
			//y = 2x + 10 +- error(1)
			std::vector<double> t = { 11, 15, 15, 19, 19, 23, 23, 27, 27, 31 };
			double slope = 0;
			strat::trend_type type = trd.get_trend(t, slope);

			Assert::IsTrue(slope/1000000 - 2 < 0.1);
			Assert::IsTrue(type == strat::trend_type::UP);
		}

		TEST_METHOD(event_ma_process_tick){

			strat::event_algo_ma algo("eur", "usd", "../../test_files/Calendar-11-24-2013.csv",
				 5, 15, 0.0003, 15, 60);

			std::vector<strat::tick> tick_vec;
			util::read_tick_csv("../../test_files/EURUSD_min_11-24-2013.csv", tick_vec);

			strat::position close_pos;
			std::queue<strat::tick> obser_q;
			strat::signal sig = strat::signal::NONE;

			for (int i = 0; i < 10; i++){
				algo.process_tick(tick_vec[i], close_pos);
				obser_q = algo.get_obser_tick_queue();
				Assert::IsTrue(obser_q.empty());
				Assert::IsTrue(strat::signal::NONE == sig);
			}

			size_t size = 1;
			for (int i = 100; i < 320; i++){
				//index no here = 'row index in file' - 2 (exclude header and c++ is 0 start index)
				sig = algo.process_tick(tick_vec[i], close_pos);

				if (i == 298){

					obser_q = algo.get_obser_tick_queue();
					Assert::IsTrue(obser_q.empty());
					Assert::IsTrue(close_pos.type == strat::signal::NONE);
					Assert::IsTrue(strat::signal::NONE == sig);
				}

				if (i == 299){

					obser_q = algo.get_obser_tick_queue();
					Assert::AreEqual(obser_q.size(), size);
					Assert::IsTrue(close_pos.type == strat::signal::NONE);
					Assert::IsTrue(strat::signal::NONE == sig);
				}

				if (i == 304){

					obser_q = algo.get_obser_tick_queue();
					Assert::IsTrue(close_pos.type == strat::signal::NONE);
					Assert::IsTrue(strat::signal::BUY == sig);
					std::list<strat::position> pos = algo.get_positions();
					Assert::IsTrue(pos.size() == size);
					Assert::IsTrue(pos.front().open_tick.close == 1.3540);
				}

				if (i == 319){

					obser_q = algo.get_obser_tick_queue();
					Assert::IsTrue(obser_q.empty());
					Assert::IsTrue(strat::signal::NONE == sig);
					std::list<strat::position> pos = algo.get_positions();
					Assert::IsTrue(close_pos.type != strat::signal::NONE);
					pos = algo.get_positions();
					Assert::IsTrue(pos.empty());
				}
			}
		}

#pragma region test mql5 export wrapper

		size_t get_algo(wchar_t* base, wchar_t* quote, wchar_t* path){

			std::wstring w_base_str = std::wstring(base);
			std::wstring w_quote_str = std::wstring(quote);
			std::wstring w_path_str = std::wstring(path);

			strat::event_algo_ma* ret_p = nullptr;

			ret_p = new strat::event_algo_ma(string(w_base_str.begin(), w_base_str.end()),
				string(w_quote_str.begin(), w_quote_str.end()),
				string(w_path_str.begin(), w_path_str.end()),
				7, 45, 0.0003, 25, 270, 0.7);

			size_t ret_add = reinterpret_cast<size_t>(ret_p);

			return ret_add;
		}

		int process_tick(size_t algo_add, wchar_t* time, double close, double stop_loss){

			std::wstring w_time_str = std::wstring(time);
			strat::signal sig = strat::signal::NONE;

			strat::tick tick;
			tick.time_stamp = boost::posix_time::time_from_string(string(w_time_str.begin(), w_time_str.end()));
			tick.close = close;

			strat::position close_pos;
			strat::event_algo_ma* algo_p = reinterpret_cast<strat::event_algo_ma*>(algo_add);
			strat::signal sig = algo_p->process_tick(tick, close_pos, stop_loss);

			return sig;
		}

		TEST_METHOD(mql5_export_wrapper){
		
			size_t algo_add = get_algo(L"eur", L"usd", L"C:\\workspace\\Strat\\back_test_files\\Calendar - 2013.csv");

			process_tick(algo_add, L"2013.04.14 05:00", 1.35, 0.01);
		}

#pragma endregion
	};
}

#endif