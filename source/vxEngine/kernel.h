#pragma once

#include <cudaStructures.h>

extern void launchCUDA(U32 normalTextureId);
extern void stopCuda();

extern void cudaUpdateCamera(const cu::Camera &camera);