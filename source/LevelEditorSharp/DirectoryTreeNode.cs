using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LevelEditor
{
    class DirectoryTreeNode : TreeNode
    {
        DirectoryInfo m_dirInfo;
        bool m_isFolder;

        public DirectoryTreeNode(string text, DirectoryInfo dirInfo, bool isfolder)
            : base(text)
        {
            m_dirInfo = dirInfo;
            m_isFolder = isfolder;
        }

        ~DirectoryTreeNode()
        {
        }

        public DirectoryInfo getDirectoryInfo()
        {
            return m_dirInfo;
        }

        public bool isFolder() { return m_isFolder; }
    }
}
