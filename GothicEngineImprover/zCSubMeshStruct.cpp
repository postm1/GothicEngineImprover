// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

	zCSubMeshStruct::zCSubMeshStruct()
	{
		parentProto = nullptr;
		subMesh = nullptr;
		bvhTree = nullptr;
	}

	void zCSubMeshStruct::Clear()
	{
		if (bvhTree)
		{
			bvhTree->DestroyTree(bvhTree->root);

			delete bvhTree;
			bvhTree = nullptr;
		}
	}
	void zCSubMeshStruct::BuildMap(zCProgMeshProto* proto, zCProgMeshProto::zCSubMesh* subMeshCurrent)
	{
		parentProto = proto;
		subMesh = subMeshCurrent;

		bvhTree = new BVH_Tree();
		bvhTree->Build(proto, subMesh);
	}

	void zCSubMeshStruct::Render(zCVob* parent, zCProgMeshProto* proto, zCProgMeshProto::zCSubMesh* subMesh)
	{
		//cmd << "Render" << endl;

		int nodesCount = 0;

		RenderNode(bvhTree->root, GFX_WHITE, parent, nodesCount, 0);

		/*
		PrintDebug("Nodes: " + Z nodesCount);
		PrintDebug("maxLevel: " + Z maxLevel);
		PrintDebug("Proto: " + Z proto->GetVisualName());
		PrintDebug("SubMesh: " + Z proto->numSubMeshes + " | Tris: " + Z subMesh->triList.GetNum());
		*/

	}

	void zCSubMeshStruct::RenderNode(BVHNode* node, zCOLOR color, zCVob* parent, int& nodesCount, int depth)
	{
		//cmd << "RenderNode: " << (int)node << endl;

		if (depth > maxLevel)
			return;

		nodesCount++;
		zTBBox3D worldBBox = node->bbox;

		if (parent)
		{
			//printWinC(worldBBox.mins.ToString() + "/" + worldBBox.maxs.ToString());
			worldBBox.Transform(parent->trafoObjToWorld, worldBBox);
		}

		//debug.AddBbox(worldBBox, color, 150 * 1000);
		//worldBBox.Draw(color);


		if (node->left)
		{
			//debug.AddLine(node->left->bbox.GetCenter(), node->left->bbox.GetCenter() + zVEC3(0, 40, 0), GFX_RED, 15);
			RenderNode(node->left, GFX_RED, parent, nodesCount, depth + 1);
		}

		if (node->right)
		{
			//debug.AddLine(node->right->bbox.GetCenter(), node->right->bbox.GetCenter() + zVEC3(0, 40, 0), GFX_GREEN, 15);
			RenderNode(node->right, GFX_GREEN, parent, nodesCount, depth + 1);
		}
	}

	constexpr size_t SIZEOF_zTPlane = sizeof(zTPlane);            // 0x10 (16  байт)
	constexpr size_t SIZEOF_zTPMWedge = sizeof(zTPMWedge);          // 0x18 (24  байта)
	constexpr size_t SIZEOF_zTPMTriangle = sizeof(zTPMTriangle);       // 0x06 (6   байт)
	constexpr size_t SIZEOF_zTPMTriangleEdges = sizeof(zTPMTriangleEdges);  // 0x06 (6   байт)
	constexpr size_t SIZEOF_zTPMEdge = sizeof(zTPMEdge);           // 0x04 (4   байта)
	constexpr size_t SIZEOF_zTPMVertexUpdate = sizeof(zTPMVertexUpdate);   // 0x04 (4   байта)

	inline std::size_t CalcSubMeshSizeBytes(const zCProgMeshProto::zCSubMesh& m)
	{
		std::size_t total = 0;

		// 1. Фиксированные поля самой структуры -------------------
		total += sizeof(m.material);      // указатель на материал (4 байта)
		total += sizeof(m.vbStartIndex);  // int (4 байта)

		// 2. Динамические массивы ---------------------------------
		total += m.triList.GetNum() * SIZEOF_zTPMTriangle;
		total += m.wedgeList.GetNum() * SIZEOF_zTPMWedge;
		total += m.colorList.GetNum() * sizeof(float);
		total += m.triPlaneIndexList.GetNum() * sizeof(unsigned short);
		total += m.triPlaneList.GetNum() * SIZEOF_zTPlane;
		total += m.triEdgeList.GetNum() * SIZEOF_zTPMTriangleEdges;
		total += m.edgeList.GetNum() * SIZEOF_zTPMEdge;
		total += m.edgeScoreList.GetNum() * sizeof(float);
		total += m.wedgeMap.GetNum() * sizeof(unsigned short);
		total += m.vertexUpdates.GetNum() * SIZEOF_zTPMVertexUpdate;

		return total;
	}

	inline double CalcSubMeshSizeKB(const zCProgMeshProto::zCSubMesh& m)
	{
		return static_cast<double>(CalcSubMeshSizeBytes(m)) / 1000.0f;
	}

	
	HOOK ivk_zCSubMesh_Destructor AS(&zCProgMeshProto::zCSubMesh::~zCSubMesh, &zCProgMeshProto::zCSubMesh::DestrUnion);
	void zCProgMeshProto::zCSubMesh::DestrUnion()
	{
		/*
		float size = CalcSubMeshSizeKB(*this);

		sizeUnloatDestr += size;

		cmd << "### zCSubMesh Remove: " << this->triList.GetNum() << " Size: " << size << " KB | Total: " << sizeUnloatDestr << endl;

		*/

		if (pTraceMap.find(this) != pTraceMap.end())
		{
			pTraceMap[this].Clear();
			pTraceMap.erase(this);
			//cmd << "Remove from pTraceMap too. Size: " << pTraceMap.size() << endl;
		}

		THISCALL(ivk_zCSubMesh_Destructor)();
	}


	std::mutex traceMapMutex;

	void ProcessSubMeshRange(
		const std::unordered_map<zCProgMeshProto::zCSubMesh*, zCProgMeshProto*>& bigSubmeshes,
		std::unordered_map<zCProgMeshProto::zCSubMesh*, zCSubMeshStruct>& localResult, // Локальный результат потока
		size_t startIndex,
		size_t endIndex
	) {
		auto it = std::next(bigSubmeshes.begin(), startIndex);
		auto endIt = std::next(bigSubmeshes.begin(), endIndex);

		for (; it != endIt; ++it) {
			auto* submesh = it->first;
			auto* meshProto = it->second;

			if (!submesh || !meshProto) continue;

			// Локальная проверка (без блокировок!)
			if (localResult.find(submesh) != localResult.end()) continue;

			zCSubMeshStruct collEntry;
			collEntry.BuildMap(meshProto, submesh);  // Самая тяжелая часть
			localResult[submesh] = std::move(collEntry);
		}
	}

	void ProcessAllSubMeshesParallel(
		const std::unordered_map<zCProgMeshProto::zCSubMesh*, zCProgMeshProto*>& bigSubmeshes,
		std::unordered_map<zCProgMeshProto::zCSubMesh*, zCSubMeshStruct>& pTraceMap
	) {
		const size_t totalItems = bigSubmeshes.size();

#if defined(DEBUG_BUILD_BVH)
		const unsigned int maxThreads = 1;
#else
		const unsigned int maxThreads = std::thread::hardware_concurrency();
#endif
		
		const size_t itemsPerThread = (totalItems + maxThreads - 1) / maxThreads;

		// 1. Каждый поток работает со своим локальным результатом
		std::vector<std::thread> threads;
		std::vector<std::unordered_map<zCProgMeshProto::zCSubMesh*, zCSubMeshStruct>> threadResults(maxThreads);

		for (unsigned int i = 0; i < maxThreads; ++i) {
			size_t start = i * itemsPerThread;
			size_t end = min(start + itemsPerThread, totalItems);
			if (start >= totalItems) break;

			threads.emplace_back(
				ProcessSubMeshRange,
				std::cref(bigSubmeshes),
				std::ref(threadResults[i]),  // Локальный контейнер для i-го потока
				start,
				end
			);
		}

		// 2. Дожидаемся завершения всех потоков
		for (auto& thread : threads) {
			thread.join();
		}

		// 3. Объединяем результаты (одна блокировка на весь merge)
		std::lock_guard<std::mutex> lock(traceMapMutex);
		for (auto& localMap : threadResults) {
			pTraceMap.insert(localMap.begin(), localMap.end());
		}
	}

	void RayCastVob_OnLevelLoaded()
	{
		//cmd << "RAM #1: " << RAMUsed() / 1000 << endl;

		RX_Begin(54);
		zCArray<zCVob*> arrVobs;

		
		cmd << "OnLoadedSize: " << pTraceMap.size() << endl;

		/*for (auto& pair : pTraceMap)
		{
			if (pair.second.parentProto)
			{
				cmd << pair.second.parentProto->GetVisualName() << endl;
			}
			
		}*/

		//Message::Box("SHOW");

		/*for (auto& pair : pTraceMap) 
		{
			pair.second.Clear();  
		}

		pTraceMap.clear();*/

		ogame->GetWorld()->SearchVobListByBaseClass(zCVob::classDef, arrVobs, NULL);

		//cmd << "Input: " << arrVobs.GetNumInList() << endl;

		double timeRequires = 0;

		std::unordered_map<zCProgMeshProto::zCSubMesh*, zCProgMeshProto*> submeshesFound;

		submeshesFound.reserve(1000);

		for (int i = 0; i < arrVobs.GetNumInList(); i++)
		{

			if (auto pVob = arrVobs.GetSafe(i))
			{
				if (pVob->collDetectionDynamic)
				{
					if (auto pVisual = pVob->GetVisual())
					{
						if (auto pProto = pVisual->CastTo<zCProgMeshProto>())
						{
							for (int s = 0; s < pProto->numSubMeshes; s++)
							{
								zCProgMeshProto::zCSubMesh* subMesh = &(pProto->subMeshList[s]);

								if (subMesh && subMesh->triList.GetNum() >= 0 && pTraceMap.find(subMesh) == pTraceMap.end())
								{
									if (submeshesFound.find(subMesh) == submeshesFound.end())
									{
										submeshesFound[subMesh] = pProto;
									}
								}
							}
						}
					}

				}
			}
		}


		pTraceMap.reserve(submeshesFound.size());
		

		//cmd << "ProcessAllSubMeshesParallel" << endl;

		ProcessAllSubMeshesParallel(submeshesFound, pTraceMap);

		RX_End(54);

		cmd << "Submeshes Found: " << submeshesFound.size() << endl;
		cmd << "RaycastVobs build time: " << RX_PerfString(54) << " Size: " << pTraceMap.size() << endl;

		

		

		cmd << "-----------------\n" << endl;
	}
}