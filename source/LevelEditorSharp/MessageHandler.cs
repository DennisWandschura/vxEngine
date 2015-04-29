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
