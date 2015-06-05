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
    class ActionCallFunction : Action
    {
        public delegate void ActionCallFunctionProc();

        ActionCallFunctionProc m_fun;
        ActionCallFunctionProc m_undoFun;
        bool m_clone;

        public ActionCallFunction(ActionCallFunctionProc fn, ActionCallFunctionProc undoFun, bool clone)
        {
            m_fun = fn;
            m_undoFun = undoFun;
            m_clone = clone;
        }

        public override void run()
        {
            m_fun();
        }

        public override void undo()
        {
            m_undoFun();
        }

        public override void redo()
        {
            m_fun();
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            if (!m_clone)
                return null;

            return new ActionCallFunction(m_fun, m_undoFun, m_clone);
        }

        public override string ToString()
        {
            return "ActionCallFunction";
        }
    }

    class ActionCallFunctionReturnBool : Action
    {
        public delegate bool ActionCallFunctionProc();

        ActionCallFunctionProc m_fun;
        ActionCallFunctionProc m_undoFun;

        public ActionCallFunctionReturnBool(ActionCallFunctionProc fn, ActionCallFunctionProc undoFn)
        {
            m_fun = fn;
            m_undoFun = undoFn;
        }

        public override void run()
        {
            m_fun();
        }

        public override void undo()
        {
            m_undoFun();
        }

        public override void redo()
        {
            m_fun();
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            return new ActionCallFunctionReturnBool(m_fun, m_undoFun);
        }

        public override string ToString()
        {
            return "ActionCallFunctionReturnBool";
        }
    }
}
