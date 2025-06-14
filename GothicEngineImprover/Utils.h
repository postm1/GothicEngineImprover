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


	enum PerfType
	{
		PERF_TYPE_PER_ONCE = 0, // прямой единичный замер участка кода
		PERF_TYPE_PER_SEC,
		PERF_TYPE_PER_FRAME, //замер всех вызовов участка кода за 1 кадр (суммарный)
	};

	struct PerfStruct
	{
	public:
		PerfType type;
		std::chrono::steady_clock::time_point begin_time;
		std::chrono::steady_clock::time_point end_time;
		double result;
		int lastCounter;
		int callsGlobal;
		int callsPerFrame;
		int id;
		int levelText;
		bool endCalled;

		PerfStruct::PerfStruct()
		{
			type = PERF_TYPE_PER_ONCE;
			lastCounter = 0;
			result = 0;
			callsGlobal = 0;
			callsPerFrame = 0;
			id = 0;
			levelText = 0;
			endCalled = false;
		}

		zSTRING GetTypeString()
		{
			zSTRING result = "NONE";

			switch (type)
			{
			case PERF_TYPE_PER_ONCE: result = "ONCE"; break;
			case PERF_TYPE_PER_SEC: result = "SEC"; break;
			case PERF_TYPE_PER_FRAME: result = "FRAME"; break;
			}

			return result;
		}

		void ResetFrame()
		{
			result = 0;
			callsPerFrame = 0;
		}
	};

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
	bool bShowPerfTimers = true;
	int globalFrameCounter = 0;
	int globalPerfId = 0;
	int textLevelCurrent = 0;

	std::map<std::string, PerfStruct> perfArray;

	// Получаем базовые цвета (можно вынести в параметры функции)
	static const Col16 NORMAL_COLOR(CMD_WHITE);
	static const Col16 WARNING_COLOR(CMD_RED);
	static const Col16 RESET_COLOR(CMD_WHITE); // Цвет сброса
	static const Col16 GREEN_COLOR(CMD_GREEN); // Цвет сброса


	// выводим инфу в консоль
	void RX_Perf_Print(PerfStruct& entry, const std::string& name) {

		if (!bShowPerfTimers)
		{
			return;
		}

		// Форматируем основное время
		const double time_ms = entry.result / 1000;
		std::ostringstream oss_time;
		oss_time << std::fixed << std::setprecision(6) << time_ms;

		// Форматируем среднее время
		double avg = 0.0;
		if (entry.callsPerFrame > 0) {
			avg = (entry.result / static_cast<double>(entry.callsPerFrame)) / 1000.0;
		}
		std::ostringstream oss_avg;
		oss_avg << std::fixed << std::setprecision(6) << avg;


		std::string tabs(entry.levelText, '\t');

		if (entry.levelText == 0)
		{
			cmd << endl;
		}

		cmd << tabs.c_str();

		// Начинаем вывод
		cmd << NORMAL_COLOR
			//<< "["<< entry.GetTypeString() << "] "
			<< "["
			<< GREEN_COLOR
			<< name.c_str()
			<< NORMAL_COLOR
			<< "] | ("
			<< entry.callsPerFrame << "/"
			<< entry.callsGlobal << ") | ";

		// Выводим время с цветом в зависимости от значения
		if (time_ms >= 0.02) {
			cmd << WARNING_COLOR << oss_time.str().c_str();
		}
		else {
			cmd << NORMAL_COLOR << oss_time.str().c_str();
		}

		// Продолжаем вывод
		cmd << NORMAL_COLOR << " ms | AVG: ";

		cmd << NORMAL_COLOR << oss_avg.str().c_str();

		// Завершаем строку
		cmd << NORMAL_COLOR << " ms";

		//cmd << " | " << entry.levelText;

		cmd << endl;
	}

	void RX_Perf_UpdateFrame()
	{
		if (!bShowPerfTimers)
		{
			return;
		}
		cmd << WARNING_COLOR << "================= TIMERS PERF INFO =================================================" << NORMAL_COLOR << endl;


		std::vector<std::reference_wrapper<std::pair<const std::string, PerfStruct>>> sortedEntries;
		for (auto& pair : perfArray) {
			sortedEntries.emplace_back(pair);
		}

		// 2. Сортируем
		std::sort(sortedEntries.begin(), sortedEntries.end(),
			[](auto& a, auto& b) {
				return a.get().second.id < b.get().second.id;
			}
		);

		// 3. Выводим
		for (auto& entry : sortedEntries) {
			zSTRING s = entry.get().first.c_str();
			RX_Perf_Print(entry.get().second, s.ToChar());

			entry.get().second.ResetFrame();

			if (!entry.get().second.endCalled)
			{
				Message::Box("End функции не существует: " + s);
			}
		}

		cmd << endl;
	}

	void RX_Perf_Start_Inner(char* name, PerfType type = PERF_TYPE_PER_ONCE)
	{
		auto& it = perfArray.find(name);

		if (it == perfArray.end())
		{
			PerfStruct perf;
			perf.type = type;
			perf.id = ++globalPerfId;

			perfArray[name] = perf;

			it = perfArray.find(name);
		}


		it->second.levelText = textLevelCurrent++;
		it->second.callsGlobal++;
		it->second.callsPerFrame++;


		it->second.lastCounter = globalFrameCounter;
		it->second.begin_time = std::chrono::steady_clock::now();
	}

	void RX_Perf_End_Inner(char* name)
	{
		auto timeNow = std::chrono::steady_clock::now();

		if (perfArray.find(name) == perfArray.end())
		{
			Message::Box("(RX_Perf_End) key not found: " + Z name);
			return;
		}

		auto& entry = perfArray[name];

		entry.end_time = timeNow;
		entry.endCalled = true; // проверка чтобы End функция была, чтобы если что показать ошибку
		// понижаем уровень вывода текста (смещение)
		textLevelCurrent--;


		auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(entry.end_time - entry.begin_time).count();

		if (entry.type == PERF_TYPE_PER_FRAME)
		{
			entry.result += deltaTime;
		}
		else if (entry.type == PERF_TYPE_PER_ONCE)
		{
			entry.result = deltaTime;

			RX_Perf_Print(entry, name);
		}
	}

}