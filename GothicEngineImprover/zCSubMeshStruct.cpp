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
			bvhTree->Clear(bvhTree->root);
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


	HOOK ivk_zCSubMesh_Destructor AS(&zCProgMeshProto::zCSubMesh::~zCSubMesh, &zCProgMeshProto::zCSubMesh::DestrUnion);
	void zCProgMeshProto::zCSubMesh::DestrUnion()
	{
		//cmd << "### zCSubMesh Remove: " << this->triList.GetNum() << endl;

		if (pTraceMap.find(this) != pTraceMap.end())
		{


			pTraceMap[this].Clear();
			pTraceMap.erase(this);

			//cmd << "Remove from pTraceMap too. Size: " << pTraceMap.size() << endl;
		}

		//THISCALL(ivk_zCSubMesh_Destructor)();
	}

	void RayCastVob_OnLevelLoaded()
	{
		RX_Begin(54);
		zCArray<zCVob*> arrVobs;

		pTraceMap.clear();
		pTraceMap.reserve(3000);

		ogame->GetWorld()->SearchVobListByBaseClass(zCVob::classDef, arrVobs, NULL);

		//cmd << "Input: " << arrVobs.GetNumInList() << endl;

		double timeRequires = 0;
		bool flagStop = false;

		std::unordered_map<zCProgMeshProto::zCSubMesh*, zCProgMeshProto*> bigSubmeshes;

		bigSubmeshes.reserve(1000);

		// Ищем сначала все визуалы с большим кол-вом поликов
		for (int i = 0; i < arrVobs.GetNumInList(); i++)
		{
			if (flagStop) break;

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

								if (subMesh && subMesh->triList.GetNum() >= 1500)
								{
									if (bigSubmeshes.find(subMesh) == bigSubmeshes.end())
									{
										bigSubmeshes[subMesh] = pProto;
									}
								}
							}
						}
					}

				}
			}
		}

		// 1. Переносим элементы в вектор для сортировки
		std::vector<std::pair<zCProgMeshProto::zCSubMesh*, zCProgMeshProto*>> sortedSubmeshes(
			bigSubmeshes.begin(),
			bigSubmeshes.end()
		);

		// 2. Сортируем по количеству полигонов (от большего к меньшему)
		std::sort(
			sortedSubmeshes.begin(),
			sortedSubmeshes.end(),
			[](const auto& a, const auto& b) {
				return a.first->triList.GetNum() > b.first->triList.GetNum();
			}
		);

		// 3. Теперь итерируемся по отсортированному списку
		for (const auto& it : sortedSubmeshes)
		{
			auto* submesh = it.first;
			auto* meshProto = it.second;

			if (submesh && meshProto)
			{
				if (pTraceMap.find(submesh) == pTraceMap.end())
				{

					/*if (submesh->triList.GetNum() >= 1500)
					{
						cmd << submesh->triList.GetNum() << " | " << meshProto->GetVisualName() << endl;
					}
	*/

					RX_Begin(55);

					zCSubMeshStruct collEntry;

					collEntry.BuildMap(meshProto, submesh);

					pTraceMap[submesh] = collEntry;

					RX_End(55);

					timeRequires += perf[55];


					// if it require > 100 ms to load => load it later
					if ((timeRequires / 1000.0f) >= 10)
					{
						/*
						cmd << "STOP LONG TIME" << endl;
						flagStop = true;
						break;
						*/
					}
				}
			}
		}



		/*

		RX_Begin(55);


									RX_End(55);

									timeRequires += perf[55];

									// if it require > 100 ms to load => load it later
									if ((timeRequires / 1000.0f) >= 10)
									{
										cmd << "STOP LONG TIME" << endl;
										flagStop = true;
										break;
									}
		*/


		RX_End(54);

		cmd << "RaycastVobs build time: " << RX_PerfString(54) << " Size: " << pTraceMap.size() << endl;
	}
}