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
    class State
    {
        List<Action> m_actions;
        List<Action> m_entryActions;
        List<Action> m_exitActions;
        List<TransitionBase> m_transitions;

        public State()
        {
            m_actions = new List<Action>();
            m_entryActions = new List<Action>();
            m_exitActions = new List<Action>();
            m_transitions = new List<TransitionBase>();
        }

        public void addAction(Action action)
        {
            m_actions.Add(action);
        }

        public void addExitAction(Action action)
        {
            m_exitActions.Add(action);
        }

        public void addEntryAction(Action action)
        {
            m_entryActions.Add(action);
        }

        public void addTransition(TransitionBase t)
        {
            m_transitions.Add(t);
        }

        public List<Action> getActions()
        {
            return m_actions;
        }

        public List<Action> getExitActions()
        {
            return m_exitActions;
        }

        public List<Action> getEntryActions()
        {
            return m_entryActions;
        }

        public List<TransitionBase> getTransitions()
        {
            return m_transitions;
        }
    }
}
