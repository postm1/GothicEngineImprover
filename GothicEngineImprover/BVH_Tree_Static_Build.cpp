// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

	void BVH_TreeStatic::SplitByBestAxis(BVHNodeStatic* node, std::vector<zCPolygon*>& triIndices, std::vector<zCPolygon*>& left, std::vector<zCPolygon*>& right, bool isDebug)
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
				auto center = triIndices[i]->GetCenter();
				auto polyBbox = triIndices[i]->GetBBox3D();


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
			auto center = triIndices[i]->GetCenter();

			if (center.n[bestAxis] < middlePoint)
			{
				left.push_back(triIndices[i]);
			}
			else
			{
				right.push_back(triIndices[i]);
			}
		}

		// If we can't split by middle of longest axis => another method
		if ((left.empty() || right.empty()) && triIndices.size() >= 5)
		{
			left.clear();
			right.clear();


			// sort and split by halves
			std::sort(triIndices.begin(), triIndices.end(), [&](zCPolygon* a, zCPolygon* b) {
				return a->GetCenter().n[bestAxis] < b->GetCenter().n[bestAxis];
				});

			size_t half = triIndices.size() / 2;
			left.insert(left.end(), triIndices.begin(), triIndices.begin() + half);
			right.insert(right.end(), triIndices.begin() + half, triIndices.end());

			if (isDebug) cmd << "SPLIT CENTER: " << triIndices.size() << " -> " << left.size() << " | " << right.size() << endl;
		}
	}

	void BVH_TreeStatic::SplitByBinnedSAH(BVHNodeStatic* node, std::vector<zCPolygon*>& triIndices, std::vector<zCPolygon*>& left, std::vector<zCPolygon*>& right, bool isDebug)
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

			for (auto& idx : triIndices)
			{
				float val = idx->GetCenter().n[axis];
				//float val = centersTrias[idx].n[axis];
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
			for (auto& idx : triIndices)
			{
				float val = idx->GetCenter().n[axis];
				//float val = centersTrias[idx].n[axis];
				int binIdx = min(BINS - 1, (int)((val - minVal) * scale));
				bins[binIdx].bbox.MergeBox(idx->GetBBox3D());
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
		for (auto& idx : triIndices)
		{
			float val = idx->GetCenter().n[bestAxis];
			//float val = centersTrias[idx].n[bestAxis];
			minVal = min(minVal, val);
			maxVal = max(maxVal, val);
		}
		float splitPos = minVal + (maxVal - minVal) * (bestSplitBin / float(BINS));


		//cmd << "SAH split by axis: " << bestAxis << endl;

		for (auto& idx : triIndices)
		{
			if (idx->GetCenter().n[bestAxis] < splitPos)
			{
				left.push_back(idx);
			}
			else
			{
				right.push_back(idx);
			}
			/*if (centersTrias[idx].n[bestAxis] < splitPos)
				left.push_back(idx);
			else
				right.push_back(idx);*/
		}
	}



	void BVH_TreeStatic::AddAllTriangles(BVHNodeStatic* node, std::vector<zCPolygon*>& input, bool isDebug)
	{
		int size = input.size();

		if (isDebug)
		{
			/*
			for (int i = 0; i < size; ++i)
			{

				if (bvhDebug.indexDebugCheck.find(input[i]) == bvhDebug.indexDebugCheck.end())
				{
					node->nodePolys.push_back(*input[i]);
					bvhDebug.indexDebugCheck.insert(input[i]);


				}
				else
				{
					cmd << "Index already exists! -> " << input[i] << endl;
					Message::Box("Index already exists: " + input[i]);
				}
			}
			*/
			//cmd << "Can't split Node, add: " << size << endl;
		}
		else
		{
			node->nodePolys.assign(input.begin(), input.begin() + size);
		}

		bvhDebug.triasCheckerCount += size;
	}



	BVHNodeStatic* BVH_TreeStatic::BuildNode(BVHNodeStatic* parent, std::vector<zCPolygon*>& triIndices, int depth, bool isDebug)
	{
		if (triIndices.size() == 0)
		{
			return NULL;
		}

		//cmd << "triIndices: " << triIndices.size() << endl;

		BVHNodeStatic* node = new BVHNodeStatic();

		nodesCount++;

		node->parent = parent;

		// Вычисляем общий bounding box для всех треугольников в данном списке
		node->bbox = CalculateBBox(triIndices);

		if (triIndices.size() <= 2)
		{
			AddAllTriangles(node, triIndices, isDebug);
			return node;
		}


		std::vector<zCPolygon*> leftIndices;
		std::vector<zCPolygon*> rightIndices;

		leftIndices.reserve(triIndices.size() / 2);
		rightIndices.reserve(triIndices.size() / 2);

		// если уже загрузились, строим дерево быстрым способом, иначе будут подфризы	
		SplitByBinnedSAH(node, triIndices, leftIndices, rightIndices, isDebug);


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


	void BVH_TreeStatic::Build()
	{

		bool isDebugBuild = false;
		bool showBuildMessage = false;

#if defined (DEBUG_BUILD_BVH)

		if (showBuildMessage) cmd << "\n========= BUILD: " << proto->GetVisualName() << endl;


		
#endif // DEBUG


		RX_Begin(53);

		bvhDebug.triasCheckerCount = 0;

		if (isDebugBuild)
		{
			bvhDebug.indexDebugCheck.clear();
		}


		//std::vector<int> triIndices; // Индексы треугольников

		//triIndices.resize(subMesh->triList.GetNum());

		// заполняем массив от 0 до n-1
		//std::iota(triIndices.begin(), triIndices.end(), 0);



		//centersTrias.reserve(subMesh->triList.GetNum());
		//bboxTrias.reserve(subMesh->triList.GetNum());

		//cmd << "BuildStart: " << proto->GetVisualName() << endl;


		zCPolygon**& trisList = ogame->GetWorld()->bspTree.treePolyList;
		int numPolys = ogame->GetWorld()->bspTree.numPolys;

		std::unordered_set<zCPolygon*> hashSet;

		std::vector<zCPolygon*> inputPolys;

		inputPolys.reserve(numPolys);

		for (int i = 0; i < numPolys; i++)
		{
			zCPolygon* poly = trisList[i];

			if (!poly) continue;

			if (poly->flags.portalPoly != 0) continue;
			if (poly->flags.ghostOccluder != 0) continue;

			auto mat = poly->material;

			if (mat && mat->matGroup == zTMat_Group::zMAT_GROUP_WATER)
			{
				continue;
			}

			if (mat && mat->noCollDet)
			{
				continue;
			}

			if (hashSet.find(poly) == hashSet.end())
			{
				hashSet.insert(poly);
				inputPolys.push_back(poly);
			}

		}

		cmd << "Static::Input polys: " << inputPolys.size() << endl;

		root = BuildNode(NULL, inputPolys, 0, isDebugBuild);

		//ScaleBboxes(root);

#if defined (DEBUG_BUILD_BVH)
		
#endif

		RX_End(53);

		cmd << "BuildTime: " << RX_PerfString(53) << endl;

		

		if (isDebugBuild)
		{

			/*
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
			*/
		}

	}


	zBOOL BVH_TreeStatic::RayCast(const zVEC3& rayOrigin, const zVEC3& rayDir, zVEC3& foundInter)
	{
		BVHNodeStatic* stack[100];  // Статический стек с запасом
		int stackPtr = 0;    // Указатель на вершину стека

		stack[stackPtr++] = this->root;

		bestAlphaGlobal = 9000.0f;

		zBOOL hitFound = FALSE;
		zVEC3 inters;
		zREAL alpha = 9999;
		zREAL tmin, tmax;

		while (stackPtr > 0)
		{
			BVHNodeStatic* node = stack[--stackPtr];

#if defined(DEBUG_BUILD_BVH)
			globalStackDepth = max(globalStackDepth, stackPtr);
			raycastReport.NodeTreeCheckCounter++;
			tmin = tmax = 1.0f;
#endif


			// Проверка пересечения луча с AABB узла (с учетом bestAlpha для early-out)
			if (!(node->bbox.IsIntersecting(rayOrigin, rayDir, tmin, tmax) && tmax >= 0.0f && tmin <= 1.0f))
			{
				continue;
			}

			if (tmin > raycastReport.bestAlphaGlobal)
			{
				continue;
			}

			// Проверка пересечения с треугольниками в листе
			for (zCPolygon* poly : node->nodePolys)
			{
#if defined(DEBUG_BUILD_BVH)
				raycastReport.TrisTreeCheckCounter++;
#endif


				//if (proto->CheckRayPolyIntersection(subMesh, triIdx, rayOrigin, rayDir, inters, alpha))
				if (poly->CheckRayPolyIntersection(rayOrigin, rayDir, inters, alpha))
				{
					hitFound = true;

					if (alpha < raycastReport.bestAlphaGlobal)
					{
						//cmd << alpha << " < " << bestAlphaGlobal << endl;

						bestAlphaGlobal = alpha;
						foundInter = inters;
					}

					/*
					// FIRSTHIT
					if (raycastReport.globalFirstHitMode)
					{
						return true;
					}
					*/
				}
			}


			// Добавляем дочерние узлы в стек (правый -> левый для DFS)
			if (node->right) stack[stackPtr++] = node->right;
			if (node->left)  stack[stackPtr++] = node->left;

		}

		return hitFound;
	}
}