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

	
    class BVHNodePool
    {
    public:
        /// »нициализаци€ пула (выдел€ет первый блок)
        void Reserve(size_t size)
        {
            if (size == 0)
                throw std::invalid_argument("Block size must be > 0");

            //cmd << "Tree reserve: " << size << endl;

            firstBlockSize = size;
            otherBlockSize = size > 50 ? size / 10 : size;

            currentBlock = nextNode = 0;
            blocks.clear();
            AllocateBlock(true); // первый блок

        }

        uint32 GetTotalSize()
        {
            uint32 size = 0;

            for (int i = 0; i < blocks.size(); i++)
            {
                if (i == 0)
                {
                    size += sizeof(BVHNode) * firstBlockSize;
                }
                else
                {
                    size += sizeof(BVHNode) * otherBlockSize;
                }
                
            }

            return size;
        }

        /// ѕолучить новый узел
        BVHNode* GetNewNode()
        {
            size_t currentSize = (currentBlock == 0 ? firstBlockSize : otherBlockSize);

            if (nextNode >= currentSize) {
                AllocateBlock(false); // добавл€ем маленький блок
                currentSize = otherBlockSize;
            }

            return &blocks[currentBlock][nextNode++];
        }

    private:
        /// ¬ыделить новый блок
        void AllocateBlock(bool first)
        {
            size_t size = (first ? firstBlockSize : otherBlockSize);


#if defined (DEBUG_MEMORY_CHECK)
            AddMemoryInfo(sizeof(BVHNode) * size, "AllocateBlock (BVHNode)");
#endif

            /*
            if (first) {
                cmd << "NEW BLOCK CREATED: " << size << endl;
            }
            else {
                cmd << "ADD BLOCK: index: " << blocks.size()
                    << " size: " << size << endl;
            }
            */

            blocks.emplace_back(std::make_unique<BVHNode[]>(size));
            currentBlock = blocks.size() - 1;
            nextNode = 0;
        }

        std::vector<std::unique_ptr<BVHNode[]>> blocks; // список блоков
        size_t firstBlockSize = 0;   // размер первого блока
        size_t otherBlockSize = 0;   // размер остальных блоков
        size_t nextNode = 0;         // индекс следующего свободного узла
        size_t currentBlock = 0;     // индекс текущего блока
    };

	class BVH_Tree
	{
	public:
        BVHNodePool pool;
		zCProgMeshProto::zCSubMesh* subMesh;
		zCProgMeshProto* proto;
		BVHNode* root;

#if defined (DEBUG_BUILD_BVH)
		int nodesCount = 0;
#endif

		//====================================================================
		void SplitByBestAxis(BVHNode* node, std::vector<int>& triIndices, std::vector<int>& left, std::vector<int>& right, bool isDebug, std::vector<zVEC3>& centersTrias,
			std::vector<zTBBox3D>& bboxTrias);
		void AddAllTriangles(BVHNode* node, std::vector<int>& input, bool isDebug);
		void SplitByBinnedSAH(BVHNode* node, std::vector<int>& triIndices, std::vector<int>& left, std::vector<int>& right, bool isDebug, std::vector<zVEC3>& centersTrias,
			std::vector<zTBBox3D>& bboxTrias);
		BVHNode* BuildNode(BVHNode* parent, std::vector<int>& triIndices, int depth, bool isDebug, std::vector<zVEC3>& centersTrias,
			std::vector<zTBBox3D>& bboxTrias);


		zTBBox3D CalculateBBox(const std::vector<int>& indices);
		zVEC3 GetTriangleCenter(int triIdx);
		zTBBox3D GetTriangleBbox(int triIdx);
		void ScaleBboxes(BVHNode* node);
		void Build(zCProgMeshProto* proto, zCProgMeshProto::zCSubMesh* subMesh);
		void DestroyTree(BVHNode*& root);

		// DEBUG FUNCS
		int CheckAllIndices(const std::vector<int>& triIndices);

		BVH_Tree::BVH_Tree();

	};

	struct BVH_Debug
	{
		std::unordered_set<int> indexDebugCheck;
		int triasCheckerCount;
		std::atomic<int> globalNodesCount;
		std::atomic<int> globalNodesHasIndexes;
	} bvhDebug;
}