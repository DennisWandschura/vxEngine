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
    class StateMachine
    {
        State m_currentState;
        List<State> m_states;

        public StateMachine()
        {
            m_currentState = null;
            m_states = new List<State>();
        }

        public void setCurrentState(State state)
        {
            m_currentState = state;
        }

        public void addState(State state)
        {
            m_states.Add(state);
        }

        public List<Action> update()
        {
            List<TransitionBase> transitions = m_currentState.getTransitions();

            TransitionBase triggeredTransition = null;
            foreach (var transition in transitions)
            {
                if(transition.isTriggered())
                {
                    triggeredTransition = transition;
                    break;
                }
            }

            List<Action> actions = new List<Action>();
            if(triggeredTransition != null)
            {
                Console.WriteLine(triggeredTransition.getDebugText());

                var exitActions = m_currentState.getExitActions();
                List<Action> transitionActions = triggeredTransition.getActions();
                State targetState = triggeredTransition.getTargetState();
                var entryActions = targetState.getEntryActions();

                actions.AddRange(exitActions);
                actions.AddRange(transitionActions);
                actions.AddRange(entryActions);

                m_currentState = targetState;
            }

            List<Action> stateActions = m_currentState.getActions();
            actions.AddRange(stateActions);

            return actions;
        }
    }
}
