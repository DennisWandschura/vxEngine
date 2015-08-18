using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LevelEditor
{
    public partial class FileBrowser : Form
    {
        EditorForm m_editorForm;
        DirectoryInfo m_currentDirectoryInfo;

        private void setDataFunctionDefault()
        {

        }

        private void setDataFbx()
        {
            var physxType = comboBoxMeshPhysxType.SelectedIndex;
        }

        public FileBrowser(EditorForm editorForm)
        {
            InitializeComponent();

            m_editorForm = editorForm;

            var currentDirectoryString = Directory.GetCurrentDirectory();

            textBoxPath.Text = currentDirectoryString;

            m_currentDirectoryInfo = new DirectoryInfo(currentDirectoryString);

            comboBoxFileTypes.Items.Add(new ItemFileExtension("Animation File", ".animation", setDataFunctionDefault));
            comboBoxFileTypes.Items.Add(new ItemFileExtension("Mesh File", ".mesh", setDataFunctionDefault));
            comboBoxFileTypes.Items.Add(new ItemFileExtension("FBX File", ".fbx", setDataFbx));
            comboBoxFileTypes.Items.Add(new ItemFileExtension("Material File", ".material", setDataFunctionDefault));
            comboBoxFileTypes.SelectedIndex = 0;

            comboBoxMeshPhysxType.Items.Add("Triangle Mesh");
            comboBoxMeshPhysxType.Items.Add("Convex Mesh");
            comboBoxMeshPhysxType.SelectedIndex = 0;

            flowLayoutPanelMesh.Hide();
        }

        void updateTreeView()
        {
            treeViewFiles.Nodes.Clear();

            addFolders(m_currentDirectoryInfo);
            addFiles(m_currentDirectoryInfo);

            System.GC.Collect();
        }

        void addFolders(DirectoryInfo directory)
        {
            try
            {
                var directories = directory.GetDirectories();

                foreach (var dir in directories)
                {
                    DirectoryTreeNode node = new DirectoryTreeNode(dir.Name, dir, true);

                    node.ImageIndex = 0;
                    treeViewFiles.Nodes.Add(node);
                }
            }
            catch (UnauthorizedAccessException)
            {

            }
        }

        void addFiles(DirectoryInfo directory)
        {
            var item = (ItemFileExtension)comboBoxFileTypes.SelectedItem;
            var selectedExtension = item.getExtension();

            try
            {
                var files = directory.GetFiles();
                foreach (var file in files)
                {
                    var fileExtension = file.Extension;

                    if (selectedExtension == fileExtension)
                    {
                        DirectoryTreeNode node = new DirectoryTreeNode(file.Name, null, false);
                        node.ImageIndex = 1;

                        treeViewFiles.Nodes.Add(node);
                    }
                }
            }
            catch
            {

            }
        }

        void addFilesNoFilter(DirectoryInfo directory)
        {
            try
            {
                var files = directory.GetFiles();
                foreach (var file in files)
                {
                    var fileExtension = file.Extension;

                    DirectoryTreeNode node = new DirectoryTreeNode(file.Name, null, false);
                    node.ImageIndex = 1;

                    treeViewFiles.Nodes.Add(node);
                }
            }
            catch
            {

            }
        }

        private void treeViewFiles_AfterSelect(object sender, TreeViewEventArgs e)
        {
            DirectoryTreeNode node = (DirectoryTreeNode)e.Node;

            if (node.isFolder())
            {
                node.SelectedImageIndex = 0;
            }
            else
            {
                node.SelectedImageIndex = 1;
                var fileName = node.Text;
                textBoxSelectedFile.Text = fileName;
            }
        }

        private void buttonParentFolder_Click(object sender, EventArgs e)
        {
            m_currentDirectoryInfo = m_currentDirectoryInfo.Parent;

            textBoxPath.Text = m_currentDirectoryInfo.FullName;
            updateTreeView();
        }

        private void treeViewFiles_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            //CustomNode node = (CustomNode)sender;
            DirectoryTreeNode node = (DirectoryTreeNode)treeViewFiles.SelectedNode;

            if (node.isFolder())
            {
                m_currentDirectoryInfo = node.getDirectoryInfo();
                textBoxPath.Text = m_currentDirectoryInfo.FullName;
                node.SelectedImageIndex = 0;
                updateTreeView();

            }
            else
            {
                node.SelectedImageIndex = 1;
            }
        }

        private void comboBoxFileTypes_SelectedIndexChanged(object sender, EventArgs e)
        {
            var selectedType = (ItemFileExtension)comboBoxFileTypes.SelectedItem;
            var selectedExtension = selectedType.getExtension();
            if (selectedExtension == ".fbx")
            {
                flowLayoutPanelMesh.Show();
            }
            else
            {
                flowLayoutPanelMesh.Hide();
            }

            updateTreeView();
        }

        private void buttonImport_Click(object sender, EventArgs e)
        {
            var fileName = textBoxSelectedFile.Text;

            var selectedType = (ItemFileExtension)comboBoxFileTypes.SelectedItem;
            var selectedExtension = selectedType.getExtension();
            var setDataFunction = selectedType.getFunction();

            setDataFunction();
            m_editorForm.importFile(fileName, selectedExtension);

            this.Close();
            //var fullPath = m_currentDirectoryInfo.FullName + "\\";

            //Console.WriteLine(fullPath + fileName);
        }
    }
}
