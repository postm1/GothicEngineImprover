// Supported with union (c) 2020 Union team
// Union HEADER file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

	// Add your code here . . .
	struct zCSubMeshStruct
	{
		zCProgMeshProto* parentProto;
		zCProgMeshProto::zCSubMesh* subMesh;
		BVH_Tree* bvhTree;

		zCSubMeshStruct::zCSubMeshStruct();

		void Clear();

		void BuildMap(zCProgMeshProto* proto, zCProgMeshProto::zCSubMesh* subMeshCurrent);

		void Render(zCVob* parent, zCProgMeshProto* proto, zCProgMeshProto::zCSubMesh* subMesh);

		void RenderNode(BVHNode* node, zCOLOR color, zCVob* parent, int& nodesCount, int depth);

	};


	struct RayCastReport
	{
		std::vector<zCProgMeshProto::zCSubMesh*> subMeshesFound;
		std::map<BVHNode*, bool> nodesStackHistory;

		int bestTreeIndexGlobal;
		float bestAlphaGlobal;
		zTPlane* foundPlaneGlobal;
		zVEC3 intersGlobal;
		bool globalFirstHitMode;
		bool hitFoundGlobal;
		int NodeTreeCheckCounter;
		int TrisTreeCheckCounter;

		zCProgMeshProto::zCSubMesh* globalSubmeshBestNew;
		zCProgMeshProto::zCSubMesh* globalSubmeshBestOld;


		RayCastReport::RayCastReport()
		{
			Clear();
		}

		void RayCastReport::Clear()
		{
			bestTreeIndexGlobal = 0;
			NodeTreeCheckCounter = 0;
			TrisTreeCheckCounter = 0;

			bestAlphaGlobal = 9999.9f;
			subMeshesFound.clear();
			nodesStackHistory.clear();

			foundPlaneGlobal = NULL;
			intersGlobal = zVEC3(0, 0, 0);
			globalFirstHitMode = false;
			hitFoundGlobal = false;

			globalSubmeshBestNew = NULL;
			globalSubmeshBestOld = NULL;
		}

		void RayCastReport::ClearCurrrentTrace()
		{
			NodeTreeCheckCounter = 0;
			TrisTreeCheckCounter = 0;
			nodesStackHistory.clear();
		}

	} raycastReport;

	std::unordered_map<zCProgMeshProto::zCSubMesh*, zCSubMeshStruct> pTraceMap;



	static bool isOldMethod = false;
	static bool freezeDebug = false;
	int maxLevel = 1;

	zSTRING nodeWayGlobal;
	BVHNode* nodeFoundGlobal = NULL;



}