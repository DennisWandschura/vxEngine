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
    class ActionDecisionTree : Action
    {
        DecisionTreeNode m_root;
        TargetState m_lastState;

        public ActionDecisionTree(DecisionTreeNode root)
        {
            m_root = root;
            m_lastState = null;
        }

        public override void run()
        {
            m_lastState = m_root.makeDecision();
            if (m_lastState != null)
            {
                var actions = m_lastState.getActions();

                foreach (var action in actions)
                {
                    action.run();
                }
            }
        }

        public override void undo()
        {
            if (m_lastState != null)
            {
                var actions = m_lastState.getActions();

                foreach (var action in actions)
                {
                    action.undo();
                }
            }
        }

        public override void redo()
        {
            if (m_lastState != null)
            {
                var actions = m_lastState.getActions();

                foreach (var action in actions)
                {
                    action.redo();
                }
            }
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            if (this.m_lastState == null)
                return null;

            var newState = this.m_lastState.clone();
            if (newState.getActions().Count == 0)
                return null;

            var action = new ActionDecisionTree(m_root);
            action.m_lastState = newState;

            return action;
        }

        public override string ToString()
        {
            if (m_lastState != null)
            {
                var actions = m_lastState.getActions();
                if (actions.Count == 1)
                {
                    return actions[0].ToString();
                }
            }

            return "ActionDecisionTree";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());

            if (m_lastState != null)
            {
                var actions = m_lastState.getActions();
                foreach (var action in actions)
                {
                    root.Nodes.Add(action.toNode());
                }
            }

            return root;
        }
    }
}
