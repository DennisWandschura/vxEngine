using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class DecisionEditorMouseButtonPressed : Decision
    {
        Form1 m_editorForm;
        System.Windows.Forms.MouseButtons m_button;

        public DecisionEditorMouseButtonPressed(DecisionTreeNode trueNode, DecisionTreeNode falseNode, Form1 editorForm, System.Windows.Forms.MouseButtons button)
            : base(trueNode, falseNode)
        {
            m_editorForm = editorForm;
            m_button = button;
        }

        public override DecisionTreeNode getBranch()
        {
            if (m_editorForm.isMouseDown() &&
                m_editorForm.getLastClickedMouseButton() == m_button)
                return m_trueNode;
            else
                return m_falseNode;
        }
    }
}
