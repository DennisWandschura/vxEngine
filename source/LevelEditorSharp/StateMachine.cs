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
