/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
namespace LevelEditor
{
    partial class Form1
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.importAssetToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadMeshToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveSceneToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createMeshInstanceToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createLightToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.itemShotNavmesh = new System.Windows.Forms.ToolStripMenuItem();
            this.itemInfluenceMap = new System.Windows.Forms.ToolStripMenuItem();
            this.panel_render = new System.Windows.Forms.Panel();
            this.openFileDialog_importAsset = new System.Windows.Forms.OpenFileDialog();
            this.treeView_entities = new System.Windows.Forms.TreeView();
            this.numericUpDown_translation_x = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown_translation_y = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown_translation_z = new System.Windows.Forms.NumericUpDown();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.label1 = new System.Windows.Forms.Label();
            this.flowLayoutPanel2 = new System.Windows.Forms.FlowLayoutPanel();
            this.numericUpDown_rot_z = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown_rot_y = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown_rot_x = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.flowLayoutPanel3 = new System.Windows.Forms.FlowLayoutPanel();
            this.numericUpDown_scale_z = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown_scale_y = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown_scale_x = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.groupBoxMesh = new System.Windows.Forms.GroupBox();
            this.flowLayoutPanel8 = new System.Windows.Forms.FlowLayoutPanel();
            this.flowLayoutPanel6 = new System.Windows.Forms.FlowLayoutPanel();
            this.textBoxMeshName = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.flowLayoutPanel9 = new System.Windows.Forms.FlowLayoutPanel();
            this.meshInstanceComboBoxMesh = new System.Windows.Forms.ComboBox();
            this.label8 = new System.Windows.Forms.Label();
            this.flowLayoutPanel7 = new System.Windows.Forms.FlowLayoutPanel();
            this.meshInstanceComboBoxMaterial = new System.Windows.Forms.ComboBox();
            this.label7 = new System.Windows.Forms.Label();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.comboBox_selectEditorMode = new System.Windows.Forms.ToolStripComboBox();
            this.toolStripButtonCreateLight = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonCreateMeshInstance = new System.Windows.Forms.ToolStripButton();
            this.openFileDialog1_loadScene = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog_scene = new System.Windows.Forms.SaveFileDialog();
            this.numericUpDownNavmeshPositionZ = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownNavmeshPositionY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownNavmeshPositionX = new System.Windows.Forms.NumericUpDown();
            this.flowLayoutPanel4 = new System.Windows.Forms.FlowLayoutPanel();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBoxNavMesh = new System.Windows.Forms.GroupBox();
            this.groupBoxLight = new System.Windows.Forms.GroupBox();
            this.flowLayoutPanel5 = new System.Windows.Forms.FlowLayoutPanel();
            this.label5 = new System.Windows.Forms.Label();
            this.numericUpDownLightX = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownLightY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownLightZ = new System.Windows.Forms.NumericUpDown();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.undoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.redoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_x)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_y)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_z)).BeginInit();
            this.flowLayoutPanel1.SuspendLayout();
            this.flowLayoutPanel2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_rot_z)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_rot_y)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_rot_x)).BeginInit();
            this.flowLayoutPanel3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_scale_z)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_scale_y)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_scale_x)).BeginInit();
            this.groupBoxMesh.SuspendLayout();
            this.flowLayoutPanel8.SuspendLayout();
            this.flowLayoutPanel6.SuspendLayout();
            this.flowLayoutPanel9.SuspendLayout();
            this.flowLayoutPanel7.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionX)).BeginInit();
            this.flowLayoutPanel4.SuspendLayout();
            this.groupBoxNavMesh.SuspendLayout();
            this.groupBoxLight.SuspendLayout();
            this.flowLayoutPanel5.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightZ)).BeginInit();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.editToolStripMenuItem,
            this.createToolStripMenuItem,
            this.viewToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(1904, 24);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.importAssetToolStripMenuItem,
            this.loadMeshToolStripMenuItem,
            this.saveToolStripMenuItem,
            this.saveSceneToolStripMenuItem,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // importAssetToolStripMenuItem
            // 
            this.importAssetToolStripMenuItem.Name = "importAssetToolStripMenuItem";
            this.importAssetToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
            this.importAssetToolStripMenuItem.Text = "Import Asset";
            this.importAssetToolStripMenuItem.Click += new System.EventHandler(this.importAssetToolStripMenuItem_Click);
            // 
            // loadMeshToolStripMenuItem
            // 
            this.loadMeshToolStripMenuItem.Name = "loadMeshToolStripMenuItem";
            this.loadMeshToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
            this.loadMeshToolStripMenuItem.Text = "Load";
            this.loadMeshToolStripMenuItem.Click += new System.EventHandler(this.loadMeshToolStripMenuItem_Click);
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
            this.saveToolStripMenuItem.Text = "Save";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
            // 
            // saveSceneToolStripMenuItem
            // 
            this.saveSceneToolStripMenuItem.Name = "saveSceneToolStripMenuItem";
            this.saveSceneToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Shift) 
            | System.Windows.Forms.Keys.S)));
            this.saveSceneToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
            this.saveSceneToolStripMenuItem.Text = "Save As";
            this.saveSceneToolStripMenuItem.Click += new System.EventHandler(this.saveSceneToolStripMenuItem_Click);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // createToolStripMenuItem
            // 
            this.createToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.createMeshInstanceToolStripMenuItem,
            this.createLightToolStripMenuItem});
            this.createToolStripMenuItem.Name = "createToolStripMenuItem";
            this.createToolStripMenuItem.Size = new System.Drawing.Size(53, 20);
            this.createToolStripMenuItem.Text = "Create";
            // 
            // createMeshInstanceToolStripMenuItem
            // 
            this.createMeshInstanceToolStripMenuItem.Name = "createMeshInstanceToolStripMenuItem";
            this.createMeshInstanceToolStripMenuItem.Size = new System.Drawing.Size(187, 22);
            this.createMeshInstanceToolStripMenuItem.Text = "Create Mesh Instance";
            this.createMeshInstanceToolStripMenuItem.Click += new System.EventHandler(this.createMeshInstanceToolStripMenuItem_Click);
            // 
            // createLightToolStripMenuItem
            // 
            this.createLightToolStripMenuItem.Name = "createLightToolStripMenuItem";
            this.createLightToolStripMenuItem.Size = new System.Drawing.Size(187, 22);
            this.createLightToolStripMenuItem.Text = "Create Light";
            this.createLightToolStripMenuItem.Click += new System.EventHandler(this.createLightToolStripMenuItem_Click);
            // 
            // viewToolStripMenuItem
            // 
            this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.itemShotNavmesh,
            this.itemInfluenceMap});
            this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
            this.viewToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.viewToolStripMenuItem.Text = "View";
            // 
            // itemShotNavmesh
            // 
            this.itemShotNavmesh.Checked = true;
            this.itemShotNavmesh.CheckOnClick = true;
            this.itemShotNavmesh.CheckState = System.Windows.Forms.CheckState.Checked;
            this.itemShotNavmesh.Name = "itemShotNavmesh";
            this.itemShotNavmesh.Size = new System.Drawing.Size(150, 22);
            this.itemShotNavmesh.Text = "Navmesh";
            this.itemShotNavmesh.Click += new System.EventHandler(this.itemShotNavmesh_Click);
            // 
            // itemInfluenceMap
            // 
            this.itemInfluenceMap.Checked = true;
            this.itemInfluenceMap.CheckOnClick = true;
            this.itemInfluenceMap.CheckState = System.Windows.Forms.CheckState.Checked;
            this.itemInfluenceMap.Name = "itemInfluenceMap";
            this.itemInfluenceMap.Size = new System.Drawing.Size(150, 22);
            this.itemInfluenceMap.Text = "Influence Map";
            this.itemInfluenceMap.Click += new System.EventHandler(this.itemInfluenceMap_Click);
            // 
            // panel_render
            // 
            this.panel_render.Location = new System.Drawing.Point(64, 118);
            this.panel_render.Name = "panel_render";
            this.panel_render.Size = new System.Drawing.Size(1280, 720);
            this.panel_render.TabIndex = 1;
            this.panel_render.Paint += new System.Windows.Forms.PaintEventHandler(this.panel_render_Paint);
            this.panel_render.MouseDown += new System.Windows.Forms.MouseEventHandler(this.panel_render_MouseDown);
            this.panel_render.MouseEnter += new System.EventHandler(this.panel_render_MouseEnter);
            this.panel_render.MouseMove += new System.Windows.Forms.MouseEventHandler(this.panel_render_MouseMove);
            this.panel_render.MouseUp += new System.Windows.Forms.MouseEventHandler(this.panel_render_MouseUp);
            // 
            // openFileDialog_importAsset
            // 
            this.openFileDialog_importAsset.Filter = "mesh|*.mesh|material|*.material";
            this.openFileDialog_importAsset.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog_import_FileOk);
            // 
            // treeView_entities
            // 
            this.treeView_entities.Location = new System.Drawing.Point(1519, 180);
            this.treeView_entities.Name = "treeView_entities";
            this.treeView_entities.Size = new System.Drawing.Size(280, 398);
            this.treeView_entities.TabIndex = 2;
            this.treeView_entities.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView_entities_AfterSelect);
            // 
            // numericUpDown_translation_x
            // 
            this.numericUpDown_translation_x.DecimalPlaces = 4;
            this.numericUpDown_translation_x.Location = new System.Drawing.Point(60, 3);
            this.numericUpDown_translation_x.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDown_translation_x.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDown_translation_x.Name = "numericUpDown_translation_x";
            this.numericUpDown_translation_x.Size = new System.Drawing.Size(100, 20);
            this.numericUpDown_translation_x.TabIndex = 3;
            this.numericUpDown_translation_x.ValueChanged += new System.EventHandler(this.numericUpDown_translation_x_ValueChanged);
            // 
            // numericUpDown_translation_y
            // 
            this.numericUpDown_translation_y.DecimalPlaces = 4;
            this.numericUpDown_translation_y.Location = new System.Drawing.Point(166, 3);
            this.numericUpDown_translation_y.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDown_translation_y.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDown_translation_y.Name = "numericUpDown_translation_y";
            this.numericUpDown_translation_y.Size = new System.Drawing.Size(100, 20);
            this.numericUpDown_translation_y.TabIndex = 4;
            this.numericUpDown_translation_y.ValueChanged += new System.EventHandler(this.numericUpDown_translation_y_ValueChanged);
            // 
            // numericUpDown_translation_z
            // 
            this.numericUpDown_translation_z.DecimalPlaces = 4;
            this.numericUpDown_translation_z.Location = new System.Drawing.Point(272, 3);
            this.numericUpDown_translation_z.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDown_translation_z.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDown_translation_z.Name = "numericUpDown_translation_z";
            this.numericUpDown_translation_z.Size = new System.Drawing.Size(100, 20);
            this.numericUpDown_translation_z.TabIndex = 5;
            this.numericUpDown_translation_z.ValueChanged += new System.EventHandler(this.numericUpDown_translation_z_ValueChanged);
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.AutoSize = true;
            this.flowLayoutPanel1.Controls.Add(this.numericUpDown_translation_z);
            this.flowLayoutPanel1.Controls.Add(this.numericUpDown_translation_y);
            this.flowLayoutPanel1.Controls.Add(this.numericUpDown_translation_x);
            this.flowLayoutPanel1.Controls.Add(this.label1);
            this.flowLayoutPanel1.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel1.Location = new System.Drawing.Point(3, 3);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(375, 26);
            this.flowLayoutPanel1.TabIndex = 7;
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(51, 26);
            this.label1.TabIndex = 8;
            this.label1.Text = "Translate";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // flowLayoutPanel2
            // 
            this.flowLayoutPanel2.AutoSize = true;
            this.flowLayoutPanel2.Controls.Add(this.numericUpDown_rot_z);
            this.flowLayoutPanel2.Controls.Add(this.numericUpDown_rot_y);
            this.flowLayoutPanel2.Controls.Add(this.numericUpDown_rot_x);
            this.flowLayoutPanel2.Controls.Add(this.label2);
            this.flowLayoutPanel2.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel2.Location = new System.Drawing.Point(3, 35);
            this.flowLayoutPanel2.Name = "flowLayoutPanel2";
            this.flowLayoutPanel2.Size = new System.Drawing.Size(363, 26);
            this.flowLayoutPanel2.TabIndex = 9;
            // 
            // numericUpDown_rot_z
            // 
            this.numericUpDown_rot_z.DecimalPlaces = 4;
            this.numericUpDown_rot_z.Location = new System.Drawing.Point(260, 3);
            this.numericUpDown_rot_z.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDown_rot_z.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDown_rot_z.Name = "numericUpDown_rot_z";
            this.numericUpDown_rot_z.Size = new System.Drawing.Size(100, 20);
            this.numericUpDown_rot_z.TabIndex = 5;
            // 
            // numericUpDown_rot_y
            // 
            this.numericUpDown_rot_y.DecimalPlaces = 4;
            this.numericUpDown_rot_y.Location = new System.Drawing.Point(154, 3);
            this.numericUpDown_rot_y.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDown_rot_y.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDown_rot_y.Name = "numericUpDown_rot_y";
            this.numericUpDown_rot_y.Size = new System.Drawing.Size(100, 20);
            this.numericUpDown_rot_y.TabIndex = 4;
            // 
            // numericUpDown_rot_x
            // 
            this.numericUpDown_rot_x.DecimalPlaces = 4;
            this.numericUpDown_rot_x.Location = new System.Drawing.Point(48, 3);
            this.numericUpDown_rot_x.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDown_rot_x.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDown_rot_x.Name = "numericUpDown_rot_x";
            this.numericUpDown_rot_x.Size = new System.Drawing.Size(100, 20);
            this.numericUpDown_rot_x.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(3, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(39, 26);
            this.label2.TabIndex = 8;
            this.label2.Text = "Rotate";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // flowLayoutPanel3
            // 
            this.flowLayoutPanel3.AutoSize = true;
            this.flowLayoutPanel3.Controls.Add(this.numericUpDown_scale_z);
            this.flowLayoutPanel3.Controls.Add(this.numericUpDown_scale_y);
            this.flowLayoutPanel3.Controls.Add(this.numericUpDown_scale_x);
            this.flowLayoutPanel3.Controls.Add(this.label3);
            this.flowLayoutPanel3.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel3.Location = new System.Drawing.Point(3, 67);
            this.flowLayoutPanel3.Name = "flowLayoutPanel3";
            this.flowLayoutPanel3.Size = new System.Drawing.Size(358, 26);
            this.flowLayoutPanel3.TabIndex = 9;
            // 
            // numericUpDown_scale_z
            // 
            this.numericUpDown_scale_z.DecimalPlaces = 4;
            this.numericUpDown_scale_z.Location = new System.Drawing.Point(255, 3);
            this.numericUpDown_scale_z.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDown_scale_z.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDown_scale_z.Name = "numericUpDown_scale_z";
            this.numericUpDown_scale_z.Size = new System.Drawing.Size(100, 20);
            this.numericUpDown_scale_z.TabIndex = 5;
            this.numericUpDown_scale_z.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // numericUpDown_scale_y
            // 
            this.numericUpDown_scale_y.DecimalPlaces = 4;
            this.numericUpDown_scale_y.Location = new System.Drawing.Point(149, 3);
            this.numericUpDown_scale_y.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDown_scale_y.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDown_scale_y.Name = "numericUpDown_scale_y";
            this.numericUpDown_scale_y.Size = new System.Drawing.Size(100, 20);
            this.numericUpDown_scale_y.TabIndex = 4;
            this.numericUpDown_scale_y.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // numericUpDown_scale_x
            // 
            this.numericUpDown_scale_x.DecimalPlaces = 4;
            this.numericUpDown_scale_x.Location = new System.Drawing.Point(43, 3);
            this.numericUpDown_scale_x.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDown_scale_x.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDown_scale_x.Name = "numericUpDown_scale_x";
            this.numericUpDown_scale_x.Size = new System.Drawing.Size(100, 20);
            this.numericUpDown_scale_x.TabIndex = 3;
            this.numericUpDown_scale_x.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // label3
            // 
            this.label3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(34, 26);
            this.label3.TabIndex = 8;
            this.label3.Text = "Scale";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // groupBoxMesh
            // 
            this.groupBoxMesh.Controls.Add(this.flowLayoutPanel8);
            this.groupBoxMesh.Location = new System.Drawing.Point(1380, 621);
            this.groupBoxMesh.Name = "groupBoxMesh";
            this.groupBoxMesh.Size = new System.Drawing.Size(419, 236);
            this.groupBoxMesh.TabIndex = 10;
            this.groupBoxMesh.TabStop = false;
            this.groupBoxMesh.Text = "Mesh Instance";
            // 
            // flowLayoutPanel8
            // 
            this.flowLayoutPanel8.Controls.Add(this.flowLayoutPanel1);
            this.flowLayoutPanel8.Controls.Add(this.flowLayoutPanel2);
            this.flowLayoutPanel8.Controls.Add(this.flowLayoutPanel3);
            this.flowLayoutPanel8.Controls.Add(this.flowLayoutPanel6);
            this.flowLayoutPanel8.Controls.Add(this.flowLayoutPanel9);
            this.flowLayoutPanel8.Controls.Add(this.flowLayoutPanel7);
            this.flowLayoutPanel8.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.flowLayoutPanel8.Location = new System.Drawing.Point(13, 19);
            this.flowLayoutPanel8.Name = "flowLayoutPanel8";
            this.flowLayoutPanel8.Size = new System.Drawing.Size(391, 200);
            this.flowLayoutPanel8.TabIndex = 18;
            // 
            // flowLayoutPanel6
            // 
            this.flowLayoutPanel6.AutoSize = true;
            this.flowLayoutPanel6.Controls.Add(this.textBoxMeshName);
            this.flowLayoutPanel6.Controls.Add(this.label6);
            this.flowLayoutPanel6.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel6.Location = new System.Drawing.Point(3, 99);
            this.flowLayoutPanel6.Name = "flowLayoutPanel6";
            this.flowLayoutPanel6.Size = new System.Drawing.Size(185, 26);
            this.flowLayoutPanel6.TabIndex = 19;
            // 
            // textBoxMeshName
            // 
            this.textBoxMeshName.Location = new System.Drawing.Point(47, 3);
            this.textBoxMeshName.MaxLength = 31;
            this.textBoxMeshName.Name = "textBoxMeshName";
            this.textBoxMeshName.Size = new System.Drawing.Size(135, 20);
            this.textBoxMeshName.TabIndex = 9;
            this.textBoxMeshName.TextChanged += new System.EventHandler(this.textBoxMeshName_TextChanged);
            this.textBoxMeshName.MouseLeave += new System.EventHandler(this.textBoxMeshName_MouseLeave);
            // 
            // label6
            // 
            this.label6.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(3, 0);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(38, 26);
            this.label6.TabIndex = 8;
            this.label6.Text = "Name:";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // flowLayoutPanel9
            // 
            this.flowLayoutPanel9.AutoSize = true;
            this.flowLayoutPanel9.Controls.Add(this.meshInstanceComboBoxMesh);
            this.flowLayoutPanel9.Controls.Add(this.label8);
            this.flowLayoutPanel9.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel9.Location = new System.Drawing.Point(3, 131);
            this.flowLayoutPanel9.Name = "flowLayoutPanel9";
            this.flowLayoutPanel9.Size = new System.Drawing.Size(252, 27);
            this.flowLayoutPanel9.TabIndex = 21;
            // 
            // meshInstanceComboBoxMesh
            // 
            this.meshInstanceComboBoxMesh.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.meshInstanceComboBoxMesh.FormattingEnabled = true;
            this.meshInstanceComboBoxMesh.Location = new System.Drawing.Point(45, 3);
            this.meshInstanceComboBoxMesh.Name = "meshInstanceComboBoxMesh";
            this.meshInstanceComboBoxMesh.Size = new System.Drawing.Size(204, 21);
            this.meshInstanceComboBoxMesh.TabIndex = 18;
            // 
            // label8
            // 
            this.label8.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(3, 0);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(36, 27);
            this.label8.TabIndex = 8;
            this.label8.Text = "Mesh:";
            this.label8.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // flowLayoutPanel7
            // 
            this.flowLayoutPanel7.AutoSize = true;
            this.flowLayoutPanel7.Controls.Add(this.meshInstanceComboBoxMaterial);
            this.flowLayoutPanel7.Controls.Add(this.label7);
            this.flowLayoutPanel7.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel7.Location = new System.Drawing.Point(3, 164);
            this.flowLayoutPanel7.Name = "flowLayoutPanel7";
            this.flowLayoutPanel7.Size = new System.Drawing.Size(252, 27);
            this.flowLayoutPanel7.TabIndex = 20;
            // 
            // meshInstanceComboBoxMaterial
            // 
            this.meshInstanceComboBoxMaterial.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.meshInstanceComboBoxMaterial.FormattingEnabled = true;
            this.meshInstanceComboBoxMaterial.Location = new System.Drawing.Point(53, 3);
            this.meshInstanceComboBoxMaterial.Name = "meshInstanceComboBoxMaterial";
            this.meshInstanceComboBoxMaterial.Size = new System.Drawing.Size(196, 21);
            this.meshInstanceComboBoxMaterial.TabIndex = 18;
            this.meshInstanceComboBoxMaterial.SelectedIndexChanged += new System.EventHandler(this.meshInstanceComboBoxMaterial_SelectedIndexChanged);
            // 
            // label7
            // 
            this.label7.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(3, 0);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(44, 27);
            this.label7.TabIndex = 8;
            this.label7.Text = "Material";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.comboBox_selectEditorMode,
            this.toolStripButtonCreateLight,
            this.toolStripButtonCreateMeshInstance});
            this.toolStrip1.Location = new System.Drawing.Point(0, 24);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(1904, 25);
            this.toolStrip1.TabIndex = 11;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // comboBox_selectEditorMode
            // 
            this.comboBox_selectEditorMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_selectEditorMode.Name = "comboBox_selectEditorMode";
            this.comboBox_selectEditorMode.Size = new System.Drawing.Size(121, 25);
            this.comboBox_selectEditorMode.SelectedIndexChanged += new System.EventHandler(this.comboBox_selectEditorMode_SelectedIndexChanged);
            this.comboBox_selectEditorMode.Click += new System.EventHandler(this.comboBox_selectEditorMode_Click);
            // 
            // toolStripButtonCreateLight
            // 
            this.toolStripButtonCreateLight.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonCreateLight.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonCreateLight.Image")));
            this.toolStripButtonCreateLight.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCreateLight.Name = "toolStripButtonCreateLight";
            this.toolStripButtonCreateLight.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonCreateLight.Text = "Create Light";
            this.toolStripButtonCreateLight.Visible = false;
            this.toolStripButtonCreateLight.Click += new System.EventHandler(this.toolStripButtonCreateLight_Click);
            // 
            // toolStripButtonCreateMeshInstance
            // 
            this.toolStripButtonCreateMeshInstance.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonCreateMeshInstance.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonCreateMeshInstance.Image")));
            this.toolStripButtonCreateMeshInstance.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCreateMeshInstance.Name = "toolStripButtonCreateMeshInstance";
            this.toolStripButtonCreateMeshInstance.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonCreateMeshInstance.Text = "Create Mesh Instance";
            this.toolStripButtonCreateMeshInstance.Visible = false;
            this.toolStripButtonCreateMeshInstance.Click += new System.EventHandler(this.toolStripButtonCreateMeshInstance_Click);
            // 
            // openFileDialog1_loadScene
            // 
            this.openFileDialog1_loadScene.FileName = "openFileDialog1";
            this.openFileDialog1_loadScene.Filter = "scene|*scene";
            this.openFileDialog1_loadScene.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog1_loadScene_FileOk);
            // 
            // saveFileDialog_scene
            // 
            this.saveFileDialog_scene.DefaultExt = "scene";
            this.saveFileDialog_scene.Filter = "Scene|*.scene";
            this.saveFileDialog_scene.FileOk += new System.ComponentModel.CancelEventHandler(this.saveFileDialog_scene_FileOk);
            // 
            // numericUpDownNavmeshPositionZ
            // 
            this.numericUpDownNavmeshPositionZ.DecimalPlaces = 4;
            this.numericUpDownNavmeshPositionZ.Location = new System.Drawing.Point(265, 3);
            this.numericUpDownNavmeshPositionZ.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownNavmeshPositionZ.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownNavmeshPositionZ.Name = "numericUpDownNavmeshPositionZ";
            this.numericUpDownNavmeshPositionZ.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownNavmeshPositionZ.TabIndex = 14;
            this.numericUpDownNavmeshPositionZ.ValueChanged += new System.EventHandler(this.numericUpDownNavmeshPositionZ_ValueChanged);
            // 
            // numericUpDownNavmeshPositionY
            // 
            this.numericUpDownNavmeshPositionY.DecimalPlaces = 4;
            this.numericUpDownNavmeshPositionY.Location = new System.Drawing.Point(159, 3);
            this.numericUpDownNavmeshPositionY.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownNavmeshPositionY.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownNavmeshPositionY.Name = "numericUpDownNavmeshPositionY";
            this.numericUpDownNavmeshPositionY.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownNavmeshPositionY.TabIndex = 13;
            this.numericUpDownNavmeshPositionY.ValueChanged += new System.EventHandler(this.numericUpDownNavmeshPositionY_ValueChanged);
            // 
            // numericUpDownNavmeshPositionX
            // 
            this.numericUpDownNavmeshPositionX.DecimalPlaces = 4;
            this.numericUpDownNavmeshPositionX.Location = new System.Drawing.Point(53, 3);
            this.numericUpDownNavmeshPositionX.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownNavmeshPositionX.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownNavmeshPositionX.Name = "numericUpDownNavmeshPositionX";
            this.numericUpDownNavmeshPositionX.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownNavmeshPositionX.TabIndex = 12;
            this.numericUpDownNavmeshPositionX.ValueChanged += new System.EventHandler(this.numericUpDownNavmeshPositionX_ValueChanged);
            // 
            // flowLayoutPanel4
            // 
            this.flowLayoutPanel4.Controls.Add(this.label4);
            this.flowLayoutPanel4.Controls.Add(this.numericUpDownNavmeshPositionX);
            this.flowLayoutPanel4.Controls.Add(this.numericUpDownNavmeshPositionY);
            this.flowLayoutPanel4.Controls.Add(this.numericUpDownNavmeshPositionZ);
            this.flowLayoutPanel4.Location = new System.Drawing.Point(12, 19);
            this.flowLayoutPanel4.Name = "flowLayoutPanel4";
            this.flowLayoutPanel4.Size = new System.Drawing.Size(380, 28);
            this.flowLayoutPanel4.TabIndex = 15;
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(3, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(44, 26);
            this.label4.TabIndex = 16;
            this.label4.Text = "Position";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // groupBoxNavMesh
            // 
            this.groupBoxNavMesh.Controls.Add(this.flowLayoutPanel4);
            this.groupBoxNavMesh.Location = new System.Drawing.Point(1386, 910);
            this.groupBoxNavMesh.Name = "groupBoxNavMesh";
            this.groupBoxNavMesh.Size = new System.Drawing.Size(398, 59);
            this.groupBoxNavMesh.TabIndex = 16;
            this.groupBoxNavMesh.TabStop = false;
            this.groupBoxNavMesh.Text = "Navmesh";
            // 
            // groupBoxLight
            // 
            this.groupBoxLight.Controls.Add(this.flowLayoutPanel5);
            this.groupBoxLight.Location = new System.Drawing.Point(1386, 975);
            this.groupBoxLight.Name = "groupBoxLight";
            this.groupBoxLight.Size = new System.Drawing.Size(398, 59);
            this.groupBoxLight.TabIndex = 17;
            this.groupBoxLight.TabStop = false;
            this.groupBoxLight.Text = "Lights";
            // 
            // flowLayoutPanel5
            // 
            this.flowLayoutPanel5.Controls.Add(this.label5);
            this.flowLayoutPanel5.Controls.Add(this.numericUpDownLightX);
            this.flowLayoutPanel5.Controls.Add(this.numericUpDownLightY);
            this.flowLayoutPanel5.Controls.Add(this.numericUpDownLightZ);
            this.flowLayoutPanel5.Location = new System.Drawing.Point(12, 19);
            this.flowLayoutPanel5.Name = "flowLayoutPanel5";
            this.flowLayoutPanel5.Size = new System.Drawing.Size(380, 28);
            this.flowLayoutPanel5.TabIndex = 15;
            // 
            // label5
            // 
            this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(3, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(44, 26);
            this.label5.TabIndex = 16;
            this.label5.Text = "Position";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // numericUpDownLightX
            // 
            this.numericUpDownLightX.DecimalPlaces = 4;
            this.numericUpDownLightX.Location = new System.Drawing.Point(53, 3);
            this.numericUpDownLightX.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownLightX.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownLightX.Name = "numericUpDownLightX";
            this.numericUpDownLightX.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownLightX.TabIndex = 12;
            this.numericUpDownLightX.ValueChanged += new System.EventHandler(this.numericUpDownLightX_ValueChanged);
            // 
            // numericUpDownLightY
            // 
            this.numericUpDownLightY.DecimalPlaces = 4;
            this.numericUpDownLightY.Location = new System.Drawing.Point(159, 3);
            this.numericUpDownLightY.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownLightY.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownLightY.Name = "numericUpDownLightY";
            this.numericUpDownLightY.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownLightY.TabIndex = 13;
            this.numericUpDownLightY.ValueChanged += new System.EventHandler(this.numericUpDownLightY_ValueChanged);
            // 
            // numericUpDownLightZ
            // 
            this.numericUpDownLightZ.DecimalPlaces = 4;
            this.numericUpDownLightZ.Location = new System.Drawing.Point(265, 3);
            this.numericUpDownLightZ.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownLightZ.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownLightZ.Name = "numericUpDownLightZ";
            this.numericUpDownLightZ.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownLightZ.TabIndex = 14;
            this.numericUpDownLightZ.ValueChanged += new System.EventHandler(this.numericUpDownLightZ_ValueChanged);
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.undoToolStripMenuItem,
            this.redoToolStripMenuItem});
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
            this.editToolStripMenuItem.Text = "Edit";
            // 
            // undoToolStripMenuItem
            // 
            this.undoToolStripMenuItem.Name = "undoToolStripMenuItem";
            this.undoToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.undoToolStripMenuItem.Text = "Undo";
            this.undoToolStripMenuItem.Click += new System.EventHandler(this.undoToolStripMenuItem_Click);
            // 
            // redoToolStripMenuItem
            // 
            this.redoToolStripMenuItem.Name = "redoToolStripMenuItem";
            this.redoToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.redoToolStripMenuItem.Text = "Redo";
            this.redoToolStripMenuItem.Click += new System.EventHandler(this.redoToolStripMenuItem_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1904, 1041);
            this.Controls.Add(this.groupBoxLight);
            this.Controls.Add(this.groupBoxNavMesh);
            this.Controls.Add(this.toolStrip1);
            this.Controls.Add(this.groupBoxMesh);
            this.Controls.Add(this.treeView_entities);
            this.Controls.Add(this.panel_render);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form1";
            this.Text = "vxEditor";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.Form1_KeyDown);
            this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Form1_KeyUp);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_x)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_y)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_z)).EndInit();
            this.flowLayoutPanel1.ResumeLayout(false);
            this.flowLayoutPanel1.PerformLayout();
            this.flowLayoutPanel2.ResumeLayout(false);
            this.flowLayoutPanel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_rot_z)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_rot_y)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_rot_x)).EndInit();
            this.flowLayoutPanel3.ResumeLayout(false);
            this.flowLayoutPanel3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_scale_z)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_scale_y)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_scale_x)).EndInit();
            this.groupBoxMesh.ResumeLayout(false);
            this.flowLayoutPanel8.ResumeLayout(false);
            this.flowLayoutPanel8.PerformLayout();
            this.flowLayoutPanel6.ResumeLayout(false);
            this.flowLayoutPanel6.PerformLayout();
            this.flowLayoutPanel9.ResumeLayout(false);
            this.flowLayoutPanel9.PerformLayout();
            this.flowLayoutPanel7.ResumeLayout(false);
            this.flowLayoutPanel7.PerformLayout();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionX)).EndInit();
            this.flowLayoutPanel4.ResumeLayout(false);
            this.flowLayoutPanel4.PerformLayout();
            this.groupBoxNavMesh.ResumeLayout(false);
            this.groupBoxLight.ResumeLayout(false);
            this.flowLayoutPanel5.ResumeLayout(false);
            this.flowLayoutPanel5.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightZ)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadMeshToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.Panel panel_render;
        private System.Windows.Forms.ToolStripMenuItem createToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem createMeshInstanceToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem createLightToolStripMenuItem;
        private System.Windows.Forms.OpenFileDialog openFileDialog_importAsset;
        private System.Windows.Forms.TreeView treeView_entities;
        private System.Windows.Forms.NumericUpDown numericUpDown_translation_x;
        private System.Windows.Forms.NumericUpDown numericUpDown_translation_y;
        private System.Windows.Forms.NumericUpDown numericUpDown_translation_z;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel2;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown numericUpDown_rot_x;
        private System.Windows.Forms.NumericUpDown numericUpDown_rot_y;
        private System.Windows.Forms.NumericUpDown numericUpDown_rot_z;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel3;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown numericUpDown_scale_x;
        private System.Windows.Forms.NumericUpDown numericUpDown_scale_y;
        private System.Windows.Forms.NumericUpDown numericUpDown_scale_z;
        private System.Windows.Forms.GroupBox groupBoxMesh;
        private System.Windows.Forms.ToolStripMenuItem saveSceneToolStripMenuItem;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripMenuItem importAssetToolStripMenuItem;
        private System.Windows.Forms.OpenFileDialog openFileDialog1_loadScene;
        private System.Windows.Forms.SaveFileDialog saveFileDialog_scene;
        private System.Windows.Forms.ToolStripComboBox comboBox_selectEditorMode;
        private System.Windows.Forms.NumericUpDown numericUpDownNavmeshPositionZ;
        private System.Windows.Forms.NumericUpDown numericUpDownNavmeshPositionY;
        private System.Windows.Forms.NumericUpDown numericUpDownNavmeshPositionX;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel4;
        private System.Windows.Forms.GroupBox groupBoxNavMesh;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem itemShotNavmesh;
        private System.Windows.Forms.ToolStripMenuItem itemInfluenceMap;
        private System.Windows.Forms.GroupBox groupBoxLight;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel5;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown numericUpDownLightX;
        private System.Windows.Forms.NumericUpDown numericUpDownLightY;
        private System.Windows.Forms.NumericUpDown numericUpDownLightZ;
        private System.Windows.Forms.ToolStripButton toolStripButtonCreateLight;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel6;
        private System.Windows.Forms.TextBox textBoxMeshName;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel8;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel9;
        private System.Windows.Forms.ComboBox meshInstanceComboBoxMesh;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel7;
        private System.Windows.Forms.ComboBox meshInstanceComboBoxMaterial;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.ToolStripButton toolStripButtonCreateMeshInstance;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem undoToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem redoToolStripMenuItem;
    }
}

