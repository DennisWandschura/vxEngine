using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class TargetState : DecisionTreeNode
    {
        List<Action> m_actions;
        State m_targetState;

        public TargetState(State targetState)
        {
            m_actions = new List<Action>();
            m_targetState = targetState;
        }

        public void addAction(Action a)
        {
            m_actions.Add(a);
        }

        public override TargetState makeDecision()
        {
            return this;
        }

        public List<Action> getActions()
        {
            return m_actions;
        }

        public State getTargetState()
        {
            return m_targetState;
        }
    }
}
