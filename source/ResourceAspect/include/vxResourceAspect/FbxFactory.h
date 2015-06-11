#pragma once

#include <fbxsdk/fbxsdk_nsbegin.h>

class FbxManager;
class FbxIOSettings;

#include <fbxsdk/fbxsdk_nsend.h>

namespace physx
{
	class PxCooking;
}

class FbxFactory
{
	FBXSDK_NAMESPACE::FbxManager *m_pFbxManager;
	FBXSDK_NAMESPACE::FbxIOSettings *m_pIOSettings;

public:
	FbxFactory();
	~FbxFactory();

	bool loadFile(const char *fbxFile, physx::PxCooking* cooking);
};