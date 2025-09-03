// Supported with union (c) 2020 Union team
// Union HEADER file

namespace GOTHIC_ENGINE {
	// Add your code here . . .
	struct BVHNode
	{
		zTBBox3D bbox;
		std::vector<int> triIndices;
		BVHNode* left = nullptr;
		BVHNode* right = nullptr;
#if defined (BVH_PARENT_POINTER)
		BVHNode* parent = nullptr;
#endif
	};

	struct BVH_Debug
	{
		std::unordered_set<int> indexDebugCheck;
		int triasCheckerCount;
		std::atomic<int> globalNodesCount;
		std::atomic<int> globalNodesHasIndexes;
	} bvhDebug;

	class BVH_Tree
	{
	public:

		zCProgMeshProto::zCSubMesh* subMesh;
		zCProgMeshProto* proto;
		BVHNode* root;

#if defined (DEBUG_BUILD_BVH)
		int nodesCount = 0;
#endif


		void SplitByBestAxis(BVHNode* node, std::vector<int>& triIndices, std::vector<int>& left, std::vector<int>& right, bool isDebug, std::vector<zVEC3>& centersTrias,
			std::vector<zTBBox3D>& bboxTrias);
		void AddAllTriangles(BVHNode* node, std::vector<int>& input, bool isDebug);
		void SplitByBinnedSAH(BVHNode* node, std::vector<int>& triIndices, std::vector<int>& left, std::vector<int>& right, bool isDebug, std::vector<zVEC3>& centersTrias,
			std::vector<zTBBox3D>& bboxTrias);

		zTBBox3D CalculateBBox(const std::vector<int>& indices);
		zVEC3 GetTriangleCenter(int triIdx);
		zTBBox3D GetTriangleBbox(int triIdx);
		void ScaleBboxes(BVHNode* node);


		void Build(zCProgMeshProto* proto, zCProgMeshProto::zCSubMesh* subMesh);
		BVHNode* BuildNode(BVHNode* parent, std::vector<int>& triIndices, int depth, bool isDebug, std::vector<zVEC3>& centersTrias,
			std::vector<zTBBox3D>& bboxTrias);
		void DestroyTree(BVHNode*& root);

		// DEBUG FUNCS

		int CheckAllIndices(const std::vector<int>& triIndices);


		BVH_Tree::BVH_Tree();

	};
}