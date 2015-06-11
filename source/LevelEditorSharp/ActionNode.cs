using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LevelEditor
{
    class ActionNode : TreeNode
    {
        string m_text;

        public ActionNode(string text)
            : base(text)
        {
            m_text = Text;
        }

        public override string ToString()
        {
            return m_text;
        }
    }
}
