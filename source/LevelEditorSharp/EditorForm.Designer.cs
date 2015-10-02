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
    partial class EditorForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(EditorForm));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.importAssetToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadMeshToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveSceneToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.undoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.redoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addJointToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.removeJointToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createMeshInstanceToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createLightToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createActorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.itemShotNavmesh = new System.Windows.Forms.ToolStripMenuItem();
            this.itemInfluenceMap = new System.Windows.Forms.ToolStripMenuItem();
            this.panel_render = new System.Windows.Forms.Panel();
            this.openFileDialog_importAsset = new System.Windows.Forms.OpenFileDialog();
            this.treeView_entities = new System.Windows.Forms.TreeView();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.comboBox_selectEditorMode = new System.Windows.Forms.ToolStripComboBox();
            this.toolStripButtonCreateLight = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonCreateMeshInstance = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonCreateSpawn = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonCreateJoint = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonCreateLightGeometryProxy = new System.Windows.Forms.ToolStripButton();
            this.openFileDialog1_loadScene = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog_scene = new System.Windows.Forms.SaveFileDialog();
            this.numericUpDownNavmeshPositionZ = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownNavmeshPositionY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownNavmeshPositionX = new System.Windows.Forms.NumericUpDown();
            this.flowLayoutPanel4 = new System.Windows.Forms.FlowLayoutPanel();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBoxNavMesh = new System.Windows.Forms.GroupBox();
            this.groupBoxLight = new System.Windows.Forms.GroupBox();
            this.flowLayoutPanel10 = new System.Windows.Forms.FlowLayoutPanel();
            this.label9 = new System.Windows.Forms.Label();
            this.numericUpDownLightFalloff = new System.Windows.Forms.NumericUpDown();
            this.flowLayoutPanel5 = new System.Windows.Forms.FlowLayoutPanel();
            this.label5 = new System.Windows.Forms.Label();
            this.numericUpDownLightX = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownLightY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownLightZ = new System.Windows.Forms.NumericUpDown();
            this.flowLayoutPanel11 = new System.Windows.Forms.FlowLayoutPanel();
            this.label10 = new System.Windows.Forms.Label();
            this.numericUpDownLightLumen = new System.Windows.Forms.NumericUpDown();
            this.treeViewActionList = new System.Windows.Forms.TreeView();
            this.groupBoxSpawn = new System.Windows.Forms.GroupBox();
            this.comboBoxActor = new System.Windows.Forms.ComboBox();
            this.flowLayoutPanel13 = new System.Windows.Forms.FlowLayoutPanel();
            this.label12 = new System.Windows.Forms.Label();
            this.numericUpDownSpawnType = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.flowLayoutPanel12 = new System.Windows.Forms.FlowLayoutPanel();
            this.label11 = new System.Windows.Forms.Label();
            this.numericUpDownSpawnPosX = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownSpawnPosY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownSpawnPosZ = new System.Windows.Forms.NumericUpDown();
            this.menuStrip1.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionX)).BeginInit();
            this.flowLayoutPanel4.SuspendLayout();
            this.groupBoxNavMesh.SuspendLayout();
            this.groupBoxLight.SuspendLayout();
            this.flowLayoutPanel10.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightFalloff)).BeginInit();
            this.flowLayoutPanel5.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightZ)).BeginInit();
            this.flowLayoutPanel11.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightLumen)).BeginInit();
            this.groupBoxSpawn.SuspendLayout();
            this.flowLayoutPanel13.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnType)).BeginInit();
            this.flowLayoutPanel12.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosZ)).BeginInit();
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
            this.menuStrip1.Size = new System.Drawing.Size(2544, 24);
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
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.undoToolStripMenuItem,
            this.redoToolStripMenuItem,
            this.addJointToolStripMenuItem,
            this.removeJointToolStripMenuItem});
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
            this.editToolStripMenuItem.Text = "Edit";
            // 
            // undoToolStripMenuItem
            // 
            this.undoToolStripMenuItem.Name = "undoToolStripMenuItem";
            this.undoToolStripMenuItem.Size = new System.Drawing.Size(145, 22);
            this.undoToolStripMenuItem.Text = "Undo";
            this.undoToolStripMenuItem.Click += new System.EventHandler(this.undoToolStripMenuItem_Click);
            // 
            // redoToolStripMenuItem
            // 
            this.redoToolStripMenuItem.Name = "redoToolStripMenuItem";
            this.redoToolStripMenuItem.Size = new System.Drawing.Size(145, 22);
            this.redoToolStripMenuItem.Text = "Redo";
            this.redoToolStripMenuItem.Click += new System.EventHandler(this.redoToolStripMenuItem_Click);
            // 
            // addJointToolStripMenuItem
            // 
            this.addJointToolStripMenuItem.Name = "addJointToolStripMenuItem";
            this.addJointToolStripMenuItem.Size = new System.Drawing.Size(145, 22);
            this.addJointToolStripMenuItem.Text = "Add Joint";
            this.addJointToolStripMenuItem.Visible = false;
            this.addJointToolStripMenuItem.Click += new System.EventHandler(this.addJointToolStripMenuItem_Click);
            // 
            // removeJointToolStripMenuItem
            // 
            this.removeJointToolStripMenuItem.Name = "removeJointToolStripMenuItem";
            this.removeJointToolStripMenuItem.Size = new System.Drawing.Size(145, 22);
            this.removeJointToolStripMenuItem.Text = "Remove Joint";
            this.removeJointToolStripMenuItem.Visible = false;
            this.removeJointToolStripMenuItem.Click += new System.EventHandler(this.removeJointToolStripMenuItem_Click);
            // 
            // createToolStripMenuItem
            // 
            this.createToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.createMeshInstanceToolStripMenuItem,
            this.createLightToolStripMenuItem,
            this.createActorToolStripMenuItem});
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
            // createActorToolStripMenuItem
            // 
            this.createActorToolStripMenuItem.Name = "createActorToolStripMenuItem";
            this.createActorToolStripMenuItem.Size = new System.Drawing.Size(187, 22);
            this.createActorToolStripMenuItem.Text = "Create Actor";
            this.createActorToolStripMenuItem.Click += new System.EventHandler(this.createActorToolStripMenuItem_Click);
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
            this.panel_render.Location = new System.Drawing.Point(12, 118);
            this.panel_render.Name = "panel_render";
            this.panel_render.Size = new System.Drawing.Size(1920, 1080);
            this.panel_render.TabIndex = 1;
            this.panel_render.Paint += new System.Windows.Forms.PaintEventHandler(this.panel_render_Paint);
            this.panel_render.MouseDown += new System.Windows.Forms.MouseEventHandler(this.panel_render_MouseDown);
            this.panel_render.MouseEnter += new System.EventHandler(this.panel_render_MouseEnter);
            this.panel_render.MouseMove += new System.Windows.Forms.MouseEventHandler(this.panel_render_MouseMove);
            this.panel_render.MouseUp += new System.Windows.Forms.MouseEventHandler(this.panel_render_MouseUp);
            // 
            // openFileDialog_importAsset
            // 
            this.openFileDialog_importAsset.Filter = "mesh|*.mesh|material|*.material|fbx|*.fbx|animation|*.animation";
            this.openFileDialog_importAsset.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog_import_FileOk);
            // 
            // treeView_entities
            // 
            this.treeView_entities.Location = new System.Drawing.Point(1977, 118);
            this.treeView_entities.Name = "treeView_entities";
            this.treeView_entities.Size = new System.Drawing.Size(280, 398);
            this.treeView_entities.TabIndex = 2;
            this.treeView_entities.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView_entities_AfterSelect);
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.comboBox_selectEditorMode,
            this.toolStripButtonCreateLight,
            this.toolStripButtonCreateMeshInstance,
            this.toolStripButtonCreateSpawn,
            this.toolStripButtonCreateJoint,
            this.toolStripButtonCreateLightGeometryProxy});
            this.toolStrip1.Location = new System.Drawing.Point(0, 24);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(2544, 25);
            this.toolStrip1.TabIndex = 11;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // comboBox_selectEditorMode
            // 
            this.comboBox_selectEditorMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_selectEditorMode.Name = "comboBox_selectEditorMode";
            this.comboBox_selectEditorMode.Size = new System.Drawing.Size(150, 25);
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
            // toolStripButtonCreateSpawn
            // 
            this.toolStripButtonCreateSpawn.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonCreateSpawn.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonCreateSpawn.Image")));
            this.toolStripButtonCreateSpawn.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCreateSpawn.Name = "toolStripButtonCreateSpawn";
            this.toolStripButtonCreateSpawn.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonCreateSpawn.Text = "Create Spawn";
            this.toolStripButtonCreateSpawn.Visible = false;
            this.toolStripButtonCreateSpawn.Click += new System.EventHandler(this.toolStripButtonCreateSpawn_Click);
            // 
            // toolStripButtonCreateJoint
            // 
            this.toolStripButtonCreateJoint.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonCreateJoint.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonCreateJoint.Image")));
            this.toolStripButtonCreateJoint.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCreateJoint.Name = "toolStripButtonCreateJoint";
            this.toolStripButtonCreateJoint.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonCreateJoint.Text = "Create Joint";
            this.toolStripButtonCreateJoint.Visible = false;
            this.toolStripButtonCreateJoint.Click += new System.EventHandler(this.toolStripButtonCreateJoint_Click);
            // 
            // toolStripButtonCreateLightGeometryProxy
            // 
            this.toolStripButtonCreateLightGeometryProxy.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonCreateLightGeometryProxy.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonCreateLightGeometryProxy.Image")));
            this.toolStripButtonCreateLightGeometryProxy.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCreateLightGeometryProxy.Name = "toolStripButtonCreateLightGeometryProxy";
            this.toolStripButtonCreateLightGeometryProxy.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonCreateLightGeometryProxy.Text = "Create Light Geometry Proxy";
            this.toolStripButtonCreateLightGeometryProxy.Visible = false;
            this.toolStripButtonCreateLightGeometryProxy.Click += new System.EventHandler(this.toolStripButtonCreateLightProxyGeometry_Click);
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
            this.groupBoxNavMesh.Location = new System.Drawing.Point(1983, 957);
            this.groupBoxNavMesh.Name = "groupBoxNavMesh";
            this.groupBoxNavMesh.Size = new System.Drawing.Size(398, 59);
            this.groupBoxNavMesh.TabIndex = 16;
            this.groupBoxNavMesh.TabStop = false;
            this.groupBoxNavMesh.Text = "Navmesh";
            // 
            // groupBoxLight
            // 
            this.groupBoxLight.Controls.Add(this.flowLayoutPanel10);
            this.groupBoxLight.Controls.Add(this.flowLayoutPanel5);
            this.groupBoxLight.Controls.Add(this.flowLayoutPanel11);
            this.groupBoxLight.Location = new System.Drawing.Point(1983, 1022);
            this.groupBoxLight.Name = "groupBoxLight";
            this.groupBoxLight.Size = new System.Drawing.Size(398, 124);
            this.groupBoxLight.TabIndex = 17;
            this.groupBoxLight.TabStop = false;
            this.groupBoxLight.Text = "Lights";
            // 
            // flowLayoutPanel10
            // 
            this.flowLayoutPanel10.Controls.Add(this.label9);
            this.flowLayoutPanel10.Controls.Add(this.numericUpDownLightFalloff);
            this.flowLayoutPanel10.Location = new System.Drawing.Point(12, 53);
            this.flowLayoutPanel10.Name = "flowLayoutPanel10";
            this.flowLayoutPanel10.Size = new System.Drawing.Size(380, 28);
            this.flowLayoutPanel10.TabIndex = 15;
            // 
            // label9
            // 
            this.label9.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(3, 0);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(35, 26);
            this.label9.TabIndex = 16;
            this.label9.Text = "Falloff";
            this.label9.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // numericUpDownLightFalloff
            // 
            this.numericUpDownLightFalloff.DecimalPlaces = 4;
            this.numericUpDownLightFalloff.Location = new System.Drawing.Point(44, 3);
            this.numericUpDownLightFalloff.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownLightFalloff.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownLightFalloff.Name = "numericUpDownLightFalloff";
            this.numericUpDownLightFalloff.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownLightFalloff.TabIndex = 12;
            this.numericUpDownLightFalloff.ValueChanged += new System.EventHandler(this.numericUpDownLightFalloff_ValueChanged);
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
            // flowLayoutPanel11
            // 
            this.flowLayoutPanel11.Controls.Add(this.label10);
            this.flowLayoutPanel11.Controls.Add(this.numericUpDownLightLumen);
            this.flowLayoutPanel11.Location = new System.Drawing.Point(12, 87);
            this.flowLayoutPanel11.Name = "flowLayoutPanel11";
            this.flowLayoutPanel11.Size = new System.Drawing.Size(380, 28);
            this.flowLayoutPanel11.TabIndex = 17;
            // 
            // label10
            // 
            this.label10.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(3, 0);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(39, 26);
            this.label10.TabIndex = 16;
            this.label10.Text = "Lumen";
            this.label10.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // numericUpDownLightLumen
            // 
            this.numericUpDownLightLumen.DecimalPlaces = 4;
            this.numericUpDownLightLumen.Location = new System.Drawing.Point(48, 3);
            this.numericUpDownLightLumen.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownLightLumen.Name = "numericUpDownLightLumen";
            this.numericUpDownLightLumen.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownLightLumen.TabIndex = 12;
            this.numericUpDownLightLumen.ValueChanged += new System.EventHandler(this.numericUpDownLightLumen_ValueChanged);
            // 
            // treeViewActionList
            // 
            this.treeViewActionList.Location = new System.Drawing.Point(2295, 118);
            this.treeViewActionList.Name = "treeViewActionList";
            this.treeViewActionList.Size = new System.Drawing.Size(209, 398);
            this.treeViewActionList.TabIndex = 18;
            this.treeViewActionList.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeViewActionList_AfterSelect);
            // 
            // groupBoxSpawn
            // 
            this.groupBoxSpawn.Controls.Add(this.comboBoxActor);
            this.groupBoxSpawn.Controls.Add(this.flowLayoutPanel13);
            this.groupBoxSpawn.Controls.Add(this.label1);
            this.groupBoxSpawn.Controls.Add(this.flowLayoutPanel12);
            this.groupBoxSpawn.Location = new System.Drawing.Point(1990, 1198);
            this.groupBoxSpawn.Name = "groupBoxSpawn";
            this.groupBoxSpawn.Size = new System.Drawing.Size(398, 147);
            this.groupBoxSpawn.TabIndex = 17;
            this.groupBoxSpawn.TabStop = false;
            this.groupBoxSpawn.Text = "Spawn";
            // 
            // comboBoxActor
            // 
            this.comboBoxActor.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxActor.FormattingEnabled = true;
            this.comboBoxActor.Location = new System.Drawing.Point(130, 102);
            this.comboBoxActor.Name = "comboBoxActor";
            this.comboBoxActor.Size = new System.Drawing.Size(121, 21);
            this.comboBoxActor.TabIndex = 19;
            this.comboBoxActor.SelectedIndexChanged += new System.EventHandler(this.comboBoxActor_SelectedIndexChanged);
            // 
            // flowLayoutPanel13
            // 
            this.flowLayoutPanel13.Controls.Add(this.label12);
            this.flowLayoutPanel13.Controls.Add(this.numericUpDownSpawnType);
            this.flowLayoutPanel13.Location = new System.Drawing.Point(12, 53);
            this.flowLayoutPanel13.Name = "flowLayoutPanel13";
            this.flowLayoutPanel13.Size = new System.Drawing.Size(380, 28);
            this.flowLayoutPanel13.TabIndex = 18;
            // 
            // label12
            // 
            this.label12.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(3, 0);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(31, 26);
            this.label12.TabIndex = 16;
            this.label12.Text = "Type";
            this.label12.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // numericUpDownSpawnType
            // 
            this.numericUpDownSpawnType.Location = new System.Drawing.Point(40, 3);
            this.numericUpDownSpawnType.Maximum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numericUpDownSpawnType.Name = "numericUpDownSpawnType";
            this.numericUpDownSpawnType.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownSpawnType.TabIndex = 12;
            this.numericUpDownSpawnType.ValueChanged += new System.EventHandler(this.numericUpDownSpawnType_ValueChanged);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(89, 110);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(35, 13);
            this.label1.TabIndex = 17;
            this.label1.Text = "Actor:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // flowLayoutPanel12
            // 
            this.flowLayoutPanel12.Controls.Add(this.label11);
            this.flowLayoutPanel12.Controls.Add(this.numericUpDownSpawnPosX);
            this.flowLayoutPanel12.Controls.Add(this.numericUpDownSpawnPosY);
            this.flowLayoutPanel12.Controls.Add(this.numericUpDownSpawnPosZ);
            this.flowLayoutPanel12.Location = new System.Drawing.Point(12, 19);
            this.flowLayoutPanel12.Name = "flowLayoutPanel12";
            this.flowLayoutPanel12.Size = new System.Drawing.Size(380, 28);
            this.flowLayoutPanel12.TabIndex = 15;
            // 
            // label11
            // 
            this.label11.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(3, 0);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(44, 26);
            this.label11.TabIndex = 16;
            this.label11.Text = "Position";
            this.label11.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // numericUpDownSpawnPosX
            // 
            this.numericUpDownSpawnPosX.DecimalPlaces = 4;
            this.numericUpDownSpawnPosX.Location = new System.Drawing.Point(53, 3);
            this.numericUpDownSpawnPosX.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownSpawnPosX.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownSpawnPosX.Name = "numericUpDownSpawnPosX";
            this.numericUpDownSpawnPosX.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownSpawnPosX.TabIndex = 12;
            this.numericUpDownSpawnPosX.ValueChanged += new System.EventHandler(this.numericUpDownSpawnPosX_ValueChanged);
            // 
            // numericUpDownSpawnPosY
            // 
            this.numericUpDownSpawnPosY.DecimalPlaces = 4;
            this.numericUpDownSpawnPosY.Location = new System.Drawing.Point(159, 3);
            this.numericUpDownSpawnPosY.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownSpawnPosY.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownSpawnPosY.Name = "numericUpDownSpawnPosY";
            this.numericUpDownSpawnPosY.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownSpawnPosY.TabIndex = 13;
            this.numericUpDownSpawnPosY.ValueChanged += new System.EventHandler(this.numericUpDownSpawnPosY_ValueChanged);
            // 
            // numericUpDownSpawnPosZ
            // 
            this.numericUpDownSpawnPosZ.DecimalPlaces = 4;
            this.numericUpDownSpawnPosZ.Location = new System.Drawing.Point(265, 3);
            this.numericUpDownSpawnPosZ.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.numericUpDownSpawnPosZ.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.numericUpDownSpawnPosZ.Name = "numericUpDownSpawnPosZ";
            this.numericUpDownSpawnPosZ.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownSpawnPosZ.TabIndex = 14;
            this.numericUpDownSpawnPosZ.ValueChanged += new System.EventHandler(this.numericUpDownSpawnPosZ_ValueChanged);
            // 
            // EditorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(2544, 1401);
            this.Controls.Add(this.groupBoxSpawn);
            this.Controls.Add(this.treeViewActionList);
            this.Controls.Add(this.groupBoxLight);
            this.Controls.Add(this.groupBoxNavMesh);
            this.Controls.Add(this.toolStrip1);
            this.Controls.Add(this.treeView_entities);
            this.Controls.Add(this.panel_render);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "EditorForm";
            this.Text = "vxEditor";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.Form1_KeyDown);
            this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.Form1_KeyPress);
            this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Form1_KeyUp);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownNavmeshPositionX)).EndInit();
            this.flowLayoutPanel4.ResumeLayout(false);
            this.flowLayoutPanel4.PerformLayout();
            this.groupBoxNavMesh.ResumeLayout(false);
            this.groupBoxLight.ResumeLayout(false);
            this.flowLayoutPanel10.ResumeLayout(false);
            this.flowLayoutPanel10.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightFalloff)).EndInit();
            this.flowLayoutPanel5.ResumeLayout(false);
            this.flowLayoutPanel5.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightZ)).EndInit();
            this.flowLayoutPanel11.ResumeLayout(false);
            this.flowLayoutPanel11.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLightLumen)).EndInit();
            this.groupBoxSpawn.ResumeLayout(false);
            this.groupBoxSpawn.PerformLayout();
            this.flowLayoutPanel13.ResumeLayout(false);
            this.flowLayoutPanel13.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnType)).EndInit();
            this.flowLayoutPanel12.ResumeLayout(false);
            this.flowLayoutPanel12.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosZ)).EndInit();
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
        private System.Windows.Forms.ToolStripButton toolStripButtonCreateMeshInstance;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem undoToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem redoToolStripMenuItem;
        private System.Windows.Forms.TreeView treeViewActionList;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel10;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.NumericUpDown numericUpDownLightFalloff;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel11;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.NumericUpDown numericUpDownLightLumen;
        private System.Windows.Forms.ToolStripButton toolStripButtonCreateSpawn;
        private System.Windows.Forms.GroupBox groupBoxSpawn;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel12;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.NumericUpDown numericUpDownSpawnPosX;
        private System.Windows.Forms.NumericUpDown numericUpDownSpawnPosY;
        private System.Windows.Forms.NumericUpDown numericUpDownSpawnPosZ;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel13;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.NumericUpDown numericUpDownSpawnType;
        private System.Windows.Forms.ToolStripButton toolStripButtonCreateJoint;
        private System.Windows.Forms.ToolStripMenuItem removeJointToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addJointToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem createActorToolStripMenuItem;
        private System.Windows.Forms.ComboBox comboBoxActor;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ToolStripButton toolStripButtonCreateLightGeometryProxy;
    }
}

