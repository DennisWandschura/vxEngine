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
    class TargetState : DecisionTreeNode
    {
        List<Action> m_actions;
        State m_targetState;
        string m_debugString;

        public TargetState(State targetState, string debugText)
        {
            m_actions = new List<Action>();
            m_targetState = targetState;
            m_debugString = debugText;
        }

        public void addAction(Action a)
        {
            if(a != null)
                m_actions.Add(a);
        }

        public override TargetState makeDecision()
        {
            return this;
        }

        public List<Action> getActions()
        {
            return m_actions;
        }

        public State getTargetState()
        {
            return m_targetState;
        }

        public TargetState clone()
        {
            var state = new TargetState(m_targetState, m_debugString);

            foreach (var item in this.m_actions)
            {
                var newAction = item.clone();
                if (newAction != null)
                    state.m_actions.Add(newAction);
            }

            return state;
        }

        public string getDebugString()
        {
            return m_debugString;
        }
    }
}
