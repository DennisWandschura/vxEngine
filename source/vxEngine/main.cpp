#include "libraries.h"
#if _VX_EDITOR
#else
#include <vxLib/ScopeGuard.h>
#include "Engine.h"
#include "Logfile.h"
#include "Clock.h"
#include "enums.h"
#include "Locator.h"

#include "Scene.h"
#include "File.h"

#include "SmallObjAllocator.h"
#include "SmallObject.h"
#include "SceneFile.h"

F32 g_halfBounds = 10.0f;
F32 g_cellSize = 2.0f;
U32 count = (g_halfBounds + g_halfBounds + 1) / g_cellSize;

vx::uint3 g_voxelCount = { count, count, count };
vx::float3 totalSize = vx::float3(g_voxelCount) * g_cellSize;
vx::float3 g_voxelHalfDim = vx::float3(g_voxelCount) / 2.0f;

vx::float3 gridHalfSize = totalSize / 2.0f;
vx::float3 gridCellSize = gridHalfSize / g_voxelHalfDim;
vx::float3 m_invGridCellSize = 1.0f / gridCellSize;

U32 quantitizeRayDirection(const vx::float3 &dir)
{
	U32 b0 = (dir.z * 0.5f + 0.5f) * 7u;
	U32 b1 = (dir.y * 0.5f + 0.5f) * 7u;
	U32 b2 = (dir.x * 0.5f + 0.5f) * 7u;

	return (b0 << 21) | (b1 << 24) | (b2 << 27);
}

U32 quantitizeRayOrigin(const vx::float3 &o)
{
	//vx::int3 cellPos = vx::fma((o - m_center), m_invGridCellSize, g_voxelHalfDim);
	vx::int3 cellPos = vx::fma(o, m_invGridCellSize, g_voxelHalfDim);

	return cellPos.z | (cellPos.y << 7) | (cellPos.x << 14);
}

struct Ray1
{
	vx::float3 o;
	vx::float3 dir;
};

inline uint64_t splitBy3(unsigned int a)
{
	uint64_t x = a & 0x1fffff; // we only look at the first 21 bits

	x = (x | x << 32) & 0x1f00000000ffff;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
	x = (x | x << 16) & 0x1f0000ff0000ff;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
	x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
	x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
	x = (x | x << 2) & 0x1249249249249249;
	return x;
}

inline uint64_t mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z)
{
	uint64_t answer = 0;
	answer |= splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
	return answer;
}

/// Puts two zero bits in-between each of the lower 10 bits of the given value.
inline U32 bitsep2(U32 x)
{
	// http://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/
	x &= 0x000003ff;                  // x = ---- ---- ---- ---- ---- --98 7654 3210
	x = (x ^ (x << 16)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
	x = (x ^ (x << 8)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
	x = (x ^ (x << 4)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
	x = (x ^ (x << 2)) & 0x09249249; // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
	return x;
}

/// Inverse of bitsep2.
inline U32 bitcomp2(U32 x)
{
	// http://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/
	x &= 0x09249249;                  // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
	x = (x ^ (x >> 2)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
	x = (x ^ (x >> 4)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
	x = (x ^ (x >> 8)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
	x = (x ^ (x >> 16)) & 0x000003ff; // x = ---- ---- ---- ---- ---- --98 7654 3210
	return x;
}

/// Morton code for 3 dimensions.
inline U32 bitzip(U32 x, U32 y, U32 z)
{
	return (bitsep2(z) << 2) + (bitsep2(y) << 1) + bitsep2(x);
}

/// 3 dimensions from morton code.
inline vx::uint3 bitunzip3(U32 c)
{
	return vx::uint3(bitcomp2(c), bitcomp2(c >> 1), bitcomp2(c >> 2));
}

int main()
{
	auto result = mortonEncode_magicbits(127, 20, 100);
	auto result1 = bitzip(127, 20, 100);
	auto coords = bitunzip3(result1);
	/*const F32 cellSize = 0.396728516f;
	const F32 invGridCellSize = 2.52061534f;
	const U32 halfDim = 128 / 2;

	const float x = -1.5f;
	const float y = 1.2f;


	const int uint21_max = 0xffff | (1 << 17) | (1 << 18) | (1 << 19) | (1 << 20);

	const float voxelPosF_x = x * invGridCellSize + halfDim;

	const int voxelPos_x = int(voxelPosF_x);
	const float offset_x = (voxelPos_x + 1) - voxelPosF_x;

	const float voxelPosF_xx = (voxelPos_x + 1) - offset_x;

	const int compressed_offset = offset_x * uint21_max;
	const float voxelPos_y = (y * invGridCellSize + halfDim);
	//float y = ;

	const float uncompressed_offset = compressed_offset / float(uint21_max);

	const float xx = (voxelPosF_x - halfDim) * cellSize;

	const float voxelPosF_xxx = (voxelPos_x + 1) - uncompressed_offset;
	const float xxx = (voxelPosF_xxx - halfDim) * cellSize;*/



	SmallObjAllocator alloc(1 KBYTE);
	SmallObject::setAllocator(&alloc);

	Clock mainClock;
	Logfile mainLogfile(mainClock);

	mainLogfile.create("logfile.xml");
	SCOPE_EXIT
	{
		LOG(mainLogfile, "Shutting down Engine", false);
		mainLogfile.close();
	};

	Scene scene;
	Engine engine(mainLogfile);

	{
		SceneFile sceneFile;
		sceneFile.loadFromYAML("data/scenes/scene5.scene.yaml");
		sceneFile.saveToFile("data/scenes/scene5.scene");
	}

	SCOPE_EXIT
	{
		engine.shutdown();
	};

	LOG(mainLogfile, "Initializing Engine", false);
	if (!engine.initialize())
	{
		LOG_ERROR(mainLogfile, "Error initializing Engine !", false);
		return 1;
	}

	engine.requestLoadFile(FileEntry("scene5.scene", FileType::Scene), &scene);

	LOG(mainLogfile, "Starting", false);

	engine.start();

	Locator::reset();

	return 0;
}
#endif