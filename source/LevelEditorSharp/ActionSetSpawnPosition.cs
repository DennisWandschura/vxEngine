using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionSetSpawnPosition : Action
    {
        uint m_spawnIndex;
        Float3 m_position;
        Float3 m_oldPosition;

        public ActionSetSpawnPosition(uint spawnIndex, Float3 position, Float3 oldPosition)
        {
            m_spawnIndex = spawnIndex;
            m_position = position;
            m_oldPosition = oldPosition;
        }

        public override Action clone()
        {
            return new ActionSetSpawnPosition(m_spawnIndex, m_position, m_oldPosition);;
        }

        public override bool isComplete()
        {
            return true;
        }

        public override void redo()
        {
            NativeMethods.setSpawnPosition(m_spawnIndex, ref m_position);
        }

        public override void run()
        {
            NativeMethods.setSpawnPosition(m_spawnIndex, ref m_position);
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            return root;
        }

        public override string ToString()
        {
            return "ActionSetSpawnPosition";
        }

        public override void undo()
        {
            NativeMethods.setSpawnPosition(m_spawnIndex, ref m_oldPosition);
        }
    }
}
