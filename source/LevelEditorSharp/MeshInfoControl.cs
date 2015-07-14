using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LevelEditor
{
    public partial class MeshInfoControl : UserControl
    {
        EditorForm m_editorForm;

        public MeshInfoControl(EditorForm editorForm)
        {
            m_editorForm = editorForm;

            InitializeComponent();

            comboBoxPhysx.Items.Add("Triangle Mesh");
            comboBoxPhysx.Items.Add("Convex Mesh");
            comboBoxPhysx.Items.Add("Invalid");
        }

        public void setPhysxType(uint type)
        {
            if(type == 0xffffffff)
            {
                comboBoxPhysx.SelectedIndex = 2;
            }
            else
            {
                comboBoxPhysx.SelectedIndex = (int)type;
            }
        }

        public void setName(string name)
        {
            textBoxName.Text = name;
        }

        private void comboBoxPhysx_SelectedIndexChanged(object sender, EventArgs e)
        {
            var type = (uint)comboBoxPhysx.SelectedIndex;
            m_editorForm.setMeshPhysxType(type);

        }
    }
}
