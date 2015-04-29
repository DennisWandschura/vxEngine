using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionMultiSelectNavMesh : Action
    {
        Form1 m_editorForm;

        public ActionMultiSelectNavMesh(Form1 editorForm)
        {
            m_editorForm = editorForm;
        }

        public override void run()
        {
            m_editorForm.multiSelectNavMeshVertex();
        }

        public override bool isComplete()
        {
            return true;
        }
    }
}
