using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionRemoveNavMeshVertex : Action
    {
        Float3 m_position;

        public ActionRemoveNavMeshVertex()
        {
            m_position = new Float3(0, 0, 0);
        }

        public override void run()
        {
            NativeMethods.getSelectNavMeshVertexPosition(ref m_position);
            NativeMethods.removeNavMeshVertex(ref m_position);
        }

        public override void undo()
        {

        }

        public override void redo()
        {
            NativeMethods.removeNavMeshVertex(ref m_position);
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            var action = new ActionRemoveNavMeshVertex();
            action.m_position.x = this.m_position.x;
            action.m_position.y = this.m_position.y;
            action.m_position.z = this.m_position.z;

            return action;
        }

        public override string ToString()
        {
            return "ActionRemoveNavMeshVertex";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            return root;
        }
    }
}
