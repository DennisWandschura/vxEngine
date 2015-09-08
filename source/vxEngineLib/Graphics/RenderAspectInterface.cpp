#include <vxEngineLib/Graphics/RenderAspectInterface.h>
#include <csignal>

bool RenderAspectInterface::setSignalHandler(AbortSignalHandlerFun signalHandlerFn)
{
	auto previousHandler = std::signal(SIGABRT, signalHandlerFn);
	if (previousHandler == SIG_ERR)
	{
		return false;
	}

	return true;
}