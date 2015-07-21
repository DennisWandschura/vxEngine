using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionDeselectJointOrMeshInstance : Action
    {
         EditorForm m_editorForm;

         public ActionDeselectJointOrMeshInstance(EditorForm editorForm)
        {
            m_editorForm = editorForm;
        }

        public override void run()
        {
            m_editorForm.deselectMesh();
            m_editorForm.deselectJoint();
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
            return "ActionDeselectJointOrMeshInstance";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            return root;
        }
    }
}
