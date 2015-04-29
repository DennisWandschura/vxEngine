using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionCreateNavMeshVertex : Action
    {
         Form1 m_editorForm;

         public ActionCreateNavMeshVertex(Form1 editorForm)
        {
            m_editorForm = editorForm;
        }

        public override void run()
        {
            m_editorForm.createNavMeshVertex();
        }

        public override bool isComplete()
        {
            return true;
        }
    }
}
