/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <memory>
#include <Shlwapi.h>
#include <PhysX/PxPhysicsAPI.h>
#include <vxLib/ScopeGuard.h>
#include <vxLib/File/File.h>
#include <vxEngineLib/MeshFile.h>
#include <vxLib/File/FileHeader.h>

#ifdef _DEBUG
#pragma comment(lib, "vxLib_d.lib")
#pragma comment(lib, "vxEngineLib_d.lib")
#pragma comment(lib, "assimpd.lib")
#pragma comment(lib, "PhysX3CHECKED_x64.lib")
#pragma comment(lib, "PhysX3CookingCHECKED_x64.lib")
#pragma comment(lib, "PhysX3CommonCHECKED_x64.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#else
#pragma comment(lib, "vxLib.lib")
#pragma comment(lib, "assimp.lib")
#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Cooking_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#endif
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Shlwapi.lib")

const std::string g_meshDir{ "../../game/data/mesh/" };
const std::string g_extension{ ".mesh" };

physx::PxCooking* g_pCooking{ nullptr };

bool createPhysXMesh(const void* vertices, u32 vertexCount, const u32* indices, u32 indexCount, physx::PxDefaultMemoryOutputStream* writeBuffer)
{
	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = vertexCount;
	meshDesc.points.stride = sizeof(vx::float3);
	meshDesc.points.data = vertices;

	meshDesc.triangles.count = indexCount / 3;
	meshDesc.triangles.stride = 3 * sizeof(u32);
	meshDesc.triangles.data = indices;

	bool status = g_pCooking->cookTriangleMesh(meshDesc, *writeBuffer);

	return status;
}

bool import(const char *file)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// propably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(file, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_TransformUVCoords);

	// If the import failed, report it
	if (!scene)
	{
		puts("error importing scene");
		return false;
	}

	std::string filename = PathFindFileName(file);

	std::string meshFileName;
	for (auto i = 0u; i < scene->mNumMeshes; ++i)
	{
		auto pMesh = scene->mMeshes[i];
		u32 vertexCount = pMesh->mNumVertices;
		u32 faceCount = pMesh->mNumFaces;
		u32 indexCount = faceCount * 3;

		if (!pMesh->HasNormals() || !pMesh->HasTangentsAndBitangents())
		{
			printf("Error: Mesh %s has no normals/tangents/bitangents !\n", pMesh->mName);
			return false;
		}

		auto totalSize = sizeof(vx::MeshVertex) * vertexCount + sizeof(u32) * indexCount;
		auto pMem = std::make_unique<u8[]>(totalSize);
		vx::MeshVertex* vertices = (vx::MeshVertex*)pMem.get();

		for (auto j = 0u; j < vertexCount; ++j)
		{
			vertices[j].position = vx::float3(pMesh->mVertices[j].x, pMesh->mVertices[j].y, pMesh->mVertices[j].z);
			vertices[j].texCoords = vx::float2(pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y);
			vertices[j].normal = vx::float3(pMesh->mNormals[j].x, pMesh->mNormals[j].y, pMesh->mNormals[j].z);
			vertices[j].tangent = vx::float3(pMesh->mTangents[j].x, pMesh->mTangents[j].y, pMesh->mTangents[j].z);
			vertices[j].bitangent = vx::float3(pMesh->mBitangents[j].x, pMesh->mBitangents[j].y, pMesh->mBitangents[j].z);
		}

		auto indices = (u32*)(pMem.get() + sizeof(vx::MeshVertex) * vertexCount);

		auto pFaces = pMesh->mFaces;
		u32 indexIndex = 0;
		for (auto j = 0u; j < faceCount; ++j)
		{
			auto &face = pFaces[j];

			assert(face.mNumIndices == 3);
			for (auto i = 0u; i < 3; ++i)
			{
				auto index = face.mIndices[i];

				indices[indexIndex] = index;
				++indexIndex;
			}
		}

		printf("trying to create physx mesh\n");
		physx::PxDefaultMemoryOutputStream writeBuffer;
		if (!createPhysXMesh(pMesh->mVertices, vertexCount, indices, indexCount, &writeBuffer))
		{
			printf("Error creating physx mesh\n");
			return false;
		}
		printf("created physx mesh\n");

		printf("creating mesh object\n");
		vx::Mesh mesh(pMem.release(), vertexCount, indexCount);

		std::string meshName = pMesh->mName.C_Str();
		if (strcmp(meshName.c_str(), "") == 0)
		{
			meshName = filename + "_mesh" + std::to_string(i);
		}

		meshFileName = g_meshDir + meshName + g_extension;

		printf("creating MeshFile object\n");
		vx::MeshFile meshFile(std::move(mesh), writeBuffer.getData(), writeBuffer.getSize());

		printf("creating File header\n");
		vx::FileHeader header;
		header.magic = vx::FileHeader::s_magic;
		printf("version\n");
		header.version = meshFile.getVersion();
		printf("crc\n");
		header.crc = meshFile.getCrc();

		if (header.crc == 0)
		{
			return false;
		}

		printf("creating file\n");
		vx::File file;
		if (!file.create(meshFileName.c_str(), vx::FileAccess::Write))
		{
			printf("error creating file\n");
			return false;
		}

		file.write(header);

		printf("trying to save mesh: %s\n", meshName.c_str());
		if (!meshFile.saveToFile(&file))
		{
			auto error = GetLastError();
			printf("error %d saving to file %s\n", error, meshName.c_str());
			return false;
		}

		printf("saved mesh: %s\n", meshName.c_str());
	}

	return true;
}

physx::PxDefaultAllocator s_defaultAllocatorCallback{};

class UserErrorCallback : public physx::PxErrorCallback
{
public:
	void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
	{
		printf("Physx Error: %d, %s %s %d\n", code, message, file, line);
	}
};

template<typename T>
struct PhysXHandler
{
	T* m_object;

	PhysXHandler() :m_object(nullptr){}

	PhysXHandler(T* object) :m_object(object){}

	~PhysXHandler()
	{
		if (m_object)
		{
			m_object->release();
			m_object = nullptr;
		}
	}

	T* operator->()
	{
		return m_object;
	}

	T& operator*()
	{
		return *m_object;
	}

	const T& operator*() const
	{
		return *m_object;
	}

	operator bool()
	{
		return (m_object != nullptr);
	}

	T* get()
	{
		return m_object;
	}
};

int main(int argc, char *argv[], char *envp[])
{
	UserErrorCallback s_defaultErrorCallback;

	if (argc < 3)
	{
		puts("not enough args");
		return 1;
	}
	PhysXHandler<physx::PxFoundation> pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_defaultAllocatorCallback, s_defaultErrorCallback);
	if (!pFoundation)
	{
		return 1;
	}

	bool recordMemoryAllocations = true;
	physx::PxTolerancesScale toleranceScale;
	PhysXHandler<physx::PxPhysics> pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pFoundation, toleranceScale, recordMemoryAllocations);
	if (!pPhysics)
	{
		return 1;
	}

	PhysXHandler<physx::PxCooking> pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pFoundation, physx::PxCookingParams(toleranceScale));
	if (!pCooking)
	{
		return 1;
	}

	g_pCooking = pCooking.get();

	int mode = StrToInt(argv[1]);
	std::string file = argv[2];
	if (mode == 0)
	{
		if (!import(file.c_str()))
		{
			puts("Import failed !");
			return 1;
		}
		puts("Import successfull !");
	}
	else if (mode == 1)
	{
		if (argc != 5)
		{
			puts("not enough args");
			return 1;
		}

		std::string file2 = argv[3];
		std::string outfile = argv[4];
	}

	g_pCooking = nullptr;

	return 0;
}