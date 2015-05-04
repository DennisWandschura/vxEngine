﻿/*
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
    class DecisionEditorMouseButtonPressed : Decision
    {
        Form1 m_editorForm;
        System.Windows.Forms.MouseButtons m_button;

        public DecisionEditorMouseButtonPressed(DecisionTreeNode trueNode, DecisionTreeNode falseNode, Form1 editorForm, System.Windows.Forms.MouseButtons button)
            : base(trueNode, falseNode)
        {
            m_editorForm = editorForm;
            m_button = button;
        }

        public override DecisionTreeNode getBranch()
        {
            if (m_editorForm.isMouseDown() &&
                m_editorForm.getLastClickedMouseButton() == m_button)
                return m_trueNode;
            else
                return m_falseNode;
        }
    }
}
