using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class DecisionSelectedJoint : Decision
    {
        ActionSelectJointOrMeshInstance m_action;

        public DecisionSelectedJoint(DecisionTreeNode trueNode, DecisionTreeNode falseNode, ActionSelectJointOrMeshInstance actionSelectJoint)
            : base(trueNode, falseNode)
        {
            m_action = actionSelectJoint;
        }

        public override DecisionTreeNode getBranch()
        {
            if (m_action.selected())
            {
                return m_trueNode;
            }
            else
                return m_falseNode;
        }
    }
}
