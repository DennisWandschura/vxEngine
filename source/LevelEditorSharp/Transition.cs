using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class Transition : TransitionBase
    {
        Condition m_condition;
        State m_targetState;
        List<Action> m_actions;
        string m_debugText;

        public Transition(Condition condition, State targetState, string debugText)
        {
            m_condition = condition;
            m_targetState = targetState;
            m_actions = new List<Action>();
            m_debugText = debugText;
        }

        public void addAction(Action a)
        {
            m_actions.Add(a);
        }

        public override bool isTriggered()
        {
            return m_condition.test();
        }

        public override State getTargetState()
        {
            return m_targetState;
        }

        public override List<Action> getActions()
        {
            return m_actions;
        }

        public override string getDebugText()
        {
            return m_debugText;
        }
    }
}
