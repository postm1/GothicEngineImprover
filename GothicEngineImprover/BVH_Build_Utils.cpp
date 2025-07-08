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
}