using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LevelEditor
{
    public partial class CreateMeshInstanceForm : Form
    {
        Form1 m_parent;

        public CreateMeshInstanceForm(Form1 parent, TreeNode meshNodes, TreeNode materialNodes)
        {
            InitializeComponent();

            m_parent = parent;

            foreach (TreeNode it in meshNodes.Nodes)
            {
                comboBox_meshes.Items.Add(it.Text);
            }
            comboBox_meshes.SelectedIndex = 0;

            foreach (TreeNode it in materialNodes.Nodes)
            {
                comboBox_materials.Items.Add(it.Text);
            }
            comboBox_materials.SelectedIndex = 0;
        }

        private void button_create_Click(object sender, EventArgs e)
        {
            Float3 translation;
            translation.x = (float)numericUpDown_translation_x.Value;
            translation.y = (float)numericUpDown_translation_y.Value;
            translation.z = (float)numericUpDown_translation_z.Value;

            Float3 rotation;
            rotation.x = 0;
            rotation.y = 0;
            rotation.z = 0;

            float scaling = 1.0f;

            try
            {
                UInt64 meshSid = NativeMethods.getSid(comboBox_meshes.SelectedItem.ToString());
                UInt64 materialSid = NativeMethods.getSid(comboBox_materials.SelectedItem.ToString());
                UInt64 instanceSid = NativeMethods.getSid(textBox_name.Text);

                /*if (NativeMethods.addMeshInstance(instanceSid, meshSid, materialSid, translation, rotation, scaling) != 0)
                {
                    m_parent.addMeshInstance(instanceSid, textBox_name.Text);
                }*/
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }

            this.Close();
        }

        private void button_cancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
