#pragma once

class Material;
class AudioFile;

namespace Graphics
{
	class Texture;
	class Font;
}

namespace vx
{
	struct Animation;
	class MeshFile;
	class FileEntry;
	union Variant;
}

#include <vxLib/StringID.h>
#include <vxEngineLib/FileEntry.h>
#include <vxLib/variant.h>
#include <vxEngineLib/event.h>

class ResourceAspectInterface
{
protected:
	ResourceAspectInterface() {}

public:
	virtual ~ResourceAspectInterface() {}

	virtual void requestLoadFile(const vx::FileEntry &fileEntry, vx::Variant arg) = 0;
	virtual void requestLoadFile(const vx::FileEntry &fileEntry, vx::Variant arg, const Event &evt) = 0;

	virtual const Graphics::Texture* getTexture(const vx::StringID &sid) const = 0;
	virtual const Material* getMaterial(const vx::StringID &sid) const = 0;
	virtual const vx::MeshFile* getMesh(const vx::StringID &sid) const = 0;
	virtual const vx::Animation* getAnimation(const vx::StringID &sid) const = 0;
	virtual const AudioFile* getAudioFile(const vx::StringID &sid) const = 0;
	virtual const Graphics::Font* getFontFile(const vx::StringID &sid) const = 0;
};