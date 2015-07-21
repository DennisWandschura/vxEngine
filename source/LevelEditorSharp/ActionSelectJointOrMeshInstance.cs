using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionSelectJointOrMeshInstance : Action
    {
        EditorForm m_editorForm;
        int m_mouseX, m_mouseY;
        uint m_index;

        public ActionSelectJointOrMeshInstance(EditorForm editorForm)
        {
            m_editorForm = editorForm;
            m_index = 0;
        }

        public override void run()
        {
            m_mouseX = m_editorForm.getMouseX();
            m_mouseY = m_editorForm.getMouseY();

            if(NativeMethods.selectJoint(m_mouseX, m_mouseY, ref m_index))
            {
                 m_editorForm.deselectMesh();
                 m_editorForm.setSelectedJoint(m_index);
            }
            else
            {
                ulong prevSelectedSid = 0;
                if(m_editorForm.selectMesh(m_mouseX, m_mouseY, out prevSelectedSid))
                {
                    m_editorForm.deselectJoint();
                }
            }
        }

        public override void undo()
        {
        }

        public override void redo()
        {
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            return null;
        }

        public override string ToString()
        {
            return"ActionSelectJoint";
        }

        public override ActionNode toNode()
        {
            return null;
        }

        public bool selected()
        {
            return false;
        }
    }
}
