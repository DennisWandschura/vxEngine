using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class DecisionTreeTransition : TransitionBase
    {
        TargetState m_targetState;
        DecisionTreeNode m_root;
        string m_debugText;

        public DecisionTreeTransition(DecisionTreeNode root, TargetState targetState, string debugText)
        {
            m_targetState = targetState;
            m_root = root;
            m_debugText = debugText;
        }

        TargetState makeDecision(DecisionTreeNode node)
        {
            TargetState target = null;
            if(node != null)
            {
               target = node.makeDecision();
            }

            return target;
        }

        public override bool isTriggered()
        {
            m_targetState = makeDecision(m_root);

            return (m_targetState != null);
        }

        public override State getTargetState()
        {
            return m_targetState.getTargetState();
        }

        public override List<Action> getActions()
        {
            List<Action> actions = null;

            if(m_targetState != null)
            {
                actions = m_targetState.getActions();
            }

            return actions;
        }

        public override string getDebugText()
        {
            return m_debugText;
        }
    }
}
