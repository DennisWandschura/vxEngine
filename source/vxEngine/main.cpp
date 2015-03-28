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

/*F32 g_halfBounds = 10.0f;
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

*/
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

#include <DirectXMath.h>

int main()
{
	// ivec3 voxelPos = ivec3(input.wsPosition * u_voxel.invGridCellSize + u_voxel.halfDim);
	const U32 s_voxelDimension = 128;
	const F32 halfDim = s_voxelDimension / 2.0f;
	const F32 gridSize = 30.0f;
	const F32 gridHalfSize = gridSize / 2.0f;
	const F32 gridCellSize = gridHalfSize / halfDim;
	const F32 invGridCellSize = 1.0f / gridCellSize;
	vx::float3 wsPosition(1.5f, 0.3f, -5.0f);

	auto voxelProjMatrix = vx::MatrixOrthographicOffCenterRH(-gridHalfSize, (float)gridHalfSize, -gridHalfSize, (float)gridHalfSize, 0.1f, (float)gridSize);



	const __m128 axisY = { 0, 1, 0, 0 };
	auto projectionMatrices0 = voxelProjMatrix * vx::MatrixRotationAxis(axisY, vx::degToRad(90.0f)) * vx::MatrixTranslation(-gridHalfSize, 0, 0);
	auto projectionMatrices2 = voxelProjMatrix * vx::MatrixTranslation(0, 0, -gridHalfSize);

	auto gridProjMatrix = vx::MatrixOrthographicOffCenterRH(-gridHalfSize, gridHalfSize, -gridHalfSize, gridHalfSize, 0.1f, gridSize);


	auto voxelPos1 = wsPosition * invGridCellSize + halfDim;

	__m128 voxelDim = {128, 128, 128, 0};
	__m128 oneHalf = { 0.5f, 0.5f, 1.0f, 0.f };
	__m128 oneHalf1 = { 0.5f, 0.5f, 0.f, 0.f };

	__m128 pp0 = { 1.5f, 0.3f, -5.0f, 1.0f };
	__m128 pp1 = { 2.5f, 0.3f, -4.5f, 1.0f };
	__m128 pp2 = { 1.9f, 1.3f, -5.0f, 1.0f };

	auto p = vx::Vector4Transform(projectionMatrices2, pp0);
	p = _mm_fmadd_ps(p, oneHalf, oneHalf1);
	p = _mm_mul_ps(p, voxelDim);

	///
	auto mat1 = vx::MatrixTranslation(gridHalfSize, gridHalfSize, gridHalfSize);
	//D3DXMatrixTranslation(&mat1, -g_voxelSpace[0].x, -g_voxelSpace[0].y, -g_voxelSpace[0].z);
	//D3DXMatrixScaling(&mat2, float(g_gridSizeX) / extent.x, float(g_gridSizeY) / extent.y, float(g_gridSizeZ) / extent.z);
	auto mat2 = vx::MatrixScaling(invGridCellSize, invGridCellSize, invGridCellSize);
	auto g_matWorldToVoxel = mat2 * mat1;
	//g_matWorldToVoxel = vx::MatrixTranspose(g_matWorldToVoxel);
	//D3DXMatrixMultiplyTranspose(&g_matWorldToVoxel, &mat1, &mat2);

	auto p0 = vx::Vector4Transform(g_matWorldToVoxel, pp0);
	auto p1 = vx::Vector4Transform(g_matWorldToVoxel, pp1);
	auto p2 = vx::Vector4Transform(g_matWorldToVoxel, pp2);

	// determine bounding box
	auto vMin = _mm_min_ps(p0, _mm_min_ps(p1, p2));
	auto vMax = _mm_max_ps(p0, _mm_max_ps(p1, p2));

	auto voxOrigMin = _mm_floor_ps(vMin);
	auto voxOrigMax = _mm_floor_ps( _mm_add_ps(vMax, vx::g_VXOne) );
	auto voxOrigExtent = _mm_sub_ps(voxOrigMax, voxOrigMin);

	// determine bounding box clipped to voxel grid
	auto voxMin = _mm_max_ps(voxOrigMin, vx::g_VXZero);
	auto voxMax = _mm_min_ps(voxOrigMax, voxelDim);

	const auto voxExtent = _mm_sub_ps(voxMax, voxMin);

	/*

	const float3 voxOrigMin = float3(floor(vMin.x),
		floor(vMin.y),
		floor(vMin.z));
	const float3 voxOrigMax = float3(floor(vMax.x + 1.0),
		floor(vMax.y + 1.0),
		floor(vMax.z + 1.0));

	const float3 voxOrigExtent = voxOrigMax - voxOrigMin;*/

	int klo = 0;
	klo++;
	// 4.20102549
	// 64



	//////////////////////////
/*	auto result = mortonEncode_magicbits(127, 20, 100);
	auto result1 = bitzip(127, 20, 100);
	auto coords = bitunzip3(result1);
	const F32 cellSize = 0.396728516f;
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
	const float xxx = (voxelPosF_xxx - halfDim) * cellSize;

	const int width = 960;
	const int tmpx = 5;
	const int tmpy = 11;

	const int index = tmpx + tmpy * width;

	const int tmp = index / width;
	const int tmp1 = index % width;

	const int cellLinkCount = 670;
	const int llok = (cellLinkCount + 64 - 1) / 64;*/

	SmallObjAllocator alloc(1 KBYTE);
	SmallObject::setAllocator(&alloc);

	Clock mainClock;
	Logfile mainLogfile(mainClock);

	auto cc = vx::cross(vx::float3(2, 1, 0), vx::float3(2, 0, 0));

	mainLogfile.create("logfile.xml");
	SCOPE_EXIT
	{
		LOG(mainLogfile, "Shutting down Engine", false);
		mainLogfile.close();
	};

	Scene scene;
	Engine engine;

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