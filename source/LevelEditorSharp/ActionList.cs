using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

namespace LevelEditor
{
    class ActionList
    {
        Action action;
        ActionList m_next;
        ActionList m_prev;
        bool isUndo;

        public ActionList(Action action, ActionList prev)
        {
            this.action = action;
            m_next = null;
            m_prev = prev;
            isUndo = false;
        }

        public void undo()
        {
            if (action != null && !isUndo)
            {
                action.undo();
                isUndo = true;
            }
        }

        public ActionList next()
        {
            return m_next;
        }

        public ActionList prev()
        {
            return m_prev;
        }

        public void redo()
        {
            if (action != null && isUndo)
            {
                action.redo();
                isUndo = false;
            }
        }

        public void setNext(ActionList next)
        {
            m_next = next;
        }
    }
}
