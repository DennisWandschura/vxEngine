using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionRenameSelectedMeshInstance : Action
    {
        string m_name;
        ulong m_sid;
        string m_oldName;
        ulong m_oldSid;
        EditorForm m_editorForm;

        public ActionRenameSelectedMeshInstance(string oldName, ulong oldSid, string newName, ulong newSid, EditorForm editorForm)
        {
            m_name = newName;
            m_sid = newSid;
            m_oldName = oldName;
            m_oldSid = oldSid;
            m_editorForm = editorForm;
        }

        public override void run()
        {
            NativeMethods.setMeshInstanceName(m_oldSid, m_name);
            m_editorForm.onRenameMeshInstance(m_oldSid, m_sid, m_name);
        }

        public override void undo()
        {
            NativeMethods.setMeshInstanceName(m_sid, m_oldName);
            m_editorForm.onRenameMeshInstance(m_sid, m_oldSid, m_oldName);
        }

        public override void redo()
        {
            run();
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            return new ActionRenameSelectedMeshInstance(m_oldName, m_oldSid, m_name, m_sid, m_editorForm);
        }

        public override string ToString()
        {
            return "ActionRenameSelectedMeshInstance";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            root.Nodes.Add("old name: " + m_oldName);
            root.Nodes.Add("new name: " + m_name);
            return root;
        }
    }
}
