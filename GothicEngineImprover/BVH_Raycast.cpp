// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {



	zBOOL TraceBVHNodeStack(const zVEC3& rayOrigin, const zVEC3& rayDir, zCSubMeshStruct* meshEntry)
	{
		BVHNode* stack[100];  // Статический стек с запасом
		int stackPtr = 0;    // Указатель на вершину стека

		stack[stackPtr++] = meshEntry->bvhTree->root;  // Помещаем корень


		zBOOL hitFound = FALSE;
		zVEC3 inters;
		zREAL alpha = 9999;
		zREAL tmin, tmax;

		auto proto = meshEntry->parentProto;
		auto subMesh = meshEntry->subMesh;

		while (stackPtr > 0)
		{
			BVHNode* node = stack[--stackPtr];  // Быстрый pop

#if defined(DEBUG_BUILD_BVH)
			globalStackDepth = max(globalStackDepth, stackPtr);
#endif

			//raycastReport.NodeTreeCheckCounter++;
			//tmin = tmax = 1.0f;

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
			for (int triIdx : node->triIndices)
			{
				//raycastReport.TrisTreeCheckCounter++;



				if (proto->CheckRayPolyIntersection(subMesh, triIdx, rayOrigin, rayDir, inters, alpha))
				{
					hitFound = true;

					if (alpha < raycastReport.bestAlphaGlobal)
					{
						//cmd << alpha << " < " << bestAlphaGlobal << endl;

						raycastReport.bestAlphaGlobal = alpha;
						raycastReport.bestTreeIndexGlobal = triIdx;
						raycastReport.intersGlobal = inters;
						raycastReport.foundPlaneGlobal = &(subMesh->triPlaneList[subMesh->triPlaneIndexList[raycastReport.bestTreeIndexGlobal]]);
					}


					// FIRSTHIT
					if (raycastReport.globalFirstHitMode)
					{
						return true;
					}
				}
			}


			// Добавляем дочерние узлы в стек (правый -> левый для DFS)
			if (node->right) stack[stackPtr++] = node->right;
			if (node->left)  stack[stackPtr++] = node->left;

		}

		return hitFound;
	}

	zBOOL TraceRayBVH(const zVEC3 rayOrigin, const zVEC3 rayDir, zCSubMeshStruct* meshEntry)
	{
		raycastReport.ClearCurrrentTrace();

		auto result = TraceBVHNodeStack(rayOrigin, rayDir, meshEntry);

		if (result)
		{
			raycastReport.globalSubmeshBestNew = meshEntry->subMesh;
		}

		/*cmd << "Nodes: " << raycastReport.NodeTreeCheckCounter
			<< " Tris: " << raycastReport.TrisTreeCheckCounter << "/" << meshEntry->subMesh->triList.GetNum()
			<< endl;*/

		return result;

	}

#if defined(DEF_PERF_APPLY) || defined(DEF_PERF_UPDATE)

	/*
	HOOK ivk_zCWorld_TraceRayFirstHit  AS(&zCWorld::TraceRayFirstHit, &zCWorld::TraceRayFirstHit_Union);
	zBOOL __fastcall zCWorld::TraceRayFirstHit_Union(const zVEC3& rayOrigin, const zVEC3& ray, const zCArray<zCVob*>* ignoreVobList, const int traceFlags)
	{
		flagsTraceHistory.push_back(traceFlags);

		RX_Perf_Start("zCWorld::TraceRayFirstHit_Union", PerfType::PERF_TYPE_PER_FRAME);

		auto result = THISCALL(ivk_zCWorld_TraceRayFirstHit)(rayOrigin, ray, ignoreVobList, traceFlags);

		RX_Perf_End("zCWorld::TraceRayFirstHit_Union");

		return result;
	}

	HOOK ivk_zCWorld_TraceRayNearestHit  AS(&zCWorld::TraceRayNearestHit, &zCWorld::TraceRayNearestHit_Union);
	zBOOL __fastcall zCWorld::TraceRayNearestHit_Union(const zVEC3& rayOrigin, const zVEC3& ray, const zCArray<zCVob*>* ignoreVobList, const int traceFlags)
	{
		flagsTraceHistory.push_back(traceFlags);

		RX_Perf_Start("zCWorld::ivk_zCWorld_TraceRayNearestHit", PerfType::PERF_TYPE_PER_FRAME);

		auto result = THISCALL(ivk_zCWorld_TraceRayNearestHit)(rayOrigin, ray, ignoreVobList, traceFlags);

		RX_Perf_End("zCWorld::ivk_zCWorld_TraceRayNearestHit");

		return result;
	}


	HOOK ivk_zCVisual_TraceRay  AS(&zCVisual::TraceRay, &zCVisual::TraceRay_Union);
	zBOOL zCVisual::TraceRay_Union(const zVEC3& rayOrigin, const zVEC3& ray, const int traceFlags, zTTraceRayReport& report)
	{
		RX_Perf_Start("zCVisual::TraceRay_Union", PerfType::PERF_TYPE_PER_FRAME);

		auto result = THISCALL(ivk_zCVisual_TraceRay)(rayOrigin, ray, traceFlags, report);

		RX_Perf_End("zCVisual::TraceRay_Union");

		return result;
	}
	*/

#endif




	HOOK ivk_zCProgMeshProto_TraceRay  AS(&zCProgMeshProto::TraceRay, &zCProgMeshProto::TraceRay_Union);
	zBOOL zCProgMeshProto::TraceRay_Union(const zVEC3& rayOrigin, const zVEC3& ray, const int traceFlags, zTTraceRayReport& report)
	{

		RX_Perf_Start("zCProgMeshProto::TraceRay_Union", PerfType::PERF_TYPE_PER_FRAME);

		const int testTwoSided = traceFlags & zTRACERAY_POLY_2SIDED;

		// Если тест двусторонний => выходим (этот почти нигде не юзается)
		// Или если уровень не загрузился
		// Или игра на паузе

		if (testTwoSided || !OnLevelFullLoaded_Once || isOldMethod || ogame->singleStep)
		{
			zBOOL result = THISCALL(ivk_zCProgMeshProto_TraceRay)(rayOrigin, ray, traceFlags, report);

			RX_Perf_End("zCProgMeshProto::TraceRay_Union");

			return result;
		}

		// Ставим настройки трейсера
		const bool firstHitMode = (traceFlags & zTRACERAY_FIRSTHIT);
		const bool testWater = (traceFlags & zTRACERAY_POLY_TEST_WATER);
		const bool ignoreTransp = (traceFlags & zTRACERAY_POLY_IGNORE_TRANSP);
		const bool polyNormal = (traceFlags & zTRACERAY_POLY_NORMAL);


		report.foundHit = FALSE;
		report.foundPoly = 0;


		// чистим нашу структуру
		raycastReport.Clear();
		raycastReport.globalFirstHitMode = firstHitMode;



		// lineare Suche ueber die Tris hinweg
		zVEC3	inters;
		zREAL	alpha = 0.0;
		zREAL	bestAlpha = 999;
		int		bestTriIndex = 0;
		zTPlane* bestPlane = 0;




		for (int s = 0; s < numSubMeshes; s++)
		{
			zCSubMesh* subMesh = &(subMeshList[s]);

			if (!subMesh) continue;
			if (subMesh->triPlaneList.GetNum() <= 0) continue;

			const zCMaterial* mat = subMesh->material;

			if (mat->noCollDet) continue;

			// Traceflags: zTRACERAY_POLY_TEST_WATER, zTRACERAY_POLY_IGNORE_TRANSP
			if (mat->matGroup == zMAT_GROUP_WATER) {
				if (!testWater)			continue;
			}
			else {
				if ((ignoreTransp)) {
					if ((mat->rndAlphaBlendFunc != zRND_ALPHA_FUNC_NONE) || ((mat->texture) && (mat->texture->hasAlpha)))
						continue;
				};
			};

			
			//raycastReport.subMeshesFound.push_back(subMesh);


			auto& it = pTraceMap.find(subMesh);

			// Visual not found, build new bvh for this visual, if this frame has < 1.5 ms for building
			if (it == pTraceMap.end())
			{
				if ((visualLoadBVHTimeThisFrame / 1000.0f <= 1.5))
				{
					RX_Begin(55);
					zCSubMeshStruct collEntry;

					collEntry.BuildMap(this, subMesh);

					pTraceMap[subMesh] = collEntry;

					it = pTraceMap.find(subMesh);

					RX_End(55);
					visualLoadBVHTimeThisFrame += perf[55];

					//cmd << "NewObject: " << pTraceMap.size() << endl;
				}
				else
				{
					//cmd << "End frame" << endl;
				}
			}

			if (it != pTraceMap.end())
			{
				raycastReport.hitFoundGlobal |= TraceRayBVH(rayOrigin, ray, &it->second);




				/*
				for (int i = 0; i < subMesh->triList.GetNum(); i++)
				{
					zBOOL hit;
					if (testTwoSided)	hit = CheckRayPolyIntersection2Sided(subMesh, i, rayOrigin, ray, inters, alpha);
					else				hit = CheckRayPolyIntersection(subMesh, i, rayOrigin, ray, inters, alpha);


					if (hit)
					{
						// First Hit
						if (firstHitMode)
						{
							bestTriIndex = i;
							bestPlane = &(subMesh->triPlaneList[subMesh->triPlaneIndexList[bestTriIndex]]);
							raycastReport.globalSubmeshBestOld = subMesh;
							report.foundIntersection = inters;
							report.foundHit = TRUE;

							break;
						};
						// Nearest Hit
						if (alpha < bestAlpha) {
							bestAlpha = alpha;
							bestTriIndex = i;
							bestPlane = &(subMesh->triPlaneList[subMesh->triPlaneIndexList[bestTriIndex]]);
							raycastReport.globalSubmeshBestOld = subMesh;
							report.foundIntersection = inters;
							report.foundHit = TRUE;
						};
					};
				};
				*/



				
				
				/*
				if (false && !freezeDebug)
				{
					auto posFoundDiff = raycastReport.intersGlobal.Distance(report.foundIntersection);
					int time = 1000 * 1000;

					if (firstHitMode)
					{
						if (raycastReport.hitFoundGlobal != report.foundHit)
						{
							//debug.CleanLines();

							printWinC("FIRSTHIT");

							cmd
								<< "\n<< FIRSTHIT!!!: " << this->GetVisualName() << " - Wrong result!"
								<< " Hit: " << raycastReport.hitFoundGlobal << " | " << report.foundHit
								<< " Inters: " << raycastReport.intersGlobal.ToString() << " | " << report.foundIntersection.ToString()
								<< " Plane: " << (int)raycastReport.foundPlaneGlobal << " | " << (int)bestPlane
								<< " Closest: " << rayOrigin.Distance(raycastReport.intersGlobal) << " | " << rayOrigin.Distance(report.foundIntersection)
								<< " TriIndex: " << (int)raycastReport.bestTreeIndexGlobal << " | " << (int)bestTriIndex
								<< " | Nodes: " << raycastReport.NodeTreeCheckCounter
								<< " | Tris: " << raycastReport.TrisTreeCheckCounter << "/" << subMesh->triList.GetNum()
								<< " | RayOrigin: " << rayOrigin.ToString()
								<< " RayDir: " << ray.ToString()

								<< endl;
							cmd << " ->> NodeByIndex: " << FindNodeByIndex(&it->second, bestTriIndex) << endl;
							cmd << "Path: " << nodeWayGlobal << endl;


							ShowVisualCollisionReport(this, rayOrigin, ray, time, report, bestTriIndex, bestPlane);

							ShowVisualCollisionReportNew(this, rayOrigin, ray, time, raycastReport.hitFoundGlobal);


							DrawVisualObject(this, time, bestTriIndex);
						}
					}
					else if (raycastReport.hitFoundGlobal != report.foundHit || report.foundHit && raycastReport.hitFoundGlobal == report.foundHit && posFoundDiff > 0.08 || bestPlane != raycastReport.foundPlaneGlobal || bestTriIndex != raycastReport.bestTreeIndexGlobal)
					{
						//debug.CleanLines();
						cmd
							<< "\nUSUAL: "
							<< this->GetVisualName()
							<< " Hit: " << raycastReport.hitFoundGlobal << " | " << report.foundHit
							<< " (P: " << raycastReport.intersGlobal.ToString() << " | " << report.foundIntersection.ToString()
							<< ") Diff: " << zSTRING(raycastReport.intersGlobal.Distance(report.foundIntersection), 10)
							<< " Closest: " << rayOrigin.Distance(raycastReport.intersGlobal) << " | " << rayOrigin.Distance(report.foundIntersection)
							<< " Plane: " << (int)raycastReport.foundPlaneGlobal << " | " << (int)bestPlane
							<< " Index: " << (int)raycastReport.bestTreeIndexGlobal << " | " << (int)bestTriIndex
							<< " | Nodes: " << raycastReport.NodeTreeCheckCounter
							<< " | Tris: " << raycastReport.TrisTreeCheckCounter << "/" << subMesh->triList.GetNum()
							<< " | RayOrigin: " << rayOrigin.ToString()
							<< " RayDir: " << ray.ToString()
							<< endl;

						cmd << " ->> NodeByIndex: " << FindNodeByIndex(&it->second, bestTriIndex) << endl;
						cmd << "Path: " << nodeWayGlobal << endl;


						printWinC("USUAL: " + zSTRING(posFoundDiff));


						ShowVisualCollisionReport(this, rayOrigin, ray, time, report, bestTriIndex, bestPlane);

						ShowVisualCollisionReportNew(this, rayOrigin, ray, time, raycastReport.hitFoundGlobal);


						DrawVisualObject(this, time, bestTriIndex);
					}
				}
				*/
			}
			// если запись не нашлась, вызывает обычный код, а новая запись будет добавлена на след. кадрах
			else
			{
				zBOOL result = THISCALL(ivk_zCProgMeshProto_TraceRay)(rayOrigin, ray, traceFlags, report);

				//cmd << "Return No entry found: " << this->GetVisualName() << endl;

				RX_Perf_End("zCProgMeshProto::TraceRay_Union");

				return result;
			}

		};

		if (raycastReport.hitFoundGlobal)
		{
			report.foundIntersection = raycastReport.intersGlobal;
			report.foundHit = TRUE;

			if (polyNormal)
			{
				report.foundPolyNormal = (*raycastReport.foundPlaneGlobal).normal;
			}
		}


		RX_Perf_End("zCProgMeshProto::TraceRay_Union");


		return report.foundHit;
	}
}