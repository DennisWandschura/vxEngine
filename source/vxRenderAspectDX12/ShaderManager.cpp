#include "ShaderManager.h"
#include <d3dcompiler.h>

namespace d3d
{
	struct ShaderManager::Entry
	{
		ID3D10Blob* data;
		ShaderType type;
	};

	ShaderManager::ShaderManager()
		:m_shaders()
	{

	}

	ShaderManager::~ShaderManager()
	{

	}

	void ShaderManager::shutdown()
	{
		m_shaders.clear();
	}

	bool ShaderManager::loadShader(const char* id, const wchar_t* name, ShaderType type)
	{
		auto sid = vx::make_sid(id);
		auto it = m_shaders.find(sid);
		if (it == m_shaders.end())
		{
			Entry entry;
			entry.data = nullptr;
			entry.type = type;

			auto hresult = D3DReadFileToBlob(name, &entry.data);
			if (hresult != 0)
				return false;

			m_shaders.insert(sid, entry);
		}

		return true;
	}

	const ID3D10Blob* ShaderManager::getShader(const char* name) const
	{
		auto sid = vx::make_sid(name);
		auto it = m_shaders.find(sid);

		return (it == m_shaders.end()) ? nullptr : it->data;
	}

	ID3D10Blob* ShaderManager::getShader(const char* name)
	{
		auto sid = vx::make_sid(name);
		auto it = m_shaders.find(sid);

		return (it == m_shaders.end()) ? nullptr : it->data;
	}
}