// Supported with union (c) 2020 Union team
// Union HEADER file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

	enum AB_DebugLevel
	{
		BASIC = 1,
		NORMAL = 2,
		HIGH = 3,
		DEBUG = 4,
	};

	// вывод линий на экран, например, трейсинга
	struct DebugPointEntry
	{
		zVEC3 origin;
		zVEC3 target;
		zCOLOR color;
		int time;
		zSTRING textStart;
		zSTRING textEnd;
		bool useZBuffer;
		bool isBbox;
		zTBBox3D bbox;

		DebugPointEntry::DebugPointEntry()
		{
			isBbox = false;
		}
	};


	class AB_Debug
	{
	public:
		zSTRING fileName;
		zSTRING fileNameABLog;
		int level;
		bool showTraceLines;
	public:
		bool canWriteZSpy;
		zCView* viewText;


		zCArray<DebugPointEntry*> pListPoints;

	public:
		void Write(zSTRING msg, bool close = true, bool isScript = false, int level = AB_DebugLevel::HIGH, bool addStrMes = true);
		void Init();
		void Loop();
		void WriteAni(zSTRING msg, bool close);

		void CleanLines();
		DebugPointEntry* AddLine(zVEC3 pos, zVEC3 point, zCOLOR color = GFX_RED, float timeSeconds = 15000, bool useZBuffer = false, zSTRING textStart = "", zSTRING textEnd = "");
		DebugPointEntry* AddRay(zVEC3 pos, zVEC3 point, zCOLOR color = GFX_RED, float timeSeconds = 15000, bool useZBuffer = false, zSTRING textStart = "", zSTRING textEnd = "");
		DebugPointEntry* AB_Debug::AddSphere(zVEC3 center, float radius, zCOLOR color, float timeSeconds = 15000, bool useZBuffer = false, zSTRING textStart = "", zSTRING textEnd = "");
		DebugPointEntry* AB_Debug::AddVerticalLine(zVEC3 pos, float height, zCOLOR color, float timeSeconds = 15000, bool useZBuffer = false, zSTRING textStart = "", zSTRING textEnd = "");
		DebugPointEntry* AB_Debug::AddTriangle(zVEC3 a, zVEC3 b, zVEC3 c, zCOLOR color, float timeSeconds = 15000, bool useZBuffer = false, zSTRING textStart = "", zSTRING textEnd = "");
		DebugPointEntry* AddPlane(zTPlane* plane, float squareSize, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart = "", zSTRING textEnd = "");
		void ManageLines();
		DebugPointEntry* AB_Debug::AddBbox(zTBBox3D bbox, zCOLOR color, float timeSeconds = 15000, bool useZBuffer = false, zSTRING textStart = "", zSTRING textEnd = "");
		DebugPointEntry* AB_Debug::AddCircle(zVEC3 pos, float height, float radius, int num, zCOLOR color = GFX_RED, float timeSeconds = 15000, bool useZBuffer = false, zSTRING textStart = "", zSTRING textEnd = "");
		DebugPointEntry* AB_Debug::AddCircle2(zVEC3 pos, float height, float radius, int num, zCOLOR color = GFX_RED, float timeSeconds = 15000, bool useZBuffer = false, zSTRING textStart = "", zSTRING textEnd = "");


		DebugPointEntry* AddAxis(zVEC3 posStart = zVEC3(0, 0, 0), float lineSize = 200, float timeSeconds = 15000, bool useZBuffer = false, zSTRING textStart = "", zSTRING textEnd = "");
	};
}