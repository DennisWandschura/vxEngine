using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevelEditor
{
    class ItemFileExtension
    {
        public delegate void SetUserDataFunction();

        string m_text;
        string m_extension;
        SetUserDataFunction m_function;

        public ItemFileExtension(string text, string extension, SetUserDataFunction function)
        {
            m_text = text;
            m_extension = extension;
            m_function = function;
        }

        public override string ToString()
        {
            return m_text;
        }

        public string getExtension()
        {
            return m_extension;
        }

        public SetUserDataFunction getFunction()
        {
            return m_function;
        }
    }
}
