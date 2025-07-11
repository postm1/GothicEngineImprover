// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

	void GetNodeWay(BVHNode* node)
	{
		if (node)
		{
			nodeWayGlobal += Z(int)node + " <- ";
		}

		if (node->parent)
		{
			GetNodeWay(node->parent);
		}
	}
	int FindNodeByIndexRec(BVHNode* node, int indexToFind, int& result)
	{
		if (!node)
		{
			return 0;
		}

		auto it = std::find(node->triIndices.begin(), node->triIndices.end(), indexToFind);


		if (it != node->triIndices.end())
		{
			result = (int)node;
			nodeFoundGlobal = node;
			nodeWayGlobal = "";
			GetNodeWay(node);
			return 0;
		}

		if (node->left)
		{
			FindNodeByIndexRec(node->left, indexToFind, result);
		}

		if (node->right)
		{
			FindNodeByIndexRec(node->right, indexToFind, result);
		}
	}
	int FindNodeByIndex(zCSubMeshStruct* meshEntry, int indexToFind)
	{
		auto node = meshEntry->bvhTree->root;
		int result = -1;

		FindNodeByIndexRec(node, indexToFind, result);

		return result;
	}


	int FindNodeByNodeRec(BVHNode* node, BVHNode* nodeToFind)
	{
		if (node == nodeToFind)
		{
			nodeFoundGlobal = node;
			return 0;
		}
		if (node->left)
		{
			FindNodeByNodeRec(node->left, nodeToFind);
		}

		if (node->right)
		{
			FindNodeByNodeRec(node->right, nodeToFind);
		}
	}

	void FindNodeByNode(zCSubMeshStruct* meshEntry, BVHNode* nodeToFind)
	{
		auto node = meshEntry->bvhTree->root;

		nodeFoundGlobal = NULL;

		FindNodeByNodeRec(node, nodeToFind);
	}





	void ShowVisualCollisionReport(zCProgMeshProto* proto, const zVEC3& rayOrigin, const zVEC3& ray, int time, zTTraceRayReport& report, int bestTriIndex, zTPlane* bestPlane)
	{

		
		debug.AddRay(rayOrigin, rayOrigin + ray, zCOLOR(253, 134, 50), time); // сам тестовый луч
		debug.AddSphere(report.foundIntersection, 2.0, GFX_RED, time, false); // пересечение

		if (raycastReport.globalSubmeshBestOld)
		{


			//debug.AddTriangle(pos0, pos1, pos2, GFX_WHITE, time);
		}

		debug.AddAxis(zVEC3(0, 0, 0), 200, time);


		if (bestPlane)
		{
			debug.AddPlane(bestPlane, 200, zCOLOR(55, 253, 243), time, 0, Z "Plane", Z "Plane");
		}
		
	}

	void ShowVisualCollisionReportNew(zCProgMeshProto* proto, const zVEC3& rayOrigin, const zVEC3& ray, int time, bool hitFoundGlobal, bool separate = false)
	{
		if (!hitFoundGlobal)
		{
			return;
		}

		if (separate)
		{
			// розовый луч
			debug.AddRay(rayOrigin, rayOrigin + ray, zCOLOR(253, 89, 196), time); // сам тестовый луч
		}

		debug.AddSphere(raycastReport.intersGlobal, 2.0, GFX_GREEN, time, false); // пересечение

		if (raycastReport.globalSubmeshBestNew)
		{


			// треугольник синий
			//debug.AddTriangle(pos0, pos1, pos2, zCOLOR(104, 104, 255), time);
		}

		if (separate)
		{
			debug.AddAxis(zVEC3(0, 0, 0), 200, time);
		}

		if (raycastReport.foundPlaneGlobal)
		{
			debug.AddPlane(raycastReport.foundPlaneGlobal, 200, zCOLOR(255, 65, 128), time, 0, Z "PlaneNew", Z "PlaneNew");
		}
	}


	void DrawVisualObject(zCProgMeshProto* proto, int time, int bestTriIndexOld)
	{
		for (auto& it : raycastReport.subMeshesFound)
		{
			for (int i = 0; i < it->triList.GetNum(); i++)
			{
				const zVEC3& pos0 = proto->posList[it->wedgeList[it->triList[i].wedge[0]].position];
				const zVEC3& pos1 = proto->posList[it->wedgeList[it->triList[i].wedge[1]].position];
				const zVEC3& pos2 = proto->posList[it->wedgeList[it->triList[i].wedge[2]].position];

				zCOLOR col = GFX_WHITE;

				if (it == raycastReport.globalSubmeshBestNew && i == raycastReport.bestTreeIndexGlobal)
				{
					col = GFX_GREEN;
				}
				else if (it == raycastReport.globalSubmeshBestOld && i == bestTriIndexOld)
				{
					col = GFX_RED;
				}

				//debug.AddTriangle(pos0, pos1, pos2, col, time);
			}

			if (pTraceMap.find(it) != pTraceMap.end())
			{
				pTraceMap[it].Render(NULL, proto, it);
				break;
			}
		}
	}

	void DrawObjectBVH(BVH_Tree* tree, BVHNode* node, int time)
	{
		if (!tree) return;

		zCProgMeshProto* proto = tree->proto;

		//cmd << "Draw: " << proto->GetVisualName() << " size: " << node->triIndices.size() << endl;

		for (int s = 0; s < proto->numSubMeshes; s++)
		{
			zCProgMeshProto::zCSubMesh* it = &(proto->subMeshList[s]);

			if (!it) continue;
			if (it->triPlaneList.GetNum() <= 0) continue;

			for (int i = 0; i < it->triList.GetNum(); i++)
			{

				bool contains = false;
				
				if (node) contains = (std::find(node->triIndices.begin(), node->triIndices.end(), i) != node->triIndices.end());

				zCOLOR col = GFX_WHITE;

				if (contains)
				{
					col = GFX_RED;
				};

				const zVEC3& pos0 = proto->posList[it->wedgeList[it->triList[i].wedge[0]].position];
				const zVEC3& pos1 = proto->posList[it->wedgeList[it->triList[i].wedge[1]].position];
				const zVEC3& pos2 = proto->posList[it->wedgeList[it->triList[i].wedge[2]].position];

				

				debug.AddTriangle(pos0, pos1, pos2, col, time);
			}

		}
		
	}

	void DrawBVH_Tree(BVHNode* node, int targetLevel, int currentLevel,
		zCOLOR leftColor = GFX_RED, zCOLOR rightColor = GFX_BLUE) {

		if (!node || currentLevel > targetLevel)
			return;

		// Если достигли нужного уровня
		if (currentLevel == targetLevel) {
			// Определяем цвет: левый (красный) или правый (синий)
			zCOLOR color = GFX_WHITE; // Корень (если targetLevel=0)

			if (node->parent) {
				color = (node == node->parent->left) ? GFX_RED : GFX_BLUE;
			}

			node->bbox.Draw(color);
			return;
		}

		// Рекурсивный обход без лишней отрисовки
		DrawBVH_Tree(node->left, targetLevel, currentLevel + 1);
		DrawBVH_Tree(node->right, targetLevel, currentLevel + 1);
	}

	BVH_Tree* pFoundDebugTree = NULL;
	extern void FindIntersectingNodes(
		BVHNode* root,
		const zTBBox3D& queryBBox,
		std::vector<BVHNode*>& result
	);

	extern void FindIntersectingNodesNew(
		BVHNode* root,
		const zTBBox3D& queryBBox,
		std::vector<BVHNode*>& result
	);

	void Raycast_Loop2()
	{
		if (zKeyPressed(KEY_F1))
		{
			zinput->ClearKeyBuffer();
			//
			auto pos = player->GetPositionWorld();
			auto dir = player->GetAtVectorWorld() * 1500;
			auto end = pos + dir;

			RX_Begin(50);
			ogame->GetWorld()->TraceRayNearestHit(pos, dir, (zCVob*)NULL, zTRACERAY_STAT_POLY | zTRACERAY_VOB_IGNORE_NO_CD_DYN | zTRACERAY_VOB_IGNORE_CHARACTER);
			RX_End(50);

			if (ogame->GetWorld()->traceRayReport.foundHit)
			{
				debug.AddVerticalLine(ogame->GetWorld()->traceRayReport.foundIntersection, 150, GFX_GREEN, 5000);
			}

			debug.AddLine(pos, end, GFX_RED, 5000);

			printWinC(RX_PerfString(50));


			RX_Begin(41);
			zVEC3 result;

			bool tryFound = bvhStaticTree.RayCast(pos, dir, result);

			RX_End(41);

			if (tryFound)
			{
				debug.AddVerticalLine(result + player->GetRightVectorWorld() * 10, 200, GFX_BLUE, 5000);
			}
			
			printWinC(RX_PerfString(41));
		}
	}

	void Raycast_Loop()
	{

#if !defined(DEBUG_LOOP_KEYS)

		return;
#endif

		if (player && !player->inventory2.IsOpen() && !ogame->singleStep && zinput->KeyPressed(KEY_F9))
		{
			isOldMethod = !isOldMethod;

			if (isOldMethod)
			{
				printWinC("Новый режим: ОТКЛЮЧЕН");

			}
			else
			{
				printWinC("Новый режим: ВКЛЮЧЕН");
			}


			zinput->ClearKeyBuffer();
		}

		if (!freezeDebug)
		{
			auto start = player->GetPositionWorld();
			auto dir = player->GetAtVectorWorld();
			
			bool traceTry = ogame->GetWorld()->TraceRayNearestHit(start, dir * 2000, (zCVob*)NULL, zTRACERAY_VOB_IGNORE_CHARACTER | zTRACERAY_VOB_IGNORE_NO_CD_DYN | zTRACERAY_STAT_POLY);

			if (traceTry)
			{
				auto& report = ogame->GetWorld()->traceRayReport;

				if (report.foundVob)
				{
					if (auto pVisual = report.foundVob->GetVisual())
					{
						if (auto pProto = pVisual->CastTo<zCProgMeshProto>())
						{
							for (int s = 0; s < pProto->numSubMeshes; s++)
							{
								zCProgMeshProto::zCSubMesh* subMesh = &(pProto->subMeshList[s]);

								if (subMesh && subMesh->triList.GetNum() >= 0)
								{
									auto& it = pTraceMap.find(subMesh);

									if (it != pTraceMap.end())
									{
										pFoundDebugTree = it->second.bvhTree;
									}
								}
							}
						}
					}
					
				}
			}
			
		}

		if (pFoundDebugTree)
		{
			DrawObjectBVH(pFoundDebugTree, NULL, 10);
			DrawBVH_Tree(pFoundDebugTree->root, maxLevel, 0);
		}

		if (zinput->KeyPressed(KEY_F4))
		{
			freezeDebug = !freezeDebug;

			if (!freezeDebug)
			{
				debug.CleanLines();
			}

			printWinC("freezeDebug: " + Z freezeDebug);

			zinput->ClearKeyBuffer();
		}


		if (zinput->KeyPressed(KEY_F1))
		{
			maxLevel -= 1;
			zClamp(maxLevel, 0, 100);
			zinput->ClearKeyBuffer();
			printWinC("maxLevel: " + Z maxLevel);
			//zrenderer->SetPolyDrawMode(zRND_DRAW_MATERIAL_WIRE);
		}

		if (zinput->KeyPressed(KEY_F2))
		{
			maxLevel += 1;
			zClamp(maxLevel, 0, 100);
			zinput->ClearKeyBuffer();
			printWinC("maxLevel: " + Z maxLevel);
			//zrenderer->SetPolyDrawMode(zRND_DRAW_MATERIAL_WIRE);
		}


		if (zinput->KeyPressed(KEY_F3) && pFoundDebugTree)
		{
			zinput->ClearKeyBuffer();
			printWinC("SearchBox: " + pFoundDebugTree->proto->GetVisualName());

			auto pos = player->GetPositionWorld() + zVEC3(0, 100, 0);
			auto dir = player->GetAtVectorWorld() * 1000;

			zTBBox3D bbox;
			bbox.Init();

			bbox.AddPoint(pos);
			bbox.AddPoint(pos + dir);

			std::vector<BVHNode*> found;

			debug.AddLine(pos, pos + dir, GFX_RED, 15000);
			debug.AddBbox(bbox, GFX_BLUE, 15000);

			RX_Begin(5);
			FindIntersectingNodes(pFoundDebugTree->root, bbox, found);
			RX_End(5);

			printWinC(RX_PerfString(5) + " " + Z(int)found.size());

			found.clear();

			RX_Begin(6);
			FindIntersectingNodesNew(pFoundDebugTree->root, bbox, found);
			RX_End(6);

			
			printWinC(RX_PerfString(6) + " " + Z (int)found.size());

			//zrenderer->SetPolyDrawMode(zRND_DRAW_MATERIAL_WIRE);
		}


		
		
		return;

		/*auto wld = ogame->GetWorld();
		auto pos = player->GetPositionWorld() + zVEC3(0, -10, 0);
		auto dir = player->GetAtVectorWorld() * 600;

		zCArray <zCVob*> ignore;
		ignore.Insert(player);*/

		/*
		
		*/
		
	}
}