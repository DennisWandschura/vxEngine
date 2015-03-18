#pragma once

#include <stdint.h>

extern void initializeCUDA(uint32_t rayLinkBufferId, uint32_t rayListBufferId);
extern void shutdownCUDA();

extern void cudaSortRayLinks(uint32_t rayLinkCount);
extern void cudaSortRayList(uint32_t rayCount);