using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LevelEditor
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            Form1 form;
            try
            {
                form = new Form1();
            }
            catch(Exception e)
            {
                MessageBox.Show(e.ToString());
                return;
            }
            MessageHandler msgHandler = new MessageHandler(form, form.Handle, form.getDisplayPanelHandle());

            Application.AddMessageFilter(msgHandler);
            Application.Idle += new EventHandler(msgHandler.Application_Idle);

            Application.Run(form);
        }
    }
}
