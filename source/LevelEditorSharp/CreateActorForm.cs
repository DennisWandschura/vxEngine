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
    public partial class CreateActorForm : Form
    {
        EditorForm m_editor;

        public CreateActorForm(EditorForm editor)
        {
            InitializeComponent();

            m_editor = editor;
        }

        public void addMeshEntry(EditorEntry entry)
        {
            comboBoxMesh.Items.Add(entry);
        }

        public void addMaterialEntry(EditorEntry entry)
        {
            comboBoxMaterial.Items.Add(entry);
        }

        public void clearMaterialEntries()
        {
            comboBoxMaterial.Items.Clear();
        }
        public void clearMeshEntries()
        {
            comboBoxMesh.Items.Clear();
        }

        private void buttonCreate_Click(object sender, EventArgs e)
        {
            if (comboBoxMesh.Items.Count == 0 || comboBoxMaterial.Items.Count == 0)
                return;

            string name = textBoxActorName.Text;
            var entryMesh = (EditorEntry)comboBoxMesh.SelectedItem;
            var entryMaterial = (EditorEntry)comboBoxMaterial.SelectedItem;

            ulong actorSid = NativeMethods.createActor(name, entryMesh.m_sid, entryMaterial.m_sid);
            m_editor.addActor(actorSid);

            this.Close();
        }
    }
}
