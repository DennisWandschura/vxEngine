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
    class ActionSelectNavMesh : Action
    {
        Form1 m_editorForm;
        int m_mouseX, m_mouseY;
        int m_hasSelected;
        Float3 m_lastPosition;

        public ActionSelectNavMesh(Form1 editorForm)
        {
            m_editorForm = editorForm;
            m_mouseX = 0;
            m_mouseY = 0;
            m_hasSelected = 0;

            m_lastPosition = new Float3(0, 0, 0);
        }

        public override void run()
        {
            m_mouseX = m_editorForm.getMouseX();
            m_mouseY = m_editorForm.getMouseY();

            m_editorForm.selectNavMeshVertex(m_mouseX, m_mouseY, out m_hasSelected, ref m_lastPosition);
        }

        public override void undo()
        {
            if (m_hasSelected != 0)
            {
                m_editorForm.selectNavMeshVertex(ref m_lastPosition);
            }
            else
            {
                m_editorForm.deselectNavMeshVertex();
            }
        }

        public override void redo()
        {
            m_editorForm.selectNavMeshVertex(m_mouseX, m_mouseY);
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            var action = new ActionSelectNavMesh(m_editorForm);
            action.m_mouseX = m_mouseX;
            action.m_mouseY = m_mouseY;
            action.m_hasSelected = m_hasSelected;
            action.m_lastPosition.x = m_lastPosition.x;
            action.m_lastPosition.y = m_lastPosition.y;
            action.m_lastPosition.z = m_lastPosition.z;

            return action;
        }
    }
}
