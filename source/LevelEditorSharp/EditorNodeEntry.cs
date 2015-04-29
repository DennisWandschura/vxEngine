using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LevelEditor
{
    class EditorNodeEntry : TreeNode
    {
        public UInt64 sid;
       public  UInt32 type;

        public EditorNodeEntry(UInt64 sid, UInt32 type, string text)
            : base(text)
        {
            this.sid = sid;
            this.type = type;
        }
    }
}
