// Supported with union (c) 2020 Union team

// User API for zCWorld
// Add your methods here

zBOOL __fastcall zCWorld::TraceRayNearestHit_Union(const zVEC3& rayOrigin, const zVEC3& ray, const zCArray<zCVob*>* ignoreVobList, const int traceFlags);
zBOOL __fastcall zCWorld::TraceRayFirstHit_Union(const zVEC3& rayOrigin, const zVEC3& ray, const zCArray<zCVob*>* ignoreVobList, const int traceFlags);