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
    class ActionDeselectMesh : Action
    {
        Form1 m_editorForm;
        ulong m_deselectedMeshInstance;

        public ActionDeselectMesh(Form1 editorForm)
        {
            m_editorForm = editorForm;
            m_deselectedMeshInstance = 0;
        }

        public override void run()
        {
            m_deselectedMeshInstance = m_editorForm.deselectMesh();
        }

        public override void undo()
        {
            Console.WriteLine("ActionDeselectMesh undo: {0:G}", m_deselectedMeshInstance);
            m_editorForm.selectMesh(m_deselectedMeshInstance);
        }

        public override void redo()
        {
            Console.WriteLine("ActionDeselectMesh redo: {0:G}", m_deselectedMeshInstance);
            run();
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            /*if (m_deselectedMeshInstance == 0)
                return null;

            var action = new ActionDeselectMesh(m_editorForm);
            action.m_deselectedMeshInstance = m_deselectedMeshInstance;

            return action;*/
            return null;
        }

        public override string ToString()
        {
            return "ActionDeselectMesh";
        }
    }
}
