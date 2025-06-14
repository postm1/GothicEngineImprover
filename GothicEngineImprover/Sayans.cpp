// Supported with union (c) 2020 Union team
// Union SOURCE file


#ifdef SOMECRAT22

#include <bvh/v2/bvh.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/ray.h>
#include <bvh/v2/node.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/thread_pool.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/stack.h>
#include <bvh/v2/tri.h>

struct BVHAcceleration
{
    bvh::v2::Bvh<bvh::v2::Node<float, 3>> bvh;
    std::vector<bvh::v2::PrecomputedTri<float>> tris;
};


namespace GOTHIC_ENGINE {



using SubMeshF = zCProgMeshProto::zCSubMesh;

std::unordered_map<zCProgMeshProto*, std::vector<BVHAcceleration>> g_BVHAccelerationStructs;

bvh::v2::ThreadPool& GetBVHThreadPool()
{
    static bvh::v2::ThreadPool thread_pool;
    return thread_pool;
}

bvh::v2::ParallelExecutor& GetBVHExecutor(bvh::v2::ThreadPool& thread_pool)
{
    static bvh::v2::ParallelExecutor executor(thread_pool);
    return executor;
}

void __fastcall GenerateBVHAcelerationStructure(zCProgMeshProto* progMeshProto)
{
    bvh::v2::ThreadPool& thread_pool = GetBVHThreadPool();
    bvh::v2::ParallelExecutor& executor = GetBVHExecutor(thread_pool);
    for (int i = 0; i < progMeshProto->numSubMeshes; ++i)
    {
        BVHAcceleration bvhAcceleration;
        std::vector<bvh::v2::Tri<float, 3>> tris;

        SubMeshF* pMesh = &(progMeshProto->subMeshList[i]);

        for (int t = 0, tend = pMesh->triList.GetNum(); t < tend; ++t)
        {
            zCArrayAdapt<zVEC3>* posList = &progMeshProto->posList;

            zTPMTriangle tri = pMesh->triList.GetSafe(t);

            bvh::v2::Vec<float, 3> vec0, vec1, vec2;

            vec0[0] = progMeshProto->posList[pMesh->wedgeList[tri.wedge[0]].position].n[0];
            vec0[1] = progMeshProto->posList[pMesh->wedgeList[tri.wedge[0]].position].n[1];
            vec0[2] = progMeshProto->posList[pMesh->wedgeList[tri.wedge[0]].position].n[2];

            vec1[0] = progMeshProto->posList[pMesh->wedgeList[tri.wedge[1]].position].n[0];
            vec1[1] = progMeshProto->posList[pMesh->wedgeList[tri.wedge[1]].position].n[1];
            vec1[2] = progMeshProto->posList[pMesh->wedgeList[tri.wedge[1]].position].n[2];

            vec1[0] = progMeshProto->posList[pMesh->wedgeList[tri.wedge[2]].position].n[0];
            vec1[1] = progMeshProto->posList[pMesh->wedgeList[tri.wedge[2]].position].n[1];
            vec1[2] = progMeshProto->posList[pMesh->wedgeList[tri.wedge[2]].position].n[2];

            /*
            vec0[0] = posList->Array[pMesh->WedgeList.Array[tri->wedge[0]].position].position[0];
            vec0[1] = posList->Array[pMesh->WedgeList.Array[tri->wedge[0]].position].position[1];
            vec0[2] = posList->Array[pMesh->WedgeList.Array[tri->wedge[0]].position].position[2];
            vec1[0] = posList->Array[pMesh->WedgeList.Array[tri->wedge[1]].position].position[0];
            vec1[1] = posList->Array[pMesh->WedgeList.Array[tri->wedge[1]].position].position[1];
            vec1[2] = posList->Array[pMesh->WedgeList.Array[tri->wedge[1]].position].position[2];
            vec2[0] = posList->Array[pMesh->WedgeList.Array[tri->wedge[2]].position].position[0];
            vec2[1] = posList->Array[pMesh->WedgeList.Array[tri->wedge[2]].position].position[1];
            vec2[2] = posList->Array[pMesh->WedgeList.Array[tri->wedge[2]].position].position[2];
            */

            tris.emplace_back(vec0, vec1, vec2);
        }

        std::vector<bvh::v2::BBox<float, 3>> bboxes(tris.size());
        std::vector<bvh::v2::Vec<float, 3>> centers(tris.size());
        executor.for_each(0, tris.size(), [&](size_t begin, size_t end)
            {
                for (size_t i = begin; i < end; ++i)
                {
                    bboxes[i] = tris[i].get_bbox();
                    centers[i] = tris[i].get_center();
                }
            });

        typename bvh::v2::DefaultBuilder<bvh::v2::Node<float, 3>>::Config config;
        config.quality = bvh::v2::DefaultBuilder<bvh::v2::Node<float, 3>>::Quality::High;
        bvhAcceleration.bvh = std::move(bvh::v2::DefaultBuilder<bvh::v2::Node<float, 3>>::build(thread_pool, bboxes, centers, config));
        bvhAcceleration.tris.resize(tris.size());
        executor.for_each(0, tris.size(), [&](size_t begin, size_t end)
            {
                for (size_t i = begin; i < end; ++i)
                {
                    auto j = bvhAcceleration.bvh.prim_ids[i];
                    bvhAcceleration.tris[i] = tris[j];
                }
            });
        g_BVHAccelerationStructs[progMeshProto].emplace_back(std::move(bvhAcceleration));
    }
}

int __fastcall zCProgMeshProto_TraceRay(zCProgMeshProto* progMeshProto, DWORD _EDX, float* v1, float* v2, int flags, DWORD report)
{
    *reinterpret_cast<int*>(report + 0x00) = 0;
    *reinterpret_cast<DWORD*>(report + 0x08) = 0;

    auto it = g_BVHAccelerationStructs.find(progMeshProto);
    if (it == g_BVHAccelerationStructs.end())
    {
        GenerateBVHAcelerationStructure(progMeshProto);
        it = g_BVHAccelerationStructs.find(progMeshProto);
        if (it == g_BVHAccelerationStructs.end())
            return 0;
    }

    static constexpr size_t invalid_id = UINT_MAX;
    static constexpr size_t stack_size = 64;
    size_t prim_id;

    auto ray = bvh::v2::Ray<float, 3>{
        bvh::v2::Vec<float, 3>(v1[0], v1[1], v1[2]),
        bvh::v2::Vec<float, 3>(v2[0], v2[1], v2[2]),
        0.f,
        1.f
    };

    float u, v;
    zTPlane* pPlane = NULL;
    bvh::v2::SmallStack<bvh::v2::Bvh<bvh::v2::Node<float, 3>>::Index, stack_size> stack;
    for (int i = 0; i < progMeshProto->numSubMeshes; ++i)
    {
        SubMeshF* pMesh = &(progMeshProto->subMeshList[i]);
        if (!pMesh->triPlaneList.GetNum())
            continue;

        DWORD pMaterial = reinterpret_cast<DWORD>(pMesh->material);
        if (!(flags & 0x200))
        {
            // zTRACERAY_ALLOW_WATER
            if (*reinterpret_cast<int*>(pMaterial + 0x40) == 5)
                continue;
        }

        if (flags & 0x100)
        {
            // zTRACERAY_IGNORE_TRANSPARENT
            DWORD pTexture = *reinterpret_cast<DWORD*>(pMaterial + 0x34);
            if (*reinterpret_cast<BYTE*>(pMaterial + 0x70) != 1 || (pTexture && (*reinterpret_cast<BYTE*>(pTexture + 0x88) & 0x01)))
                continue;
        }

        // Material collision detection
        if (*reinterpret_cast<BYTE*>(pMaterial + 0x6C) & 0x10)
            continue;

        BVHAcceleration& bvhAcceleration = it->second[i];
        if (flags & 0x1000) // zTRACERAY_VOB_FIND_ANY_INTERS
        {
            prim_id = invalid_id;
            bvhAcceleration.bvh.intersect<true, false>(ray, bvhAcceleration.bvh.get_root().index, stack,
                [&](size_t begin, size_t end)
                {
                    for (size_t j = begin; j < end; ++j)
                    {
                        if (auto hit = bvhAcceleration.tris[j].intersect(ray))
                        {
                            prim_id = bvhAcceleration.bvh.prim_ids[j];
                            std::tie(ray.tmax, u, v) = *hit;
                            return prim_id != invalid_id;
                        }
                    }
                    return prim_id != invalid_id;
                });
            if (prim_id != invalid_id)
            {
               // pPlane = &pMesh->TriPlaneList.Array[pMesh->TriPlaneIndexList.Array[prim_id]];
                pPlane = &pMesh->triPlaneList[pMesh->triPlaneIndexList[prim_id]];

                *reinterpret_cast<int*>(report + 0x00) = 1; //report.intersect = 1;
                // Calculate intersection point based on distance
                *reinterpret_cast<float*>(report + 0x0C) = v1[0] + v2[0] * ray.tmax;
                *reinterpret_cast<float*>(report + 0x10) = v1[1] + v2[1] * ray.tmax;
                *reinterpret_cast<float*>(report + 0x14) = v1[2] + v2[2] * ray.tmax;
                break;
            }
        }
        else
        {
            prim_id = invalid_id;
            bvhAcceleration.bvh.intersect<false, true>(ray, bvhAcceleration.bvh.get_root().index, stack,
                [&](size_t begin, size_t end)
                {
                    for (size_t j = begin; j < end; ++j)
                    {
                        if (auto hit = bvhAcceleration.tris[j].intersect(ray))
                        {
                            prim_id = bvhAcceleration.bvh.prim_ids[j];
                            std::tie(ray.tmax, u, v) = *hit;
                        }
                    }
                    return prim_id != invalid_id;
                });
            if (prim_id != invalid_id)
            {
                //pPlane = &pMesh->TriPlaneList.Array[pMesh->TriPlaneIndexList.Array[prim_id]];
                pPlane = &pMesh->triPlaneList[pMesh->triPlaneIndexList[prim_id]];

                *reinterpret_cast<int*>(report + 0x00) = 1; //report.intersect = 1;
                // Calculate intersection point based on distance
                *reinterpret_cast<float*>(report + 0x0C) = v1[0] + v2[0] * ray.tmax;
                *reinterpret_cast<float*>(report + 0x10) = v1[1] + v2[1] * ray.tmax;
                *reinterpret_cast<float*>(report + 0x14) = v1[2] + v2[2] * ray.tmax;
            }
        }
    }

    // report.intersect != 0 && zTRACERAY_CALC_NORMAL
    if (*reinterpret_cast<int*>(report + 0x00) && (flags & 0x80))
    {
        //report.Normal = pPlane->Normal;
        *reinterpret_cast<float*>(report + 0x18) = pPlane->normal[0];
        *reinterpret_cast<float*>(report + 0x1C) = pPlane->normal[1];
        *reinterpret_cast<float*>(report + 0x20) = pPlane->normal[2];
    }
    return *reinterpret_cast<int*>(report + 0x00);


	// Add your code here . . .
}

#endif