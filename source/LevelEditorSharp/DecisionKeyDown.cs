using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class DecisionKeyDown : Decision
    {
        System.Windows.Input.Key m_key;

        public DecisionKeyDown(DecisionTreeNode trueNode, DecisionTreeNode falseNode, System.Windows.Input.Key key)
            : base(trueNode, falseNode)
        {
            m_key = key;
        }

        public override DecisionTreeNode getBranch()
        {
            if (System.Windows.Input.Keyboard.IsKeyDown(m_key))
            {
                return m_trueNode;
            }

           return m_falseNode;
        }
    }
}
