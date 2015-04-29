using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ConditionEditorStateEditNavMesh : Condition
    {
        Form1 m_editorForm;

        public ConditionEditorStateEditNavMesh(Form1 editorForm)
        {
            m_editorForm = editorForm;
        }

        public override bool test()
        {
            if (m_editorForm.getEditorState() == EditorState.EditNavMesh)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}
