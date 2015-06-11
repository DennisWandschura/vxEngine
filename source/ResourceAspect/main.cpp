#include <vxResourceAspect/FbxFactory.h>
#include <PxPhysicsAPI.h>

#pragma comment(lib, "vxLib_sd.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "vxEngineLib_d.lib")
#pragma comment(lib, "libfbxsdk-mt.lib")
#pragma comment(lib, "PhysX3CHECKED_x64.lib")
#pragma comment(lib, "PhysX3CookingCHECKED_x64.lib")
#pragma comment(lib, "PhysX3CommonCHECKED_x64.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")

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

physx::PxDefaultAllocator s_defaultAllocatorCallback{};
UserErrorCallback s_defaultErrorCallback{};

#include <fstream>

struct input : public physx::PxInputData
{
	char* m_ptr;
	char* m_data;
	unsigned m_size;

public:
	input()
		:m_ptr(nullptr),
		m_data(nullptr),
		m_size(0)
	{

	}

	~input()
	{
		if (m_data)
		{
			delete[]m_data;
			m_ptr = nullptr;
			m_data = nullptr;
			m_size = 0;
		}
	}

	void create(const char* file)
	{
		std::ifstream inFile(file);

		inFile.seekg(0, std::ifstream::end);
		auto size = inFile.tellg();
		inFile.seekg(0, std::ifstream::beg);

		m_data = new char[size];
		inFile.read(m_data, size);
		m_ptr = m_data;
		m_size = size;
	}

	physx::PxU32 getLength() const
	{
		return m_size;
	}

	void seek(physx::PxU32 offset)
	{
		m_ptr = m_data + offset;
		if (m_ptr > m_data + m_size)
		{
			m_ptr = m_data + m_size;
		}
	}

	physx::PxU32 tell() const
	{
		return m_ptr - m_data;
	}

	physx::PxU32 read(void* dest, physx::PxU32 count)
	{
		auto readSize = count;
		auto last = m_data + m_size;
		auto next = m_ptr + count;
		
		if (next > last)
		{
			readSize = last - m_ptr;
		}

		memcpy(dest, m_ptr, readSize);

		return readSize;
	}
};

int wmain()
{
	physx::PxCooking* cooking{ nullptr };
	physx::PxPhysics* physics{ nullptr };

	PhysXHandler<physx::PxFoundation> pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_defaultAllocatorCallback, s_defaultErrorCallback);
	if (!pFoundation)
	{
		return false;
	}

	bool recordMemoryAllocations = true;
	physx::PxTolerancesScale toleranceScale;
	PhysXHandler<physx::PxPhysics> pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pFoundation, toleranceScale, recordMemoryAllocations);
	if (!pPhysics)
	{
		return false;
	}

	auto cookingParams = physx::PxCookingParams(toleranceScale);
	//cookingParams.meshWeldTolerance = 0.1f;
	//cookingParams.meshPreprocessParams = physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
	cookingParams.meshSizePerformanceTradeOff = 0.9f;
	PhysXHandler<physx::PxCooking> pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pFoundation, cookingParams);
	if (!pCooking)
	{
		return false;
	}

	cooking = pCooking.get();
	physics = pPhysics.get();

	FbxFactory factory;
	factory.loadFile("door_animated.fbx", cooking);

	return 0;
}