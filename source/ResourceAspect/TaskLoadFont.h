#pragma once

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

namespace Graphics
{
	class Font;
	class Texture;
}

template<typename T>
class ResourceManager;

class ResourceAspect;

#include "TaskLoadFile.h"
#include <vxLib/StringID.h>
#include <vxEngineLib/Graphics/FontAtlas.h>

struct TaskLoadFontDesc
{
	ResourceManager<Graphics::Font>* m_fontManager;
	ResourceManager<Graphics::Texture>* m_textureManager;
	std::string m_fileNameWithPath;
	std::string m_filename;
	std::string m_path;
	vx::StringID m_sid;
	Event evt;
	ResourceAspect* m_resourceAspect;
};

class TaskLoadFont : public TaskLoadFile
{
	std::string m_filename;
	ResourceManager<Graphics::Font>* m_fontManager;
	ResourceManager<Graphics::Texture>* m_textureManager;
	vx::StringID m_sid;
	std::string m_path;
	Graphics::FontAtlas m_fontAtlas;
	std::string m_textureName;
	ResourceAspect* m_resourceAspect;
	bool m_waitingForTexture;

	bool loadFontData();
	void requestLoadTexture();

	TaskReturnType runImpl() override;

public:
	explicit TaskLoadFont(TaskLoadFontDesc &&desc);
	~TaskLoadFont();

	f32 getTimeMs() const override;
};