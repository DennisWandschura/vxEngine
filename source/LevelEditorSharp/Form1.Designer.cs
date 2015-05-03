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
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.comboBox_selectEditorMode = new System.Windows.Forms.ToolStripComboBox();
            this.openFileDialog1_loadScene = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog_scene = new System.Windows.Forms.SaveFileDialog();
            this.numericUpDownNavmeshPositionZ = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownNavmeshPositionY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownNavmeshPositionX = new System.Windows.Forms.NumericUpDown();
            this.flowLayoutPanel4 = new System.Windows.Forms.FlowLayoutPanel();
            this.groupBoxNavMesh = new System.Windows.Forms.GroupBox();
            this.label4 = new System.Windows.Forms.Label();
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
            this.toolStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionX)).BeginInit();
            this.flowLayoutPanel4.SuspendLayout();
            this.groupBoxNavMesh.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.createToolStripMenuItem});
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
            this.flowLayoutPanel1.Location = new System.Drawing.Point(6, 19);
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
            this.flowLayoutPanel2.Location = new System.Drawing.Point(6, 51);
            this.flowLayoutPanel2.Name = "flowLayoutPanel2";
            this.flowLayoutPanel2.Size = new System.Drawing.Size(375, 26);
            this.flowLayoutPanel2.TabIndex = 9;
            // 
            // numericUpDown_rot_z
            // 
            this.numericUpDown_rot_z.DecimalPlaces = 4;
            this.numericUpDown_rot_z.Location = new System.Drawing.Point(272, 3);
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
            this.numericUpDown_rot_y.Location = new System.Drawing.Point(166, 3);
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
            this.numericUpDown_rot_x.Location = new System.Drawing.Point(60, 3);
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
            this.label2.Location = new System.Drawing.Point(15, 0);
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
            this.flowLayoutPanel3.Location = new System.Drawing.Point(6, 83);
            this.flowLayoutPanel3.Name = "flowLayoutPanel3";
            this.flowLayoutPanel3.Size = new System.Drawing.Size(375, 26);
            this.flowLayoutPanel3.TabIndex = 9;
            // 
            // numericUpDown_scale_z
            // 
            this.numericUpDown_scale_z.DecimalPlaces = 4;
            this.numericUpDown_scale_z.Location = new System.Drawing.Point(272, 3);
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
            this.numericUpDown_scale_y.Location = new System.Drawing.Point(166, 3);
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
            this.numericUpDown_scale_x.Location = new System.Drawing.Point(60, 3);
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
            this.label3.Location = new System.Drawing.Point(20, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(34, 26);
            this.label3.TabIndex = 8;
            this.label3.Text = "Scale";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // groupBoxMesh
            // 
            this.groupBoxMesh.Controls.Add(this.flowLayoutPanel1);
            this.groupBoxMesh.Controls.Add(this.flowLayoutPanel2);
            this.groupBoxMesh.Controls.Add(this.flowLayoutPanel3);
            this.groupBoxMesh.Location = new System.Drawing.Point(1380, 621);
            this.groupBoxMesh.Name = "groupBoxMesh";
            this.groupBoxMesh.Size = new System.Drawing.Size(398, 124);
            this.groupBoxMesh.TabIndex = 10;
            this.groupBoxMesh.TabStop = false;
            this.groupBoxMesh.Text = "Transform";
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.comboBox_selectEditorMode});
            this.toolStrip1.Location = new System.Drawing.Point(0, 24);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(1904, 25);
            this.toolStrip1.TabIndex = 11;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // comboBox_selectEditorMode
            // 
            this.comboBox_selectEditorMode.Name = "comboBox_selectEditorMode";
            this.comboBox_selectEditorMode.Size = new System.Drawing.Size(121, 25);
            this.comboBox_selectEditorMode.SelectedIndexChanged += new System.EventHandler(this.comboBox_selectEditorMode_SelectedIndexChanged);
            this.comboBox_selectEditorMode.Click += new System.EventHandler(this.comboBox_selectEditorMode_Click);
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
            this.flowLayoutPanel4.Location = new System.Drawing.Point(12, 34);
            this.flowLayoutPanel4.Name = "flowLayoutPanel4";
            this.flowLayoutPanel4.Size = new System.Drawing.Size(380, 28);
            this.flowLayoutPanel4.TabIndex = 15;
            // 
            // groupBoxNavMesh
            // 
            this.groupBoxNavMesh.Controls.Add(this.flowLayoutPanel4);
            this.groupBoxNavMesh.Location = new System.Drawing.Point(1380, 760);
            this.groupBoxNavMesh.Name = "groupBoxNavMesh";
            this.groupBoxNavMesh.Size = new System.Drawing.Size(398, 100);
            this.groupBoxNavMesh.TabIndex = 16;
            this.groupBoxNavMesh.TabStop = false;
            this.groupBoxNavMesh.Text = "Navmesh";
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
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1904, 1041);
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
            this.groupBoxMesh.PerformLayout();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionX)).EndInit();
            this.flowLayoutPanel4.ResumeLayout(false);
            this.flowLayoutPanel4.PerformLayout();
            this.groupBoxNavMesh.ResumeLayout(false);
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
    }
}

