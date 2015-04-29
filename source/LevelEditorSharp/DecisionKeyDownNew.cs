using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class DecisionKeyDownNew : Decision
    {
        Form1 m_editorForm;
        System.Windows.Forms.Keys m_key;

        public DecisionKeyDownNew(DecisionTreeNode trueNode, DecisionTreeNode falseNode, Form1 editorForm, System.Windows.Forms.Keys key)
            : base(trueNode, falseNode)
        {
            m_editorForm = editorForm;
            m_key = key;
        }

        public override DecisionTreeNode getBranch()
        {
            if (m_editorForm.isKeyDown(m_key))
            {
                Console.WriteLine("KeyDown");
                return m_trueNode;
            }

            return m_falseNode;
        }
    }
}
