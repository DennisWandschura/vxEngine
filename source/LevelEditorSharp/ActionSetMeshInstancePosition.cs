using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionSetMeshInstancePosition : Action
    {
        ulong m_sid;
        Float3 m_oldPosition;
        Float3 m_newPosition;

        public ActionSetMeshInstancePosition(ulong sid, Float3 newPosition)
        {
            m_sid = sid;
            m_oldPosition = new Float3(0, 0, 0);
            m_newPosition = newPosition;
        }

        public override void run()
        {
            NativeMethods.getMeshInstancePosition(m_sid, ref m_oldPosition);
            NativeMethods.setMeshInstancePosition(m_sid, ref m_newPosition);
        }

        public override void undo()
        {
            NativeMethods.setMeshInstancePosition(m_sid, ref m_oldPosition);
        }

        public override void redo()
        {
            NativeMethods.setMeshInstancePosition(m_sid, ref m_newPosition);
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            var action = new ActionSetMeshInstancePosition(m_sid, m_newPosition);
            action.m_oldPosition = m_oldPosition;

            return action;
        }

        public override string ToString()
        {
            return "ActionSetMeshInstancePosition";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            root.Nodes.Add("old position: " + m_oldPosition.x + ", " + m_oldPosition.y + ", " + m_oldPosition.z);
            root.Nodes.Add("new position: " + m_newPosition.x + ", " + m_newPosition.y + ", " + m_newPosition.z);

            return root;
        }
    }
}
