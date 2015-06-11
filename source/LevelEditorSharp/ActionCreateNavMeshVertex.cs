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
    class ActionCreateNavMeshVertex : Action
    {
        Form1 m_editorForm;
        Float3 m_vertexPosition;
        int m_mouseX, m_mouseY;

        public ActionCreateNavMeshVertex(Form1 editorForm)
        {
            m_editorForm = editorForm;
            m_vertexPosition = new Float3();
            m_mouseX = 0;
            m_mouseY = 0;
        }

        public override void run()
        {
            m_mouseX = m_editorForm.getMouseX();
            m_mouseY = m_editorForm.getMouseY();

            NativeMethods.addNavMeshVertex(m_mouseX, m_mouseY, ref m_vertexPosition);
        }

        public override void undo()
        {
            Console.WriteLine("ActionCreateNavMeshVertex undo: {0:G} {1:G}", m_mouseX, m_mouseY);
            NativeMethods.removeNavMeshVertex(ref m_vertexPosition);
        }

        public override void redo()
        {
            Console.WriteLine("ActionCreateNavMeshVertex redo: {0:G} {1:G}", m_mouseX, m_mouseY);
            NativeMethods.addNavMeshVertex(m_mouseX, m_mouseY, ref m_vertexPosition);
        }

        public override bool isComplete()
        {
            return true;
        }

        public override Action clone()
        {
            var action = new ActionCreateNavMeshVertex(m_editorForm);
            action.m_vertexPosition.x = m_vertexPosition.x;
            action.m_vertexPosition.y = m_vertexPosition.y;
            action.m_vertexPosition.z = m_vertexPosition.z;
            action.m_mouseX = m_mouseX;
            action.m_mouseY = m_mouseY;

            Console.WriteLine("ActionCreateNavMeshVertex clone: {0:G} {1:G}", m_mouseX, m_mouseY);

            return action;
        }

        public override string ToString()
        {
            return "ActionCreateNavMeshVertex";
        }

        public override ActionNode toNode()
        {
            ActionNode root = new ActionNode(this.ToString());
            root.Nodes.Add("position: " + m_vertexPosition.x + ", " + m_vertexPosition.y + ", " + m_vertexPosition.z);
            return root;
        }
    }
}
