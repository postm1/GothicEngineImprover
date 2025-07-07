// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

	BVH_Tree::BVH_Tree()
	{
		subMesh = nullptr;
		proto = nullptr;
		root = nullptr;
	}

	void BVH_Tree::SplitByBestAxis(BVHNode* node, std::vector<int>& triIndices, std::vector<int>& left, std::vector<int>& right)
	{
		int data[3][2] = { 0 };
		zTBBox3D dataVolume[3][2];
		float middlePoints[3] = { 0 };

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				dataVolume[i][j].Init();
			}
		}

		for (int axis = 0; axis < 3; axis++)
		{
			middlePoints[axis] = node->bbox.GetMiddleOfAxis(axis);

			for (size_t i = 0; i < triIndices.size(); ++i)
			{
				auto index = triIndices[i];
				auto center = centersTrias[index];
				auto polyBbox = bboxTrias[index];


				if (center.n[axis] < middlePoints[axis])
				{
					data[axis][0]++;
					dataVolume[axis][0].MergeBox(polyBbox);
				}
				else
				{
					data[axis][1]++;
					dataVolume[axis][1].MergeBox(polyBbox);
				}

			}
		}

		int bestAxis = 0;
		int bestPair = abs(data[0][0] - data[0][1]);


		for (int i = 1; i < 3; i++)
		{
			int diff = abs(data[i][0] - data[i][1]);

			if (diff < bestPair)
			{
				bestAxis = i;
				bestPair = diff;
			}
		}


		float middlePoint = middlePoints[bestAxis];

		for (size_t i = 0; i < triIndices.size(); ++i)
		{
			auto index = triIndices[i];
			auto center = centersTrias[index];

			if (center.n[bestAxis] < middlePoint)
			{
				left.push_back(index);
			}
			else
			{
				right.push_back(index);
			}
		}

		// Если разделение не удалось (все элементы в одной группе)
		if (left.empty() || right.empty())
		{
			left.clear();
			right.clear();

			
			// Альтернативный метод: сортируем и делим пополам
			std::sort(triIndices.begin(), triIndices.end(), [&](int a, int b) {
				return centersTrias[a].n[bestAxis] < centersTrias[b].n[bestAxis];
				});

			size_t half = triIndices.size() / 2;
			left.insert(left.end(), triIndices.begin(), triIndices.begin() + half);
			right.insert(right.end(), triIndices.begin() + half, triIndices.end());

			cmd << "SPLIT CENTER: " << triIndices.size() << " -> " << left.size() << " | " << right.size() << endl;
		}
	}

	

	void BVH_Tree::AddAllTriangles(BVHNode* node, std::vector<int>& input, bool isDebug)
	{
		int size = input.size();

		if (isDebug)
		{
			for (int i = 0; i < size; ++i)
			{

				if (bvhDebug.indexDebugCheck.find(input[i]) == bvhDebug.indexDebugCheck.end())
				{
					node->triIndices.push_back(input[i]);
					bvhDebug.indexDebugCheck.insert(input[i]);


				}
				else
				{
					cmd << "Index already exists! -> " << input[i] << endl;
					Message::Box("Index already exists: " + input[i]);
				}
			}

			//cmd << "Can't split Node, add: " << size << endl;
		}
		else
		{
			node->triIndices.assign(input.begin(), input.begin() + size);
		}

		if (node->triIndices.size() >= 50 && !showModel)
		{
			showModel = true;

			DrawObjectBVH(this, node, 1150*1000);
			
		}

		bvhDebug.triasCheckerCount += size;
	}

	

	BVHNode* BVH_Tree::BuildNode(BVHNode* parent, std::vector<int>& triIndices, int depth, bool isDebug)
	{
		if (triIndices.size() == 0)
		{
			return NULL;
		}

		//cmd << "triIndices: " << triIndices.size() << endl;

		BVHNode* node = new BVHNode();

		nodesCount++;

		node->parent = parent;

		// Вычисляем общий bounding box для всех треугольников в данном списке
		node->bbox = CalculateBBox(triIndices);

		if (triIndices.size() <= 2)
		{
			AddAllTriangles(node, triIndices, isDebug);
			return node;
		}


		std::vector<int> leftIndices;
		std::vector<int> rightIndices;

		leftIndices.reserve(triIndices.size() / 2);
		rightIndices.reserve(triIndices.size() / 2);

		SplitByBestAxis(node, triIndices, leftIndices, rightIndices);

		//if (isDebug) cmd << triIndices.size() << " -> " << leftIndices.size() << " | " << rightIndices.size() << endl;

		if (triIndices.size() == rightIndices.size() || triIndices.size() == leftIndices.size())
		{
			AddAllTriangles(node, triIndices, isDebug);
			return node;
		}

		node->left = BuildNode(node, leftIndices, 0, isDebug);
		node->right = BuildNode(node, rightIndices, 0, isDebug);

		return node;
	}

	void BVH_Tree::DestroyTree(BVHNode*& root)
	{
		if (!root) return;

		std::stack<BVHNode*> nodes;

		nodes.push(root);
		root = nullptr;

		while (!nodes.empty())
		{
			BVHNode* node = nodes.top();
			nodes.pop();

			node->parent = NULL;

			// Добавляем детей в стек
			if (node->left) nodes.push(node->left);
			if (node->right) nodes.push(node->right);

			delete node;
		}
	}

	zTBBox3D BVH_Tree::CalculateBBox(const std::vector<int>& indices)
	{
		zTBBox3D bbox;

		bbox.Init();

		// Для каждого индекса из списка
		for (size_t i = 0; i < indices.size(); i++)
		{
			int triIdx = indices[i];
			// Получаем треугольник по индексу
			const auto& tri = subMesh->triList[triIdx];

			// Добавляем все 3 вершины треугольника в bounding box
			bbox.AddPoint(proto->posList[subMesh->wedgeList[tri.wedge[0]].position]);
			bbox.AddPoint(proto->posList[subMesh->wedgeList[tri.wedge[1]].position]);
			bbox.AddPoint(proto->posList[subMesh->wedgeList[tri.wedge[2]].position]);
		}
		return bbox;
	}

	zVEC3 BVH_Tree::GetTriangleCenter(int triIdx)
	{
		const auto& tri = subMesh->triList[triIdx];
		zVEC3 center;
		center += proto->posList[subMesh->wedgeList[tri.wedge[0]].position];
		center += proto->posList[subMesh->wedgeList[tri.wedge[1]].position];
		center += proto->posList[subMesh->wedgeList[tri.wedge[2]].position];
		return center / 3.0f;
	}

	zTBBox3D BVH_Tree::GetTriangleBbox(int triIdx)
	{
		const auto& tri = subMesh->triList[triIdx];

		// Инициализация bounding box с первой вершины треугольника
		zTBBox3D triangleBbox;

		triangleBbox.Init();

		const auto& vertex0 = proto->posList[subMesh->wedgeList[tri.wedge[0]].position];
		triangleBbox.AddPoint(vertex0);

		// Добавляем оставшиеся вершины треугольника
		const auto& vertex1 = proto->posList[subMesh->wedgeList[tri.wedge[1]].position];
		triangleBbox.AddPoint(vertex1);

		const auto& vertex2 = proto->posList[subMesh->wedgeList[tri.wedge[2]].position];
		triangleBbox.AddPoint(vertex2);

		return triangleBbox;
	}

	void BVH_Tree::ScaleBboxes(BVHNode* node)
	{
		if (node)
		{
			node->bbox.Scale(1.01f);

			if (node->left)
			{
				ScaleBboxes(node->left);
			}

			if (node->right)
			{
				ScaleBboxes(node->right);
			}
		}
	}
	void BVH_Tree::Build(zCProgMeshProto* proto, zCProgMeshProto::zCSubMesh* subMesh)
	{
		this->proto = proto;
		this->subMesh = subMesh;


		bool isDebugBuild = false;
		bool showBuildMessage = false;

		//RX_Begin(53);

		bvhDebug.triasCheckerCount = 0;

		if (isDebugBuild)
		{
			bvhDebug.indexDebugCheck.clear();
		}


		std::vector<int> triIndices; // Индексы треугольников

		triIndices.resize(subMesh->triList.GetNum());

		// заполняем массив от 0 до n-1
		std::iota(triIndices.begin(), triIndices.end(), 0);



		centersTrias.reserve(subMesh->triList.GetNum());
		bboxTrias.reserve(subMesh->triList.GetNum());

		int size = triIndices.size();

		for (int i = 0; i < size; ++i)
		{
			centersTrias[i] = GetTriangleCenter(i);
			bboxTrias[i] = GetTriangleBbox(i);
		}


		//cmd << "BuildStart: " << proto->GetVisualName() << endl;



		root = BuildNode(NULL, triIndices, 0, isDebugBuild);

		ScaleBboxes(root);

		//RX_End(53);

		bboxTrias.clear();
		centersTrias.clear();

		if (showBuildMessage)
		{
			cmd << "Build: "
				<< proto->GetVisualName()
				<< " Name: " << proto->GetObjectName()
				<< " | Tris: " << bvhDebug.triasCheckerCount << "/" << subMesh->triList.GetNum()
				<< " | Nodes: " << nodesCount
				<< " | " << RX_PerfString(53) << endl;
		}


		if (isDebugBuild)
		{


			if (bvhDebug.triasCheckerCount != subMesh->triList.GetNum() || bvhDebug.indexDebugCheck.size() != bvhDebug.triasCheckerCount)
			{
				cmd << "-------- BUILD WRONG: " << proto->GetVisualName() << " " << bvhDebug.triasCheckerCount
					<< "/" << subMesh->triList.GetNum()
					<< " / IndexDebugSize: " << bvhDebug.indexDebugCheck.size()
					<< endl;
				Message::Box("Wrong: Size");
			}

			int bad = CheckAllIndices(triIndices);

			if (bad > 0)
			{
				Message::Box("BAD TREE");
				cmd << "Tree Is BAD: NOT ALL!!! -> " << bad << endl;
			}
		}

	}

	int BVH_Tree::CheckAllIndices(const std::vector<int>& triIndices)
	{
		int badCount = 0;

		for (int i : triIndices)
		{
			if (bvhDebug.indexDebugCheck.find(i) == bvhDebug.indexDebugCheck.end())
			{
				badCount++;
			}
		}
		return badCount;
	}
}