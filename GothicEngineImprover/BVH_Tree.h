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
		BVHNode* parent = nullptr;
	};

	struct BVH_Debug
	{
		std::unordered_set<int> indexDebugCheck;
		int triasCheckerCount;

	} bvhDebug;

	class BVH_Tree
	{
	public:

		zCProgMeshProto::zCSubMesh* subMesh;
		zCProgMeshProto* proto;
		BVHNode* root;

		std::vector<zVEC3> centersTrias;
		std::vector<zTBBox3D> bboxTrias;
		int nodesCount = 0;

		void SplitByBestAxis(BVHNode* node, std::vector<int>& triIndices, std::vector<int>& left, std::vector<int>& right);
		void AddAllTriangles(BVHNode* node, std::vector<int>& input, bool isDebug);


		zTBBox3D CalculateBBox(const std::vector<int>& indices);
		zVEC3 GetTriangleCenter(int triIdx);
		zTBBox3D GetTriangleBbox(int triIdx);
		void ScaleBboxes(BVHNode* node);


		void Build(zCProgMeshProto* proto, zCProgMeshProto::zCSubMesh* subMesh);
		BVHNode* BuildNode(BVHNode* parent, std::vector<int>& triIndices, int depth = 0, bool isDebug = false);
		void Clear(BVHNode*& root);

		// DEBUG FUNCS

		int CheckAllIndices(const std::vector<int>& triIndices);


		BVH_Tree::BVH_Tree();

	};
}