#pragma once

#include <vxLib/types.h>

const F32 g_heightStanding = 1.8f;
const F32 g_heightCrouching = 0.45f;
const F32 g_gravity = -9.81f;

// default walking speed, 1.0 m/s
VX_GLOBALCONST __m128 g_vMoveVelocity = { 0.1f, 0.0f, 0.1f, 0 };