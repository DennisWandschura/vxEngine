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
    public partial class MeshInstanceDataControl : UserControl
    {
        EditorForm m_editorForm;
        EditorEntry m_animationEntryEmpty;

        public MeshInstanceDataControl(EditorForm editorForm)
        {
            InitializeComponent();

            m_editorForm = editorForm;
            m_animationEntryEmpty = new EditorEntry("", 0);

            comboBoxRigidBody.Items.Add("Static");
            comboBoxRigidBody.Items.Add("Dynamic");
        }

        public void clearMeshEntries()
        {
            comboBoxMesh.Items.Clear();
        }

        public void clearAnimationEntries()
        {
            comboBoxAnimation.Items.Clear();
            comboBoxAnimation.Items.Add(m_animationEntryEmpty);
        }

        public void clearMaterialEntries()
        {
            comboBoxMaterial.Items.Clear();
        }

        public void addMeshEntry(EditorEntry entry)
        {
            comboBoxMesh.Items.Add(entry);
        }

        public void addMaterialEntry(EditorEntry entry)
        {
            comboBoxMaterial.Items.Add(entry);
        }

        public void addAnimationEntry(EditorEntry entry)
        {
            comboBoxAnimation.Items.Add(entry);
        }

        public void setTranslation(Float3 p)
        {
            translationX.Value = (decimal)p.x;
            translationY.Value = (decimal)p.y;
            translationZ.Value = (decimal)p.z;
        }

        public void setRotation(Float3 p)
        {
            rotationX.Value = (decimal)p.x;
            rotationY.Value = (decimal)p.y;
            rotationZ.Value = (decimal)p.z;
        }

        public void setInstanceName(string meshName)
        {
            texboxName.Text = meshName;
        }

        public void setMesh(EditorEntry entry)
        {
            comboBoxMesh.SelectedItem = entry;
        }

        public void setMaterial(EditorEntry entry)
        {
            comboBoxMaterial.SelectedItem = entry;
        }

        public void setAnimation(EditorEntry entry)
        {
            if (entry == null)
            {
                comboBoxAnimation.SelectedItem = m_animationEntryEmpty;
            }
            else
            {
                comboBoxAnimation.SelectedItem = entry;
            }
        }
         
        public Float3 getTranslation()
        {
            Float3 translation = new Float3();
            translation.x = (float)translationX.Value;
            translation.y = (float)translationY.Value;
            translation.z = (float)translationZ.Value;

            return translation;
        }

        public Float3 getRotation()
        {
            Float3 rotation = new Float3();
            rotation.x = (float)rotationX.Value;
            rotation.y = (float)rotationY.Value;
            rotation.z = (float)rotationZ.Value;

            return rotation;
        }

        EditorEntry getSelectedMaterial()
        {
            return (EditorEntry)comboBoxMaterial.SelectedItem;
        }

        private void setEditorFormPosition()
        {
            var translation = getTranslation();
            m_editorForm.setMeshInstancePosition(translation);
        }

        private void translationZ_ValueChanged(object sender, EventArgs e)
        {
            setEditorFormPosition();
        }

        private void translationY_ValueChanged(object sender, EventArgs e)
        {
            setEditorFormPosition();
        }

        private void translationX_ValueChanged(object sender, EventArgs e)
        {
            setEditorFormPosition();
        }

        private void setEditorRotation()
        {
            Float3 rotation = getRotation();

            m_editorForm.setMeshInstanceRotation(rotation);
        }

        private void rotationX_ValueChanged(object sender, EventArgs e)
        {
            setEditorRotation();
        }

        private void rotationY_ValueChanged(object sender, EventArgs e)
        {
            setEditorRotation();
        }

        private void rotationZ_ValueChanged(object sender, EventArgs e)
        {
            setEditorRotation();
        }

        private void comboBoxMesh_SelectedIndexChanged(object sender, EventArgs e)
        {
            EditorEntry entry = (EditorEntry)comboBoxMesh.SelectedItem;

            var newMeshSid = entry.m_sid;

            var meshInstanceSid = NativeMethods.getSelectedMeshInstanceSid();

            m_editorForm.setMeshInstanceMesh(meshInstanceSid, newMeshSid);
        }

        private void comboBoxMaterial_SelectedIndexChanged(object sender, EventArgs e)
        {
            EditorEntry item = (EditorEntry)comboBoxMaterial.SelectedItem;

           m_editorForm. setMeshInstanceMaterial(item.m_sid);
        }

        private void comboBoxAnimation_SelectedIndexChanged(object sender, EventArgs e)
        {
            EditorEntry entry = (EditorEntry)comboBoxAnimation.SelectedItem;
           m_editorForm. setMeshInstanceAnimation(entry.m_sid);
        }

        private void comboBoxRigidBody_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void texboxName_TextChanged(object sender, EventArgs e)
        {
           /* var oldSid = NativeMethods.getSelectedMeshInstanceSid();

            if (oldSid != 0)
            {
                var oldName = NativeMethods.getSelectedMeshInstanceName();

                var newName = textBoxMeshName.Text;
                var newSid = NativeMethods.getSid(newName);

                if (newSid != oldSid)
                {
                    //ActionRenameSelectedMeshInstance action = new ActionRenameSelectedMeshInstance(oldName, oldSid, newName, newSid, this);

                    //runAction(action);
                    throw new Exception("not properly implemented yet");
                }
            }*/
        }
    }
}
