using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionSetMeshInstanceMesh : Action
    {
        ulong m_instanceSid;
        ulong m_oldMeshSid;
        ulong m_newMeshSid;

        public ActionSetMeshInstanceMesh(ulong sid, ulong oldMeshSid, ulong newMeshSid)
        {
            m_instanceSid = sid;
            m_oldMeshSid = oldMeshSid;
            m_newMeshSid = newMeshSid;
        }

        public override void run()
        {
            NativeMethods.setMeshInstanceMeshSid(m_instanceSid, m_newMeshSid);
        }

        public override void undo()
        {
            NativeMethods.setMeshInstanceMeshSid(m_instanceSid, m_oldMeshSid);
        }

        public override void redo()
        {
            NativeMethods.setMeshInstanceMeshSid(m_instanceSid, m_newMeshSid);
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            return new ActionSetMeshInstanceMesh(m_instanceSid, m_oldMeshSid, m_newMeshSid);
        }

        public override string ToString()
        {
            return "ActionSetMeshInstanceMesh";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            return root;
        }
    }
}
