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

#include <vxLib/StringID.h>

class ResourceAspectInterface
{
protected:
	ResourceAspectInterface() {}

public:
	virtual ~ResourceAspectInterface() {}

	virtual const Graphics::Texture* getTexture(const vx::StringID &sid) const = 0;
	virtual const Material* getMaterial(const vx::StringID &sid) const = 0;
	virtual const vx::MeshFile* getMesh(const vx::StringID &sid) const = 0;
	virtual const vx::Animation* getAnimation(const vx::StringID &sid) const = 0;
};