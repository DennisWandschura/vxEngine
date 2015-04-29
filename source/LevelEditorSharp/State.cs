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
