#pragma once

#include "Squirrel.h"

class SquirrelVM
{
	typedef SQInteger(*SQFUNCTION)(HSQUIRRELVM);

	HSQUIRRELVM m_vm{ nullptr };

public:
	SquirrelVM();
	~SquirrelVM();

	void initialize();

	void registerGlobalFunction(SQFUNCTION f, const SQChar* name);

	bool doFile(const SQChar* file);
	bool loadFile(const SQChar* file);
	HSQUIRRELVM get(){ return m_vm; }
};