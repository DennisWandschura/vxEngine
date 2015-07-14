namespace LevelEditor
{
    partial class FileBrowser
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FileBrowser));
            this.treeViewFiles = new System.Windows.Forms.TreeView();
            this.imageList1 = new System.Windows.Forms.ImageList(this.components);
            this.textBoxPath = new System.Windows.Forms.TextBox();
            this.textBoxSelectedFile = new System.Windows.Forms.TextBox();
            this.comboBoxFileTypes = new System.Windows.Forms.ComboBox();
            this.buttonParentFolder = new System.Windows.Forms.Button();
            this.buttonImport = new System.Windows.Forms.Button();
            this.flowLayoutPanelMesh = new System.Windows.Forms.FlowLayoutPanel();
            this.comboBoxMeshPhysxType = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.flowLayoutPanelMesh.SuspendLayout();
            this.SuspendLayout();
            // 
            // treeViewFiles
            // 
            this.treeViewFiles.ImageIndex = 0;
            this.treeViewFiles.ImageList = this.imageList1;
            this.treeViewFiles.Location = new System.Drawing.Point(76, 76);
            this.treeViewFiles.Name = "treeViewFiles";
            this.treeViewFiles.SelectedImageIndex = 0;
            this.treeViewFiles.Size = new System.Drawing.Size(419, 293);
            this.treeViewFiles.TabIndex = 0;
            this.treeViewFiles.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeViewFiles_AfterSelect);
            this.treeViewFiles.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.treeViewFiles_MouseDoubleClick);
            // 
            // imageList1
            // 
            this.imageList1.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList1.ImageStream")));
            this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList1.Images.SetKeyName(0, "Folder_6222.png");
            this.imageList1.Images.SetKeyName(1, "Image_24x.png");
            // 
            // textBoxPath
            // 
            this.textBoxPath.Location = new System.Drawing.Point(76, 28);
            this.textBoxPath.Name = "textBoxPath";
            this.textBoxPath.Size = new System.Drawing.Size(406, 20);
            this.textBoxPath.TabIndex = 1;
            // 
            // textBoxSelectedFile
            // 
            this.textBoxSelectedFile.Location = new System.Drawing.Point(76, 410);
            this.textBoxSelectedFile.Name = "textBoxSelectedFile";
            this.textBoxSelectedFile.Size = new System.Drawing.Size(358, 20);
            this.textBoxSelectedFile.TabIndex = 2;
            // 
            // comboBoxFileTypes
            // 
            this.comboBoxFileTypes.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxFileTypes.FormattingEnabled = true;
            this.comboBoxFileTypes.Location = new System.Drawing.Point(76, 480);
            this.comboBoxFileTypes.Name = "comboBoxFileTypes";
            this.comboBoxFileTypes.Size = new System.Drawing.Size(121, 21);
            this.comboBoxFileTypes.TabIndex = 3;
            this.comboBoxFileTypes.SelectedIndexChanged += new System.EventHandler(this.comboBoxFileTypes_SelectedIndexChanged);
            // 
            // buttonParentFolder
            // 
            this.buttonParentFolder.Location = new System.Drawing.Point(529, 24);
            this.buttonParentFolder.Name = "buttonParentFolder";
            this.buttonParentFolder.Size = new System.Drawing.Size(75, 23);
            this.buttonParentFolder.TabIndex = 4;
            this.buttonParentFolder.Text = "Parent";
            this.buttonParentFolder.UseVisualStyleBackColor = true;
            this.buttonParentFolder.Click += new System.EventHandler(this.buttonParentFolder_Click);
            // 
            // buttonImport
            // 
            this.buttonImport.Location = new System.Drawing.Point(529, 480);
            this.buttonImport.Name = "buttonImport";
            this.buttonImport.Size = new System.Drawing.Size(75, 23);
            this.buttonImport.TabIndex = 5;
            this.buttonImport.Text = "Import";
            this.buttonImport.UseVisualStyleBackColor = true;
            this.buttonImport.Click += new System.EventHandler(this.buttonImport_Click);
            // 
            // flowLayoutPanelMesh
            // 
            this.flowLayoutPanelMesh.Controls.Add(this.label1);
            this.flowLayoutPanelMesh.Controls.Add(this.comboBoxMeshPhysxType);
            this.flowLayoutPanelMesh.Location = new System.Drawing.Point(657, 142);
            this.flowLayoutPanelMesh.Name = "flowLayoutPanelMesh";
            this.flowLayoutPanelMesh.Size = new System.Drawing.Size(266, 69);
            this.flowLayoutPanelMesh.TabIndex = 6;
            // 
            // comboBoxMeshPhysxType
            // 
            this.comboBoxMeshPhysxType.FormattingEnabled = true;
            this.comboBoxMeshPhysxType.Location = new System.Drawing.Point(103, 3);
            this.comboBoxMeshPhysxType.Name = "comboBoxMeshPhysxType";
            this.comboBoxMeshPhysxType.Size = new System.Drawing.Size(121, 21);
            this.comboBoxMeshPhysxType.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(94, 27);
            this.label1.TabIndex = 1;
            this.label1.Text = "Physx Mesh Type:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // FileBrowser
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(989, 548);
            this.Controls.Add(this.flowLayoutPanelMesh);
            this.Controls.Add(this.buttonImport);
            this.Controls.Add(this.buttonParentFolder);
            this.Controls.Add(this.comboBoxFileTypes);
            this.Controls.Add(this.textBoxSelectedFile);
            this.Controls.Add(this.textBoxPath);
            this.Controls.Add(this.treeViewFiles);
            this.Name = "FileBrowser";
            this.Text = "ChildWindow";
            this.flowLayoutPanelMesh.ResumeLayout(false);
            this.flowLayoutPanelMesh.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TreeView treeViewFiles;
        private System.Windows.Forms.TextBox textBoxPath;
        private System.Windows.Forms.TextBox textBoxSelectedFile;
        private System.Windows.Forms.ComboBox comboBoxFileTypes;
        private System.Windows.Forms.Button buttonParentFolder;
        private System.Windows.Forms.ImageList imageList1;
        private System.Windows.Forms.Button buttonImport;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanelMesh;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comboBoxMeshPhysxType;

    }
}