// Supported with union (c) 2020 Union team
// Union HEADER file


#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <fstream>
#include <chrono>

#include <dinput.h>
#include <thread>
#include <memory>
#include <string>
#include <mutex>

#include <regex>
#include <random> 
#include <iomanip> 
#include <sstream>
#include <numeric>
#include <stack>

using namespace std;

namespace GOTHIC_ENGINE {
	// Add your code here . . .

#define ToStr (zSTRING)
#define printWin(a) { if (ogame && ogame->game_text) {ogame->game_text->Printwin(ToStr a); }};
#define printWinC(a) { if (ogame && ogame->game_text) {ogame->game_text->Printwin(ToStr a); cmd << a << endl; }};


	std::chrono::steady_clock::time_point begin_time[70];
	double perf[70];

	void RX_Begin(int index) {
		begin_time[index] = std::chrono::steady_clock::now();
	}

	void RX_End(int index) {
		perf[index] = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - begin_time[index]).count());
	}

	void RX_EndNano(int index) {
		perf[index] = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - begin_time[index]).count());
	}

	zSTRING RX_PerfString(int index) {
		return  zSTRING(perf[index] / 1000, 10) + zSTRING(" ms");
	}

	zSTRING RX_PerfStringNano(int index) {
		return  zSTRING(perf[index] / 1e6, 10) + zSTRING(" ms");
	}

	bool OnLevelFullLoaded_Once = false;
	float visualLoadBVHTimeThisFrame = 0.0f;


#define RX_Perf_Start_Inner(name, type)    /* nothing */
#define RX_Perf_End_Inner(name)            /* nothing */
}