using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionCreateMeshInstance : Action
    {
        EditorForm m_editorForm;
        ulong m_instanceSid;

        public ActionCreateMeshInstance(EditorForm form)
        {
            m_editorForm = form;
            m_instanceSid = 0;
        }

        public override void run()
        {
            m_instanceSid = NativeMethods.createMeshInstance();
            m_editorForm.addMeshInstance(m_instanceSid);
        }

        public override void undo()
        {
            NativeMethods.removeMeshInstance(m_instanceSid);
            m_editorForm.removeMeshInstance(m_instanceSid);
        }

        public override void redo()
        {
            m_instanceSid = NativeMethods.createMeshInstance();
            m_editorForm.addMeshInstance(m_instanceSid);
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            if(m_instanceSid == 0)
              return null;

            var action = new ActionCreateMeshInstance(m_editorForm);
            action.m_instanceSid = m_instanceSid;

            return action;
        }

        public override string ToString()
        {
            return "ActionCreateMeshInstance";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            root.Nodes.Add("Sid: " + m_instanceSid);

            return root;
        }
    }
}
