using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionSetMeshInstanceRotation : Action
    {
        ulong m_sid;
        Float3 m_oldRotation;
        Float3 m_newRotation;

        public ActionSetMeshInstanceRotation(ulong sid, Float3 newRotation)
        {
            m_sid = sid;
            m_oldRotation = new Float3(0, 0, 0);
            m_newRotation = newRotation;
        }

        public override void run()
        {
            NativeMethods.getMeshInstanceRotation(m_sid, ref m_oldRotation);
            NativeMethods.setMeshInstanceRotation(m_sid, ref m_newRotation);
        }

        public override void undo()
        {
            NativeMethods.setMeshInstanceRotation(m_sid, ref m_oldRotation);
        }

        public override void redo()
        {
            NativeMethods.setMeshInstanceRotation(m_sid, ref m_newRotation);
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            var action = new ActionSetMeshInstanceRotation(m_sid, m_newRotation);
            action.m_oldRotation = m_oldRotation;

            return action;
        }

        public override string ToString()
        {
            return "ActionSetMeshInstanceRotation";
        }
    }
}
