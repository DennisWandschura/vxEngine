using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionDecisionTree : Action
    {
        DecisionTreeNode m_root;

        public ActionDecisionTree(DecisionTreeNode root)
        {
            m_root = root;
        }

        public override void run()
        {
            var targetState = m_root.makeDecision();
            if(targetState != null)
            {
                var actions = targetState.getActions();

                foreach (var action in actions)
                {
                    action.run();
                }
            }
        }

        public override bool isComplete()
        {
            return true;
        }
    }
}
