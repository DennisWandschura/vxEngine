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
    public partial class ActorInfoControl : UserControl
    {
        EditorForm m_editor;

        public ActorInfoControl(EditorForm editor)
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
    }
}
