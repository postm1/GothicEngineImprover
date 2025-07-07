// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {



	void AB_Debug::Init()
	{
		//cmd << level << endl;
		viewText = standardView();
		screen->InsertItem(viewText);
	}

	void AB_Debug::Loop()
	{
		if (showTraceLines)
		{
			ManageLines();
		}

	}

	// линий трейсинга
	void AB_Debug::ManageLines()
	{
		viewText->ClrPrintwin();

		for (int i = 0; i < pListPoints.GetNumInList(); i++)
		{
			auto entry = pListPoints.GetSafe(i);

			if (entry)
			{
				if (entry->time > 0)
				{
					entry->time -= ztimer->frameTimeFloat;

					if (entry->isBbox)
					{
						entry->bbox.Draw(entry->color);

						continue;
					}

					zlineCache->Line3D(entry->origin, entry->target, entry->color, FALSE);


					if (entry->textStart.Length() > 0)
					{
						zVEC3 vWorldPos = entry->target + zVEC3(0, 20, 0);

						// transform the world position to pixels on screen
						float fPixelPosX = FLT_MAX, fPixelPosY = FLT_MAX;
						if (vWorldPos[0] != FLT_MAX) {
							pCamera->Activate();
							zVEC3 vec = pCamera->camMatrix * vWorldPos;
							if (vec[2] > 0)
								pCamera->Project(&vec, fPixelPosX, fPixelPosY);
						}

						if (fPixelPosX != FLT_MAX) {
							fPixelPosX -= screen->nax(screen->FontSize(entry->textStart) / 2);
							int x = screen->anx(fPixelPosX);
							int y = screen->any(fPixelPosY);

							zClamp(y, screen->FontY(), 8191 - screen->FontY());
							zClamp(x, 0, 8191 - screen->FontSize(entry->textStart));


							viewText->Print(x, y, entry->textStart);
						}
					}


					if (entry->textEnd.Length() > 0)
					{
						zVEC3 vWorldPos = entry->target + zVEC3(0, 20, 0);

						// transform the world position to pixels on screen
						float fPixelPosX = FLT_MAX, fPixelPosY = FLT_MAX;
						if (vWorldPos[0] != FLT_MAX) {
							pCamera->Activate();
							zVEC3 vec = pCamera->camMatrix * vWorldPos;
							if (vec[2] > 0)
								pCamera->Project(&vec, fPixelPosX, fPixelPosY);
						}

						if (fPixelPosX != FLT_MAX) {
							fPixelPosX -= screen->nax(screen->FontSize(entry->textEnd) / 2);
							int x = screen->anx(fPixelPosX);
							int y = screen->any(fPixelPosY);

							zClamp(y, screen->FontY(), 8191 - screen->FontY());
							zClamp(x, 0, 8191 - screen->FontSize(entry->textEnd));


							viewText->Print(x, y, entry->textEnd);
						}
					}

				}
			}
		}
	}

	//

	DebugPointEntry* AB_Debug::AddBbox(zTBBox3D bbox, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd)
	{
		DebugPointEntry* pEntry = new DebugPointEntry();
		pEntry->color = color;
		pEntry->time = timeSeconds;
		pEntry->textEnd = textEnd;
		pEntry->textStart = textStart;
		pEntry->useZBuffer = useZBuffer;
		pEntry->isBbox = true;
		pListPoints.InsertEnd(pEntry);

		pEntry->bbox = bbox;

		return pEntry;
	}

	DebugPointEntry* AB_Debug::AddLine(zVEC3 pos, zVEC3 point, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd)
	{
		DebugPointEntry* pEntry = new DebugPointEntry();
		pEntry->origin = pos;
		pEntry->target = point;
		pEntry->color = color;
		pEntry->time = timeSeconds;
		pEntry->textEnd = textEnd;
		pEntry->textStart = textStart;
		pEntry->useZBuffer = useZBuffer;
		pListPoints.InsertEnd(pEntry);

		return pEntry;
	}

	// рисует луч, как и линия, просто на конце будут стрелки, чтобы показать, что это конечная точка луча
	DebugPointEntry* AB_Debug::AddRay(zVEC3 start, zVEC3 end, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd)
	{
		// Основная линия
		DebugPointEntry* pMainLine = AddLine(start, end, color, timeSeconds, useZBuffer, "", "");

		// Направление луча (нормализованное)
		zVEC3 dir = end - start;
		float length = dir.Length();
		if (length < 0.001f) return pMainLine; // защита от нулевой длины
		dir.Normalize();

		// Поиск перпендикулярного вектора (любого, не коллинеарного с dir)
		zVEC3 perp;
		if (fabs(dir.n[0]) > 0.5f) {
			perp = zVEC3(dir.n[1], -dir.n[0], 0.0f); // перпендикуляр в плоскости XY
		}
		else {
			perp = zVEC3(0.0f, dir.n[2], -dir.n[1]); // перпендикуляр в плоскости YZ
		}
		perp.Normalize();

		// Второй перпендикуляр (через векторное произведение)
		zVEC3 perp2 = dir.Cross(perp);
		perp2.Normalize();

		float angleDeg = 15.0f;
		float markerSize = length / 50.0f;
		zClamp(markerSize, 5, 500);
		// Угол в радианах
		float angleRad = ToRadians(angleDeg);
		float cosAngle = cos(angleRad);
		float sinAngle = sin(angleRad);

		// 4 "усика" под углом
		for (int i = 0; i < 4; ++i) {
			// Поворачиваем базовые перпендикуляры на 90° для каждого усика
			float currentAngle = i * (M_PI / 2.0f); // 0°, 90°, 180°, 270°
			zVEC3 rotatedPerp = perp * cos(currentAngle) + perp2 * sin(currentAngle);

			// Направление усика (наклонённое от основного луча)
			zVEC3 markerDir = dir * (-cosAngle) + rotatedPerp * sinAngle;
			markerDir.Normalize();

			// Конечная точка усика
			zVEC3 markerEnd = end + markerDir * markerSize;

			// Рисуем усик
			AddLine(end, markerEnd, color, timeSeconds, useZBuffer, "", "");
		}

		return pMainLine;
	}

	// рисует треугольник по 3-м точкам
	DebugPointEntry* AB_Debug::AddTriangle(zVEC3 a, zVEC3 b, zVEC3 c, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd)
	{
		AddLine(a, b, color, timeSeconds, useZBuffer, textStart, textEnd);
		AddLine(b, c, color, timeSeconds, useZBuffer, textStart, textEnd);

		auto result = AddLine(c, a, color, timeSeconds, useZBuffer, textStart, textEnd);

		return result;
	}

	// рисует сферу с заданным радиусом с заданным центром
	DebugPointEntry* AB_Debug::AddSphere(zVEC3 center, float radius, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd)
	{
		// Минимальное количество сегментов (даже для маленькой сферы)
		const int MIN_SEGMENTS = 12;
		int segments = 30;

		// Для очень маленьких сфер уменьшаем сегменты, но не ниже MIN_SEGMENTS
		if (radius < 5.0f) {
			segments = max(MIN_SEGMENTS, segments / 2); // Не меньше 12
		}

		const float angleStep = (2.0f * M_PI) / segments;

		// Окружность в плоскости XY
		for (int i = 0; i < segments; ++i) {
			float angle1 = angleStep * i;
			float angle2 = angleStep * (i + 1);

			zVEC3 p1 = center + zVEC3(cos(angle1) * radius, sin(angle1) * radius, 0);
			zVEC3 p2 = center + zVEC3(cos(angle2) * radius, sin(angle2) * radius, 0);
			AddLine(p1, p2, color, timeSeconds, useZBuffer, "", "");
		}

		// Окружность в плоскости XZ
		for (int i = 0; i < segments; ++i) {
			float angle1 = angleStep * i;
			float angle2 = angleStep * (i + 1);

			zVEC3 p1 = center + zVEC3(cos(angle1) * radius, 0, sin(angle1) * radius);
			zVEC3 p2 = center + zVEC3(cos(angle2) * radius, 0, sin(angle2) * radius);
			AddLine(p1, p2, color, timeSeconds, useZBuffer, "", "");
		}

		// Окружность в плоскости YZ
		for (int i = 0; i < segments; ++i) {
			float angle1 = angleStep * i;
			float angle2 = angleStep * (i + 1);

			zVEC3 p1 = center + zVEC3(0, cos(angle1) * radius, sin(angle1) * radius);
			zVEC3 p2 = center + zVEC3(0, cos(angle2) * radius, sin(angle2) * radius);
			AddLine(p1, p2, color, timeSeconds, useZBuffer, "", "");
		}

		return nullptr;
	}


	DebugPointEntry* AB_Debug::AddVerticalLine(zVEC3 pos, float height, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd)
	{
		DebugPointEntry* pEntry = new DebugPointEntry();
		pEntry->origin = pos;
		pEntry->target = pos + zVEC3(0, height, 0);
		pEntry->color = color;
		pEntry->time = timeSeconds;
		pEntry->textEnd = textEnd;
		pEntry->textStart = textStart;
		pEntry->useZBuffer = useZBuffer;
		pListPoints.InsertEnd(pEntry);

		return pEntry;
	}

	//DebugPointEntry* AddAxis(zVEC3 posStart, float lineSize, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart = "", zSTRING textEnd = "");

	DebugPointEntry* AB_Debug::AddAxis(zVEC3 posStart, float lineSize, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd)
	{
		DebugPointEntry* pEntry1 = new DebugPointEntry();
		pEntry1->origin = posStart;
		pEntry1->target = posStart + zVEC3(1, 0, 0) * lineSize;
		pEntry1->color = GFX_RED;
		pEntry1->time = timeSeconds;
		pEntry1->textEnd = textEnd;
		pEntry1->textStart = textStart;
		pEntry1->useZBuffer = useZBuffer;

		pListPoints.InsertEnd(pEntry1);


		DebugPointEntry* pEntry2 = new DebugPointEntry();
		pEntry2->origin = posStart;
		pEntry2->target = posStart + zVEC3(0, 1, 0) * lineSize;
		pEntry2->color = GFX_GREEN;
		pEntry2->time = timeSeconds;
		pEntry2->textEnd = textEnd;
		pEntry2->textStart = textStart;
		pEntry2->useZBuffer = useZBuffer;

		pListPoints.InsertEnd(pEntry2);


		DebugPointEntry* pEntry3 = new DebugPointEntry();
		pEntry3->origin = posStart;
		pEntry3->target = posStart + zVEC3(0, 0, 1) * lineSize;
		pEntry3->color = GFX_YELLOW;
		pEntry3->time = timeSeconds;
		pEntry3->textEnd = textEnd;
		pEntry3->textStart = textStart;
		pEntry3->useZBuffer = useZBuffer;

		pListPoints.InsertEnd(pEntry3);


		return pEntry1;
	}

	DebugPointEntry* AB_Debug::AddPlane(zTPlane* plane, float squareSize, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd)
	{
		DebugPointEntry* pEntryResult = NULL;

		zVEC3 normal = plane->normal;

		// Находим два вектора, которые будут лежать в плоскости
		// Это будут вектора, перпендикулярные нормали
		zVEC3 axis1, axis2;

		// Выбираем первую ось, которая не совпадает с нормалью
		if (std::abs(normal.n[0]) < std::abs(normal.n[1])) {
			if (std::abs(normal.n[0]) < std::abs(normal.n[2])) {
				axis1 = zVEC3(1, 0, 0); // x-ось
			}
			else {
				axis1 = zVEC3(0, 0, 1); // z-ось
			}
		}
		else {
			if (std::abs(normal.n[1]) < std::abs(normal.n[2])) {
				axis1 = zVEC3(0, 1, 0); // y-ось
			}
			else {
				axis1 = zVEC3(0, 0, 1); // z-ось
			}
		}

		// Вектор на плоскости перпендикулярен нормали
		axis1 = axis1.Cross(normal).Normalize();

		// Второй вектор на плоскости перпендикулярен обоим
		axis2 = normal.Cross(axis1).Normalize();

		// Размер квадрата
		float halfSize = squareSize / 2.0f;

		zCArray<zVEC3> points;

		points.InsertEnd(zVEC3(plane->distance, plane->distance, plane->distance) + axis1 * halfSize + axis2 * halfSize);
		points.InsertEnd(zVEC3(plane->distance, plane->distance, plane->distance) + axis1 * halfSize - axis2 * halfSize);
		points.InsertEnd(zVEC3(plane->distance, plane->distance, plane->distance) - axis1 * halfSize - axis2 * halfSize);
		points.InsertEnd(zVEC3(plane->distance, plane->distance, plane->distance) - axis1 * halfSize + axis2 * halfSize);

		for (int i = 0; i < 4; i++) {
			// Индекс следующей точки с учётом зацикливания
			int nextIndex = (i + 1) % 4;

			DebugPointEntry* pEntry = new DebugPointEntry();
			pEntry->origin = points[i];
			pEntry->target = points[nextIndex];  // Соединяем текущую точку с следующей
			pEntry->color = color;
			pEntry->time = timeSeconds;
			pEntry->textEnd = textEnd;
			pEntry->textStart = textStart;
			pEntry->useZBuffer = useZBuffer;

			textStart = "";
			textEnd = "";

			pListPoints.InsertEnd(pEntry);

			pEntryResult = pEntry;  // Сохраняем последний добавленный элемент
		}


		DebugPointEntry* pEntry1 = new DebugPointEntry();
		pEntry1->origin = points[0];  // P1
		pEntry1->target = points[2];  // P3 (диагональ 1)
		pEntry1->color = color;
		pEntry1->time = timeSeconds;
		pEntry1->textEnd = textEnd;
		pEntry1->textStart = textStart;
		pEntry1->useZBuffer = useZBuffer;

		pListPoints.InsertEnd(pEntry1);

		DebugPointEntry* pEntry2 = new DebugPointEntry();
		pEntry2->origin = points[1];  // P2
		pEntry2->target = points[3];  // P4 (диагональ 2)
		pEntry2->color = color;
		pEntry2->time = timeSeconds;
		pEntry2->textEnd = textEnd;
		pEntry2->textStart = textStart;
		pEntry2->useZBuffer = useZBuffer;

		pListPoints.InsertEnd(pEntry2);


		pEntryResult = pEntry2;

		return pEntryResult;
	}



	DebugPointEntry* AB_Debug::AddCircle(zVEC3 pos, float height, float radius, int num, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd)
	{
		DebugPointEntry* pEntryResult;
		float curAngle = 0;
		float step = (2 * M_PI) / num;

		for (int i = 0; i < num; i++)
		{
			DebugPointEntry* pEntry = new DebugPointEntry();
			pEntry->origin = pos + zVEC3(cos(curAngle) * radius, 0, sin(curAngle) * radius);
			pEntry->target = pEntry->origin + zVEC3(0, height, 0);
			pEntry->color = color;
			pEntry->time = timeSeconds;
			pEntry->textEnd = textEnd;
			pEntry->textStart = textStart;
			pEntry->useZBuffer = useZBuffer;

			pListPoints.InsertEnd(pEntry);

			curAngle += step;

			pEntryResult = pEntry;
		}


		return pEntryResult;
	}

	DebugPointEntry* AB_Debug::AddCircle2(zVEC3 pos, float height, float radius, int num, zCOLOR color, float timeSeconds, bool useZBuffer, zSTRING textStart, zSTRING textEnd) {
		DebugPointEntry* pEntryResult = NULL;
		float curAngle = 0;
		float step = (2 * M_PI) / num;

		for (int i = 0; i < num; i++)
		{

			// bottom - top
			DebugPointEntry* pEntry = new DebugPointEntry();
			pEntry->origin = pos + zVEC3(cos(curAngle) * radius, 0, sin(curAngle) * radius);
			pEntry->target = pEntry->origin + zVEC3(0, height, 0);
			pEntry->color = color;
			pEntry->time = timeSeconds;
			pEntry->textEnd = textEnd;
			pEntry->textStart = textStart;
			pEntry->useZBuffer = useZBuffer;

			pListPoints.InsertEnd(pEntry);
			pEntryResult = pEntry;

			// circle
			pEntry = new DebugPointEntry();
			zVEC3 currentStep = pos + zVEC3(cos(curAngle) * radius, 0, sin(curAngle) * radius);
			currentStep = currentStep + zVEC3(0, height, 0);;
			zVEC3 nextStep = pos + zVEC3(cos(curAngle + step) * radius, 0, sin(curAngle + step) * radius);
			nextStep = nextStep + zVEC3(0, height, 0);
			pEntry->origin = currentStep;
			pEntry->target = nextStep;
			pEntry->color = color;
			pEntry->time = timeSeconds;
			pEntry->useZBuffer = useZBuffer;

			pListPoints.InsertEnd(pEntry);

			curAngle += step;

			pEntryResult = pEntry;
		}


		return pEntryResult;
	}

	void AB_Debug::CleanLines()
	{
		cmd << "Clean Lines" << endl;

		pListPoints.DeleteListDatas();
	}


}