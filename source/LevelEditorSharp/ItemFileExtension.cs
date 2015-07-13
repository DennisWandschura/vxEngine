using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ItemFileExtension
    {
        string m_text;
        string m_extension;

        public ItemFileExtension(string text, string extension)
        {
            m_text = text;
            m_extension = extension;
        }

        public override string ToString()
        {
            return m_text;
        }

        public string getExtension()
        {
            return m_extension;
        }
    }
}
