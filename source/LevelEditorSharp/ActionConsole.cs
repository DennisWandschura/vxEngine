using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ActionConsole : Action
    {
        public override void run()
        {
            Console.WriteLine("Test\n");
        }

        public override bool isComplete()
        {
            return true;
        }
    }
}
