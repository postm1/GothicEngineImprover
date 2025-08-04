// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

	BVH_Tree::BVH_Tree()
	{
		subMesh = nullptr;
		proto = nullptr;
		root = nullptr;
	}

	// Add your code here . . .
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

#if defined (DEBUG_MEMORY_CHECK)
			SubMemoryInfo(sizeof(BVHNode), "DestroyTree (BVHNode)");
#endif
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

	// EXACT Ray-Triangle method
	zBOOL zCProgMeshProto::CheckRayPolyIntersectionExactMethod(zCProgMeshProto::zCSubMesh* subMesh, int triIndex, const zVEC3& rayOrigin, const zVEC3& ray, zVEC3& inters, zREAL& alpha)
	{
		const zVEC3& pos0 = posList[subMesh->wedgeList[subMesh->triList[triIndex].wedge[0]].position];
		const zVEC3& pos1 = posList[subMesh->wedgeList[subMesh->triList[triIndex].wedge[1]].position];
		const zVEC3& pos2 = posList[subMesh->wedgeList[subMesh->triList[triIndex].wedge[2]].position];

		const float EPSILON = 0.000001f;

		zVEC3 edge1 = pos1 - pos0;
		zVEC3 edge2 = pos2 - pos0;
		zVEC3 h = ray.Cross(edge2);
		float a = edge1.Dot(h);

		if (a > -EPSILON && a < EPSILON)
			return false;    // Ray is parallel to the triangle

		float f = 1.0f / a;
		zVEC3 s = rayOrigin - pos0;
		float u = f * s.Dot(h);

		if (u < 0.0f || u > 1.0f)
			return false;

		zVEC3 q = s.Cross(edge1);
		float v = f * ray.Dot(q);

		if (v < 0.0f || u + v > 1.0f)
			return false;

		alpha = f * edge2.Dot(q);

		if (alpha > EPSILON)
		{
			inters = rayOrigin + ray * alpha;
			return true;
		}

		return false;
	};
}