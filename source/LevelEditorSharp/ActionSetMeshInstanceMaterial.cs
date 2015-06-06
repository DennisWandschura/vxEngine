using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionSetMeshInstanceMaterial : Action
    {
        ulong m_instanceSid;
        ulong m_oldMaterialSid;
        ulong m_newMaterialSid;

        public ActionSetMeshInstanceMaterial(ulong sid, ulong oldMaterialSid, ulong newMaterialSid)
        {
            m_instanceSid = sid;
            m_oldMaterialSid = oldMaterialSid;
            m_newMaterialSid = newMaterialSid;
        }

        public override void run()
        {
            NativeMethods.setMeshInstanceMaterial(m_instanceSid, m_newMaterialSid);
        }

        public override void undo()
        {
            NativeMethods.setMeshInstanceMaterial(m_instanceSid, m_oldMaterialSid);
        }

        public override void redo()
        {
            NativeMethods.setMeshInstanceMaterial(m_instanceSid, m_newMaterialSid);
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            return new ActionSetMeshInstanceMaterial(m_instanceSid, m_oldMaterialSid, m_newMaterialSid);
        }

        public override string ToString()
        {
            return "ActionSetMeshInstanceMaterial";
        }
    }
}
