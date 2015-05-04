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
using System.Windows.Forms;

namespace LevelEditor
{
    public class MessageHandler : IMessageFilter
    {
        const int WM_KEYDOWN = 0x0100;
        const int WM_KEYUP = 0x0101;

        const int VK_NUMPAD4 = 0x64;

        IntPtr m_formHandle;
        IntPtr m_displayPanelHandle;
        Form1 m_parent;

        public MessageHandler(Form1 editorForm, IntPtr formHandle, IntPtr panelHandle)
        {
            m_formHandle = formHandle;
            m_displayPanelHandle = panelHandle;
            m_parent = editorForm;
        }

        void handleKey(int wParam)
        {
            if(wParam == VK_NUMPAD4 )
            {
                MessageBox.Show("ee");
            }
        }

        public bool PreFilterMessage(ref Message m)
        {
            /*if (m.HWnd == m_formHandle)
            {
                switch (m.Msg)
                {
                    case WM_KEYDOWN:
                        {
                            MessageBox.Show("down");
                            handleKey(m.WParam.ToInt32());
                            return true;
                        };
                    default: 
                        break;
                }
            }*/

            return false;
        }

        public void Application_Idle(object sender, EventArgs e)
        {
            try
            {
               NativeMethods.frame();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
                throw;
            }
        }
    }
}
