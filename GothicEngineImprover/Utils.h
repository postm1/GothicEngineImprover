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
//#include <immintrin.h>

using namespace std;

namespace GOTHIC_ENGINE {
	// Add your code here . . .

#define ToStr (zSTRING)
#define printWin(a) { if (ogame && ogame->game_text) {ogame->game_text->Printwin(ToStr a); }};
#define printWinC(a) { if (ogame && ogame->game_text) {ogame->game_text->Printwin(ToStr a); cmd << a << endl; }};
#define SCREEN_MAX 8192
#define M_PI            3.14159265358979323846
#define Alg_Deg2Rad(rad) (zREAL(zREAL(rad)*zREAL(M_PI)/zREAL(180.0f)))
#define ToRadians(f) ((f) * M_PI / 180.0)
#define ToDegrees(f) ((f) * 180.0 / M_PI)
#define FACTOR 81.919998
#define F(a) a * FACTOR			// 8


#define DEF_PERF_APPLY
#define DEBUG_BUILD_BVH
//#define DEBUG_LOOP_KEYS
#define DEBUG_HOOKS_TRACERAY

	zCCamera*& pCamera = *(zCCamera**)0x008D7F94;

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
		long long result;
		int lastCounter;
		int callsGlobal;
		int callsPerFrame;
		int id;
		int levelText;
		bool endCalled;
		long long timeGlobal;

		PerfStruct::PerfStruct()
		{
			type = PERF_TYPE_PER_ONCE;
			lastCounter = 0;
			result = 0;
			callsGlobal = 0;
			callsPerFrame = 0;
			timeGlobal = 0;
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

	int RAMUsed() {
		int nRam;

		PROCESS_MEMORY_COUNTERS pmc;

		GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
		nRam = pmc.WorkingSetSize / 1000;
		return nRam;
	}

	bool OnLevelFullLoaded_Once = false;
	double visualLoadBVHTimeThisFrame = 0.0f;
	bool bShowPerfTimers = true;
	int globalFrameCounter = 0;
	int globalPerfId = 0;
	int textLevelCurrent = 0;
	int globalStackDepth = 0;
	float sizeUnloatDestr = 0.0f;
	bool showModel = false;


	std::map<std::string, PerfStruct> perfArray;

	// Получаем базовые цвета (можно вынести в параметры функции)
	static const Col16 NORMAL_COLOR(CMD_WHITE);
	static const Col16 WARNING_COLOR(CMD_RED);
	static const Col16 RESET_COLOR(CMD_WHITE); // Цвет сброса
	static const Col16 GREEN_COLOR(CMD_GREEN); // Цвет сброса

	std::vector<int> flagsTraceHistory;


	// выводим инфу в консоль
	void RX_Perf_Print(PerfStruct& entry, const std::string& name) {

		if (!bShowPerfTimers)
		{
			return;
		}

		// Форматируем основное время
		const double time_ms = static_cast<double>(entry.result) / 1000000.0;
		std::ostringstream oss_time;
		oss_time << std::fixed << std::setprecision(6) << time_ms;

		// Форматируем среднее время
		double avg = 0.0;
		if (entry.callsPerFrame > 0) {
			avg = (entry.result / static_cast<double>(entry.callsPerFrame)) / 1000000.0;
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

		//cmd << "GlobalStackMaxSize: " << globalStackDepth << endl;

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


		/*
		int stats[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		cmd << "=== Trace count: " << flagsTraceHistory.size() << " ===" << endl;

		for (auto& it : flagsTraceHistory)
		{
			stats[0] += (int)(it & zTTraceRayFlags::zTRACERAY_POLY_2SIDED) != 0;
			stats[1] += (int)(it & zTTraceRayFlags::zTRACERAY_POLY_IGNORE_TRANSP) != 0;
			stats[2] += (int)(it & zTTraceRayFlags::zTRACERAY_STAT_IGNORE) != 0;
			stats[3] += (int)(it & zTTraceRayFlags::zTRACERAY_VOB_BBOX) != 0;
			stats[4] += (int)(it & zTTraceRayFlags::zTRACERAY_VOB_IGNORE) != 0;
			stats[5] += (int)(it & zTTraceRayFlags::zTRACERAY_VOB_IGNORE_CHARACTER) != 0;
			stats[6] += (int)(it & zTTraceRayFlags::zTRACERAY_VOB_IGNORE_NO_CD_DYN) != 0;
			stats[7] += (int)(it & zTTraceRayFlags::zTRACERAY_VOB_OBB) != 0;
			stats[8] += (int)(it & zTTraceRayFlags::zTRACERAY_STAT_POLY) != 0;
			stats[9] += (int)(it & zTTraceRayFlags::zTRACERAY_FIRSTHIT) != 0;

			stats[10] += (int)(it & zTTraceRayFlags::zTRACERAY_STAT_PORTALS) != 0;
			stats[11] += (int)(it & zTTraceRayFlags::zTRACERAY_POLY_TEST_WATER) != 0;

			stats[12] += (int)(it & zTTraceRayFlags::zTRACERAY_POLY_NORMAL) != 0;
			stats[13] += (int)(it & zTTraceRayFlags::zTRACERAY_VOB_TEST_HELPER_VISUALS) != 0;
			stats[14] += (int)(it & zTTraceRayFlags::zTRACERAY_VOB_IGNORE_PROJECTILES) != 0;
			//zTRACERAY_POLY_NORMAL
			//zTRACERAY_VOB_TEST_HELPER_VISUALS
			//zTRACERAY_VOB_IGNORE_PROJECTILES
		}

		cmd << "zTRACERAY_POLY_2SIDED: " << stats[0] << endl;
		cmd << "zTRACERAY_POLY_IGNORE_TRANSP: " << stats[1] << endl;
		cmd << "zTRACERAY_STAT_IGNORE: " << stats[2] << endl;
		cmd << "zTRACERAY_VOB_BBOX: " << stats[3] << endl;
		cmd << "zTRACERAY_VOB_IGNORE: " << stats[4] << endl;
		cmd << "zTRACERAY_VOB_IGNORE_CHARACTER: " << stats[5] << endl;
		cmd << "zTRACERAY_VOB_IGNORE_NO_CD_DYN: " << stats[6] << endl;
		cmd << "zTRACERAY_VOB_OBB: " << stats[7] << endl;
		cmd << "zTRACERAY_STAT_POLY: " << stats[8] << endl;
		cmd << "zTRACERAY_FIRSTHIT: " << stats[9] << endl;


		cmd << "zTRACERAY_STAT_PORTALS: " << stats[10] << endl;
		cmd << "zTRACERAY_POLY_TEST_WATER: " << stats[11] << endl;

		cmd << "zTRACERAY_POLY_NORMAL: " << stats[12] << endl;
		cmd << "zTRACERAY_VOB_TEST_HELPER_VISUALS: " << stats[13] << endl;
		cmd << "zTRACERAY_VOB_IGNORE_PROJECTILES: " << stats[14] << endl;
		*/
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

		

		auto deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(entry.end_time - entry.begin_time).count();

		entry.timeGlobal += deltaTime;

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



#ifdef DEF_PERF_APPLY
#define RX_Perf_Start(name, type) RX_Perf_Start_Inner(name, type);
#define RX_Perf_End(name) RX_Perf_End_Inner(name);
#else
#define RX_Perf_Start(name, type)    /* nothing */
#define RX_Perf_End(name)            /* nothing */
#endif



	zCView* standardView() {
		zCView* pView = new zCView(0, 0, SCREEN_MAX, SCREEN_MAX);
		pView->SetFont("FONT_OLD_10_WHITE.TGA");
		pView->SetFontColor(zCOLOR(255, 255, 255, 255));
		return pView;
	}

	inline void zClamp(int& value, int bl, int bh) {
		if (value < bl)
			value = bl;
		else if (value > bh)
			value = bh;
	}

	// Поместить вещественное value в заданные границы сверху и снизу
	inline void zClamp(float& value, float bl, float bh) {
		if (value < bl)
			value = bl;
		else if (value > bh)
			value = bh;
	}


	static int countPrintDebug = 0;
	static zCView* testView = NULL;


	void ClearPrintDebug() {
		countPrintDebug = 0;

		static int init = 0;

		if (!init) {
			init = true;
			testView = new zCView(0, 0, SCREEN_MAX, SCREEN_MAX);
			screen->InsertItem(testView);
		}

		if (testView) {
			testView->SetFontColor(zCOLOR(0, 255, 0));
			testView->ClrPrintwin();
		}

	}

	void PrintDebug(zSTRING str) {

		testView->Print(F(3), F(14) + F(3) * countPrintDebug++, str);
	}

	/*
	bool zTBBox3D::IsIntersectingAVX(const zTBBox3D& bbox3D) const
	{
		const __m256 this_min = _mm256_loadu_ps(&mins.n[VX]); // Загружаем minX,minY,minZ
		const __m256 this_max = _mm256_loadu_ps(&maxs.n[VX]); // Загружаем maxX,maxY,maxZ

		// Загружаем min/max второго бокса (невыровненная загрузка)
		const __m256 other_min = _mm256_loadu_ps(&bbox3D.mins.n[VX]);
		const __m256 other_max = _mm256_loadu_ps(&bbox3D.maxs.n[VX]);

		// Сравнение: other_max >= this_min (для всех осей)
		const __m256 cmp_ge_min = _mm256_cmp_ps(other_max, this_min, _CMP_GE_OQ);

		// Сравнение: this_max >= other_min (для всех осей)
		const __m256 cmp_ge_max = _mm256_cmp_ps(this_max, other_min, _CMP_GE_OQ);

		// Комбинируем результаты (логическое И)
		const __m256 combined = _mm256_and_ps(cmp_ge_min, cmp_ge_max);

		// Получаем битовую маску (первые 3 бита - X,Y,Z)
		const int mask = _mm256_movemask_ps(combined) & 0x07;

		// Все три оси должны пересекаться
		return (mask == 0x07);
		
	}
	*/
	/*
	bool zTBBox3D::IsIntersectingRayAVX(const zVEC3& rayOrigin, const zVEC3& rayDirection, float& scaleMin, float& scaleMax) const
	{
		constexpr float  EPS = 1.0e-4f;
		const  __m256 vEps = _mm256_set1_ps(EPS);
		const  __m256 vNegInf = _mm256_set1_ps(-FLT_MAX);
		const  __m256 vPosInf = _mm256_set1_ps(FLT_MAX);

		const __m256 vMins = _mm256_setr_ps(mins[0], mins[1], mins[2], 0.f, 0.f, 0.f, 0.f, 0.f);
		const __m256 vMaxs = _mm256_setr_ps(maxs[0], maxs[1], maxs[2], 0.f, 0.f, 0.f, 0.f, 0.f);
		const __m256 vOrigin = _mm256_setr_ps(rayOrigin[0], rayOrigin[1], rayOrigin[2], 0.f, 0.f, 0.f, 0.f, 0.f);
		const __m256 vDir = _mm256_setr_ps(rayDirection[0], rayDirection[1], rayDirection[2],
			1.f, 1.f, 1.f, 1.f, 1.f);         // фиктивные 1 — для надёжного деления


		const __m256 vAbsDir = _mm256_andnot_ps(_mm256_set1_ps(-0.0f), vDir);  // fabs(dir)
		const __m256 maskPar = _mm256_cmp_ps(vAbsDir, vEps, _CMP_LT_OQ);         // abs(dir) < EPS ?


		const __m256 maskInside =
			_mm256_and_ps(_mm256_cmp_ps(vOrigin, vMins, _CMP_GE_OQ),              // origin ≥ mins
				_mm256_cmp_ps(vOrigin, vMaxs, _CMP_LE_OQ));            // origin ≤ maxs

		const __m256 maskParReject = _mm256_andnot_ps(maskInside, maskPar);        // parallel & outside
		if (_mm256_movemask_ps(maskParReject) & 0x7)                             // достаточно 3 лэйнов
			return FALSE;


		const __m256 vInvDir = _mm256_blendv_ps(_mm256_div_ps(_mm256_set1_ps(1.f), vDir),
			_mm256_set1_ps(0.f), maskPar);   // 1/dir, а для паралл. =0

		__m256 t0 = _mm256_mul_ps(_mm256_sub_ps(vMins, vOrigin), vInvDir);       // (mins - O) / dir
		__m256 t1 = _mm256_mul_ps(_mm256_sub_ps(vMaxs, vOrigin), vInvDir);       // (maxs - O) / dir


		const __m256 maskSwap = _mm256_cmp_ps(t1, t0, _CMP_LT_OQ);
		const __m256 tt0 = _mm256_blendv_ps(t0, t1, maskSwap);
		const __m256 tt1 = _mm256_blendv_ps(t1, t0, maskSwap);


		const __m256 tMin = _mm256_blendv_ps(tt0, vNegInf, maskPar);
		const __m256 tMax = _mm256_blendv_ps(tt1, vPosInf, maskPar);


		alignas(32) float tmpMin[8], tmpMax[8];
		_mm256_storeu_ps(tmpMin, tMin);
		_mm256_storeu_ps(tmpMax, tMax);

		scaleMin = max(tmpMin[0], max(tmpMin[1], tmpMin[2]));
		scaleMax = min(tmpMax[0], min(tmpMax[1], tmpMax[2]));

		return (scaleMin <= scaleMax);
	}
	*/
}