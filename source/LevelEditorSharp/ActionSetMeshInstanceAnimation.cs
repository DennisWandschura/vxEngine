using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionSetMeshInstanceAnimation : Action
    {
        ulong m_instanceSid;
        ulong m_oldAnimSid;
        ulong m_newAnimSid;

        public ActionSetMeshInstanceAnimation(ulong sid, ulong oldAnimSid, ulong newAnimSid)
        {
            m_instanceSid = sid;
            m_oldAnimSid = oldAnimSid;
            m_newAnimSid = newAnimSid;
        }

        public override void run()
        {
            NativeMethods.setMeshInstanceAnimation(m_instanceSid, m_newAnimSid);
        }

        public override void undo()
        {
            NativeMethods.setMeshInstanceAnimation(m_instanceSid, m_oldAnimSid);
        }

        public override void redo()
        {
            NativeMethods.setMeshInstanceAnimation(m_instanceSid, m_newAnimSid);
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            return new ActionSetMeshInstanceAnimation(m_instanceSid, m_oldAnimSid, m_newAnimSid);
        }

        public override string ToString()
        {
            return "ActionSetMeshInstanceAnimation";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            return root;
        }
    }
}
