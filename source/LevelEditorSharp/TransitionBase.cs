using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    abstract class TransitionBase
    {
        public abstract bool isTriggered();

        public abstract State getTargetState();

        public abstract List<Action> getActions();

        public abstract string getDebugText();
    }
}
