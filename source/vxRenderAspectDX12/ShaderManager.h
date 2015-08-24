#pragma once

struct ID3D10Blob;

#include <vxLib/types.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>

enum ShaderType : u32
{
	Vertex,
	Geometry,
	Pixel
};

class ShaderManager
{
	struct Entry;

	vx::sorted_vector<vx::StringID, Entry> m_shaders;

public:
	ShaderManager();
	~ShaderManager();

	bool loadShader(const char* id, const wchar_t* name, ShaderType type);

	const ID3D10Blob* getShader(const char* name) const;
	ID3D10Blob* getShader(const char* name);
};