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


            EditorForm form;
            try
            {
                form = new EditorForm();
            }
            catch(Exception e)
            {
                MessageBox.Show(e.ToString());
                return;
            }
            MessageHandler msgHandler = new MessageHandler(form, form.Handle, form.getDisplayPanelHandle());

            Application.AddMessageFilter(msgHandler);
            Application.Idle += new EventHandler(msgHandler.Application_Idle);

            System.Timers.Timer autosaveTimer = new System.Timers.Timer();
            autosaveTimer.AutoReset = true;
            autosaveTimer.Interval = 1000  * 60 * 2;
            autosaveTimer.Elapsed += new System.Timers.ElapsedEventHandler(msgHandler.Application_Autosave);

            autosaveTimer.Enabled = true;

            Application.Run(form);
        }
    }
}
