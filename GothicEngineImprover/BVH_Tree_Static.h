// Supported with union (c) 2020 Union team
// Union HEADER file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

	struct BVHNodeStatic
	{
		zTBBox3D bbox;
		std::vector<zCPolygon> nodePolys;
		BVHNodeStatic* left = nullptr;
		BVHNodeStatic* right = nullptr;
		BVHNodeStatic* parent = nullptr;
	};

	class BVH_TreeStatic
	{
	public:

		BVHNodeStatic* root;

		std::vector<zVEC3> centersTrias;
		std::vector<zTBBox3D> bboxTrias;
		int nodesCount = 0;

		void SplitByBestAxis(BVHNodeStatic* node, std::vector<zCPolygon*>& triIndices, std::vector<zCPolygon*>& left, std::vector<zCPolygon*>& right, bool isDebug);
		void AddAllTriangles(BVHNodeStatic* node, std::vector<zCPolygon*>& input, bool isDebug);
		void SplitByBinnedSAH(BVHNodeStatic* node, std::vector<zCPolygon*>& triIndices, std::vector<zCPolygon*>& left, std::vector<zCPolygon*>& right, bool isDebug);

		zTBBox3D CalculateBBox(const std::vector<zCPolygon*>& indices);
		zVEC3 GetTriangleCenter(zCPolygon* poly);
		void ScaleBboxes(BVHNodeStatic* node);


		void Build();
		BVHNodeStatic* BuildNode(BVHNodeStatic* parent, std::vector<zCPolygon*>& triIndices, int depth = 0, bool isDebug = false);
		void DestroyTree(BVHNodeStatic*& root);


		BVH_TreeStatic::BVH_TreeStatic();

	};
}