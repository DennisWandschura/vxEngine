#pragma once

class ArrayAllocator;

#include <fbxsdk/fbxsdk_nsbegin.h>

class FbxManager;
class FbxIOSettings;

#include <fbxsdk/fbxsdk_nsend.h>

namespace physx
{
	class PxCooking;
}

#include <vxLib/File/FileHandle.h>
#include <vector>

enum class PhsyxMeshType : u32;

class FbxFactory
{
	FBXSDK_NAMESPACE::FbxManager* m_pFbxManager;
	FBXSDK_NAMESPACE::FbxIOSettings* m_pIOSettings;

public:
	FbxFactory();
	~FbxFactory();

	bool loadFile(const char *fbxFile, const std::string &meshDir, const std::string &animDir, PhsyxMeshType meshType, physx::PxCooking* cooking, std::vector<vx::FileHandle>* meshFiles, std::vector<vx::FileHandle>* animFiles, ArrayAllocator* meshDataAllocator);
};