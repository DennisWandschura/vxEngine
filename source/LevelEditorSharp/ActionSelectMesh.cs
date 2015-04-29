using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionSelectMesh : Action
    {
        Form1 m_editorForm;

        public ActionSelectMesh(Form1 editorForm)
        {
            m_editorForm = editorForm;
        }

        public override void run()
        {
            m_editorForm.selectMesh();
        }

        public override bool isComplete()
        {
            return true;
        }
    }
}
