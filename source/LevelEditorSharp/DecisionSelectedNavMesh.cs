using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class DecisionSelectedNavMesh : Decision
    {
        public DecisionSelectedNavMesh(DecisionTreeNode trueNode, DecisionTreeNode falseNode)
            : base(trueNode, falseNode)
        {
        }

        public override DecisionTreeNode getBranch()
        {
            if (NativeMethods.getSelectedNavMeshCount() == 3)
            {
                return m_trueNode;
            }

            return m_falseNode;
        }
    }
}
