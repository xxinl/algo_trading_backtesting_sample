// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <fstream>
#include <sstream>
#include <tuple>   
#include <algorithm>
#include <ppl.h>
#include <concurrent_vector.h>
#include <utility>
#include <mutex>
//#include <deque>
#include <future>
#include <cmath>
#include <oaidl.h>
#include <comutil.h>
#include <exception>
#include <iostream>
#include <thread>
#include <functional>

#include <boost/date_time.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/algorithm/string.hpp> 

//#include <dlib/svm.h>

//#include <vld.h>

using std::string;