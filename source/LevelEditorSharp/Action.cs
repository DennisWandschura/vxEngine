using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    abstract class Action
    {
        public abstract void run();

        public abstract bool isComplete();
    }
}
