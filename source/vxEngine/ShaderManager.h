#pragma once

class Logfile;

#include <vxLib\gl\ProgramPipeline.h>
#include <vxLib\gl\ShaderProgram.h>
#include <vxLib\Container\sorted_vector.h>
#include <vxLib\StringID.h>
#include "Logfile.h"

class ShaderManager
{
	vx::sorted_vector<vx::StringID64, vx::gl::ProgramPipeline> m_programPipelines;
	vx::sorted_vector<vx::StringID64, vx::gl::ShaderProgram> m_shaderPrograms;
	Logfile &m_logfile;
	std::string m_dataDir;

	bool loadShaderInclude(const char *file);
	bool loadProgram(const char *file, vx::gl::ShaderProgramType type);
	bool useProgram(vx::gl::ProgramPipeline &pipe, const char *id);
	bool loadUseProgram(vx::gl::ProgramPipeline &pipe, const char *id, vx::gl::ShaderProgramType type, const std::string &dataDir);

	const vx::gl::ShaderProgram* getProgram(const char *id) const;

	bool loadIncludes(const std::string &dataDir);
	bool loadPipelines(const std::string &dataDir);

public:
	ShaderManager(Logfile &logfile);

	bool initialize(const std::string &dataDir);

	bool loadPipeline(const char *file);

	const vx::gl::ProgramPipeline* getPipeline(const char *id) const;
	const vx::gl::ProgramPipeline* getPipeline(const vx::StringID64 sid) const;
};