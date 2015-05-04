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
    class DecisionTreeTransition : TransitionBase
    {
        TargetState m_targetState;
        DecisionTreeNode m_root;
        string m_debugText;

        public DecisionTreeTransition(DecisionTreeNode root, TargetState targetState, string debugText)
        {
            m_targetState = targetState;
            m_root = root;
            m_debugText = debugText;
        }

        TargetState makeDecision(DecisionTreeNode node)
        {
            TargetState target = null;
            if(node != null)
            {
               target = node.makeDecision();
            }

            return target;
        }

        public override bool isTriggered()
        {
            m_targetState = makeDecision(m_root);

            return (m_targetState != null);
        }

        public override State getTargetState()
        {
            return m_targetState.getTargetState();
        }

        public override List<Action> getActions()
        {
            List<Action> actions = null;

            if(m_targetState != null)
            {
                actions = m_targetState.getActions();
            }

            return actions;
        }

        public override string getDebugText()
        {
            return m_debugText;
        }
    }
}
