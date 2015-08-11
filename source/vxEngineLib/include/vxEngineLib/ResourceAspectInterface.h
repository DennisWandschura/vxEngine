#pragma once

class Material;

namespace Graphics
{
	class Texture;
}

namespace vx
{
	struct Animation;
	class MeshFile;
}

#include <vxEngineLib/Reference.h>
#include <vxLib/StringID.h>

class ResourceAspectInterface
{
protected:
	ResourceAspectInterface() {}

public:
	virtual ~ResourceAspectInterface() {}

	virtual Reference<Graphics::Texture> getTexture(const vx::StringID &sid) const = 0;
	virtual Reference<Material> getMaterial(const vx::StringID &sid) const = 0;
	virtual Reference<vx::MeshFile> getMesh(const vx::StringID &sid) const = 0;
	virtual Reference<vx::Animation> getAnimation(const vx::StringID &sid) const = 0;
};