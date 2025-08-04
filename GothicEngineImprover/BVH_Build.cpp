// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

	void BVH_Tree::SplitByBestAxis(BVHNode* node, std::vector<int>& triIndices, std::vector<int>& left, std::vector<int>& right, bool isDebug)
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

		// If we can't split by middle of longest axis => another method
		if ((left.empty() || right.empty()) && triIndices.size() >= 5)
		{
			left.clear();
			right.clear();

			
			// sort and split by halves
			std::sort(triIndices.begin(), triIndices.end(), [&](int a, int b) {
				return centersTrias[a].n[bestAxis] < centersTrias[b].n[bestAxis];
				});

			size_t half = triIndices.size() / 2;
			left.insert(left.end(), triIndices.begin(), triIndices.begin() + half);
			right.insert(right.end(), triIndices.begin() + half, triIndices.end());

			if (isDebug) cmd << "SPLIT CENTER: " << triIndices.size() << " -> " << left.size() << " | " << right.size() << endl;
		}
	}

	void BVH_Tree::SplitByBinnedSAH(BVHNode* node, std::vector<int>& triIndices, std::vector<int>& left, std::vector<int>& right, bool isDebug)
	{
		int BINS = (triIndices.size() < 100) ? 6 : 12; // Пример настройки

		float bestCost = FLT_MAX;
		int bestAxis = -1;
		int bestSplitBin = -1;

		// Перебираем все оси (X, Y, Z)
		for (int axis = 0; axis < 3; axis++)
		{
			// Находим min/max центров вдоль оси
			float minVal = FLT_MAX, maxVal = -FLT_MAX;
			for (int idx : triIndices)
			{
				float val = centersTrias[idx].n[axis];
				minVal = min(minVal, val);
				maxVal = max(maxVal, val);
			}

			// Если все треугольники на одной позиции — пропускаем ось
			if (minVal == maxVal) continue;

			// Инициализируем корзины
			struct Bin { zTBBox3D bbox; int count = 0; };
			std::vector<Bin> bins(BINS);

			// Заполняем корзины
			float scale = BINS / (maxVal - minVal);
			for (int idx : triIndices)
			{
				float val = centersTrias[idx].n[axis];
				int binIdx = min(BINS - 1, (int)((val - minVal) * scale));
				bins[binIdx].bbox.MergeBox(bboxTrias[idx]);
				bins[binIdx].count++;
			}

			// Перебираем все возможные разбиения между корзинами
			for (int split = 1; split < BINS; split++)
			{
				// Вычисляем AABB и количество слева от split
				zTBBox3D leftBox;
				int leftCount = 0;
				for (int i = 0; i < split; i++)
				{
					leftBox.MergeBox(bins[i].bbox);
					leftCount += bins[i].count;
				}

				// Вычисляем AABB и количество справа от split
				zTBBox3D rightBox;
				int rightCount = 0;
				for (int i = split; i < BINS; i++)
				{
					rightBox.MergeBox(bins[i].bbox);
					rightCount += bins[i].count;
				}

				// Считаем стоимость SAH
				float cost = 0.3f + (leftBox.Area() * leftCount + rightBox.Area() * rightCount) / node->bbox.Area();

				// Если нашли лучшее разбиение — запоминаем
				if (cost < bestCost)
				{
					bestCost = cost;
					bestAxis = axis;
					bestSplitBin = split;
				}
			}
		}

		// Если не нашли разбиения — делим пополам
		if (bestAxis == -1)
		{
			//cmd << "Can't find -> Split by Best axis" << endl;
			SplitByBestAxis(node, triIndices, left, right, isDebug);
			return;
		}

		// Разделяем треугольники по лучшему разбиению
		float minVal = FLT_MAX, maxVal = -FLT_MAX;
		for (int idx : triIndices)
		{
			float val = centersTrias[idx].n[bestAxis];
			minVal = min(minVal, val);
			maxVal = max(maxVal, val);
		}
		float splitPos = minVal + (maxVal - minVal) * (bestSplitBin / float(BINS));


		//cmd << "SAH split by axis: " << bestAxis << endl;

		for (int idx : triIndices)
		{
			if (centersTrias[idx].n[bestAxis] < splitPos)
				left.push_back(idx);
			else
				right.push_back(idx);
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

#if defined (DEBUG_MEMORY_CHECK)
		AddMemoryInfo(sizeof(BVHNode), "BuildNode (BVHNode)");
#endif

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

		// если уже загрузились, строим дерево быстрым способом, иначе будут подфризы	
		if (OnLevelFullLoaded_Once)
		{
			SplitByBestAxis(node, triIndices, leftIndices, rightIndices, isDebug);
		}
		else
		{
			SplitByBinnedSAH(node, triIndices, leftIndices, rightIndices, isDebug);
		}
		

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

	
	void BVH_Tree::Build(zCProgMeshProto* proto, zCProgMeshProto::zCSubMesh* subMesh)
	{
		this->proto = proto;
		this->subMesh = subMesh;

		

		bool isDebugBuild = false;
		bool showBuildMessage = false;

#if defined (DEBUG_BUILD_BVH)

		if (showBuildMessage) cmd << "\n========= BUILD: " << proto->GetVisualName() << endl;
		

		RX_Begin(53);
#endif // DEBUG


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

#if defined (DEBUG_BUILD_BVH)
		RX_End(53);
#endif

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

	
}