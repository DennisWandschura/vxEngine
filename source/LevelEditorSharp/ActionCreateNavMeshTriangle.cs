using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionCreateNavMeshTriangle : Action
    {
        public override void run()
        {
            NativeMethods.createNavMeshTriangleFromSelectedVertices();
        }

        public override bool isComplete()
        {
            return true;
        }
    }
}
