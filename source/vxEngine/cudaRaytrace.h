#pragma once

#include <cudaStructures.h>

extern void cudaInitialize(U32 normalTextureId, U32 bvhNodeCount, U32 maxTriangleCount, U32 maxLightCount);
extern void cudaDestroy();
extern void cudaUpdate(const cu::Camera &cameraCuda);

extern void cudaStartRaytrace();
extern void cudaStopRaytrace();

extern void cudaUpdateBVH(const cu::BVHNode* pBVHNodes, U32 nodeCount);
extern void cudaUpdateMeshTriangles(const cu::Triangle* pTriangles, U32 triangleCount);
extern void cudaUpdateLights(const cu::SphereLightData* pLights, U32 lightCount);