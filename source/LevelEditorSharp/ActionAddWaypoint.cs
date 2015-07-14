using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionAddWaypoint : Action
    {
        EditorForm m_editorForm;
        Float3 m_position;
        bool m_added;

        public ActionAddWaypoint(EditorForm editorform)
        {
            m_editorForm = editorform;
            m_position = new Float3(0, 0, 0);
            m_added = false;
        }

        public override void run()
        {
            var x = m_editorForm.getMouseX();
            var y = m_editorForm.getMouseY();

            m_added = NativeMethods.addWaypoint(x, y, ref m_position);
        }

        public override void undo()
        {
            if (m_added)
            {
                NativeMethods.removeWaypoint(ref m_position);
            }
        }

        public override void redo()
        {
            if (m_added)
            {
                NativeMethods.addWaypointPosition(ref m_position);
            }
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            if (m_added == false)
                return null;

            var action = new ActionAddWaypoint(m_editorForm);
            action.m_position.x = m_position.x;
            action.m_position.y = m_position.y;
            action.m_position.z = m_position.z;
            action.m_added = m_added;

            return action;
        }

        public override string ToString()
        {
            return "ActionAddWaypoint";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            root.Nodes.Add("position: " + m_position.x + ", " + m_position.y + ", " + m_position.z);

            return root;
        }
    }
}
