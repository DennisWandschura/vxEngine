#include "ShaderManager.h"
#include <d3dcompiler.h>

namespace d3d
{
	struct ShaderManager::Entry
	{
		ID3D10Blob* data;
		//ShaderType type;
	};

	ShaderManager::ShaderManager()
		:m_shaders()
	{

	}

	ShaderManager::~ShaderManager()
	{

	}

	void ShaderManager::initialize(const wchar_t(&rootDir)[16])
	{
		memcpy(m_rootDir, rootDir, sizeof(wchar_t) * 16);
	}

	void ShaderManager::shutdown()
	{
		m_shaders.clear();
	}

	bool ShaderManager::loadShader(const wchar_t* name)
	{
		auto sid = vx::make_sid(name);
		auto it = m_shaders.find(sid);
		if (it == m_shaders.end())
		{
			Entry entry;
			entry.data = nullptr;
			//entry.type = type;

			wchar_t buffer[64];
			swprintf(buffer, 64, L"%ws%ws", m_rootDir, name);

			auto hresult = D3DReadFileToBlob(buffer, &entry.data);
			if (hresult != 0)
				return false;

			m_shaders.insert(sid, entry);
		}

		return true;
	}

	const ID3D10Blob* ShaderManager::getShader(const wchar_t* name) const
	{
		auto sid = vx::make_sid(name);
		auto it = m_shaders.find(sid);

		return (it == m_shaders.end()) ? nullptr : it->data;
	}

	ID3D10Blob* ShaderManager::getShader(const wchar_t* name)
	{
		auto sid = vx::make_sid(name);
		auto it = m_shaders.find(sid);

		return (it == m_shaders.end()) ? nullptr : it->data;
	}
}