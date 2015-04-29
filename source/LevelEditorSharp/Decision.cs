using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    abstract class Decision : DecisionTreeNode
    {
        protected DecisionTreeNode m_trueNode;
        protected DecisionTreeNode m_falseNode;

        public Decision(DecisionTreeNode trueNode, DecisionTreeNode falseNode)
        {
            m_trueNode = trueNode;
            m_falseNode = falseNode;
        }

        abstract public DecisionTreeNode getBranch();

        public override TargetState makeDecision()
        {
            var node = getBranch();
            TargetState result = null;

            if(node != null)
            {
                result = node.makeDecision();
            }

            return result;
        }
    }
}
