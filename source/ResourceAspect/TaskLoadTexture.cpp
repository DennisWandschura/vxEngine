#include "TaskLoadTexture.h"
#include <vxLib/StringID.h>
#include <Shlwapi.h>
#include <vxEngineLib/Graphics/Texture.h>
#include <vxEngineLib/Graphics/TextureFactory.h>
#include <vxResourceAspect/ResourceManager.h>

TaskLoadTexture::TaskLoadTexture(TaskLoadTextureDesc &&desc)
	:TaskLoadFile(std::move(desc.m_fileNameWithPath), desc.m_textureManager->getScratchAllocator(), desc.m_textureManager->getScratchAllocatorMutex(), std::move(desc.evt)),
	m_textureManager(desc.m_textureManager),
	m_sid(desc.m_sid)
{

}

TaskLoadTexture::~TaskLoadTexture()
{

}

TaskReturnType TaskLoadTexture::runImpl()
{
	static const auto ddsSid = vx::make_sid(".dds");
	static const auto pngSid = vx::make_sid(".png");

	auto tmp = m_textureManager->find(m_sid);
	if (tmp != nullptr)
	{
		return TaskReturnType::Success;
	}

	auto extension = PathFindExtensionA(m_fileNameWithPath.c_str());
	auto extensionSid = vx::make_sid(extension);

	u8* fileData = nullptr;
	u32 fileSize = 0;
	if (!loadFromFile(&fileData, &fileSize))
	{
		return TaskReturnType::Failure;
	}

	std::unique_lock<std::mutex> lock;
	bool result = false;
	Graphics::Texture texture;
	if (extensionSid == ddsSid)
	{
		auto dataAllocator = m_textureManager->lockDataAllocator(&lock);
		result = Graphics::TextureFactory::createDDSFromMemory(fileData, true, &texture, dataAllocator);
	}
	else if (extensionSid == pngSid)
	{
		auto dataAllocator = m_textureManager->lockDataAllocator(&lock);
		result = Graphics::TextureFactory::createPngFromMemory(fileData, fileSize, true, &texture, dataAllocator);
	}

	if (result)
	{
		auto ref = m_textureManager->insertEntry(m_sid, std::move(texture));
		VX_ASSERT(ref != nullptr);
	}

	return TaskReturnType::Success;
}