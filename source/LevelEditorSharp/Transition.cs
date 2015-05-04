/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class Transition : TransitionBase
    {
        Condition m_condition;
        State m_targetState;
        List<Action> m_actions;
        string m_debugText;

        public Transition(Condition condition, State targetState, string debugText)
        {
            m_condition = condition;
            m_targetState = targetState;
            m_actions = new List<Action>();
            m_debugText = debugText;
        }

        public void addAction(Action a)
        {
            m_actions.Add(a);
        }

        public override bool isTriggered()
        {
            return m_condition.test();
        }

        public override State getTargetState()
        {
            return m_targetState;
        }

        public override List<Action> getActions()
        {
            return m_actions;
        }

        public override string getDebugText()
        {
            return m_debugText;
        }
    }
}
