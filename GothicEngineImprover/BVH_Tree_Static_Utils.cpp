// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	// Add your code here . . .
	BVH_TreeStatic::BVH_TreeStatic()
	{
		root = nullptr;
	}


	void BVH_TreeStatic::DestroyTree(BVHNodeStatic*& root)
	{
		if (!root) return;

		std::stack<BVHNodeStatic*> nodes;

		nodes.push(root);
		root = nullptr;

		while (!nodes.empty())
		{
			BVHNodeStatic* node = nodes.top();
			nodes.pop();

			node->parent = NULL;

			// Добавляем детей в стек
			if (node->left) nodes.push(node->left);
			if (node->right) nodes.push(node->right);

			delete node;
		}
	}


	zVEC3 BVH_TreeStatic::GetTriangleCenter(zCPolygon* poly)
	{
		return poly->GetCenter();
	}


	void BVH_TreeStatic::ScaleBboxes(BVHNodeStatic* node)
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