// Supported with union (c) 2020 Union team

// User API for zCBspTree
// Add your methods here

zBOOL zCBspTree::TraceRay_Union(const zVEC3& start,
	const zVEC3& end,
	const int		traceFlags,
	zVEC3& inters,
	zCPolygon*& hitPoly,
	zCArray<zCVob*>* vobList);