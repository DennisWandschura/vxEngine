#include "Transition.h"
#include "Condition.h"

U8 Transition::isTriggered()
{
	return m_pCondition->test();
}