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

		/*
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
		*/
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
			//debug.AddRay(rayOrigin, rayOrigin + ray, zCOLOR(253, 89, 196), time); // сам тестовый луч
		}

		//debug.AddSphere(raycastReport.intersGlobal, 2.0, GFX_GREEN, time, false); // пересечение

		if (raycastReport.globalSubmeshBestNew)
		{


			// треугольник синий
			//debug.AddTriangle(pos0, pos1, pos2, zCOLOR(104, 104, 255), time);
		}

		if (separate)
		{
			//debug.AddAxis(zVEC3(0, 0, 0), 200, time);
		}

		if (raycastReport.foundPlaneGlobal)
		{
			//debug.AddPlane(raycastReport.foundPlaneGlobal, 200, zCOLOR(255, 65, 128), time, 0, Z "PlaneNew", Z "PlaneNew");
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

	void RenderTestCast()
	{

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

		return;

		/*auto wld = ogame->GetWorld();
		auto pos = player->GetPositionWorld() + zVEC3(0, -10, 0);
		auto dir = player->GetAtVectorWorld() * 600;

		zCArray <zCVob*> ignore;
		ignore.Insert(player);*/

		/*
		if (KeyClick(KEY_F1))
		{
			maxLevel -= 1;
			zClamp(maxLevel, 0, 100);
			CLR_KEY(KEY_F1);
			printWinC("maxLevel: " + Z maxLevel);
			//zrenderer->SetPolyDrawMode(zRND_DRAW_MATERIAL_WIRE);
		}

		if (KeyClick(KEY_F2))
		{
			maxLevel += 1;
			zClamp(maxLevel, 0, 100);
			CLR_KEY(KEY_F2);
			printWinC("maxLevel: " + Z maxLevel);
			//zrenderer->SetPolyDrawMode(zRND_DRAW_MATERIAL_WIRE);
		}
		*/

	

		/*if (KeyClick(KEY_F4))
		{
			freezeDebug = !freezeDebug;

			if (!freezeDebug)
			{
				debug.CleanLines();
			}

			printWinC("freezeDebug: " + Z freezeDebug);

			CLR_KEY(KEY_F4);
		}*/
		
	}
}