namespace LevelEditor
{
    partial class MeshInstanceDataControl
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.groupBoxMesh = new System.Windows.Forms.GroupBox();
            this.flowLayoutPanel8 = new System.Windows.Forms.FlowLayoutPanel();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.translationZ = new System.Windows.Forms.NumericUpDown();
            this.translationY = new System.Windows.Forms.NumericUpDown();
            this.translationX = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.flowLayoutPanel2 = new System.Windows.Forms.FlowLayoutPanel();
            this.rotationZ = new System.Windows.Forms.NumericUpDown();
            this.rotationY = new System.Windows.Forms.NumericUpDown();
            this.rotationX = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.flowLayoutPanel3 = new System.Windows.Forms.FlowLayoutPanel();
            this.scaleZ = new System.Windows.Forms.NumericUpDown();
            this.scaleY = new System.Windows.Forms.NumericUpDown();
            this.scaleX = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.flowLayoutPanel6 = new System.Windows.Forms.FlowLayoutPanel();
            this.texboxName = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.flowLayoutPanel9 = new System.Windows.Forms.FlowLayoutPanel();
            this.comboBoxMesh = new System.Windows.Forms.ComboBox();
            this.label8 = new System.Windows.Forms.Label();
            this.flowLayoutPanel7 = new System.Windows.Forms.FlowLayoutPanel();
            this.comboBoxMaterial = new System.Windows.Forms.ComboBox();
            this.label7 = new System.Windows.Forms.Label();
            this.flowLayoutPanel14 = new System.Windows.Forms.FlowLayoutPanel();
            this.comboBoxAnimation = new System.Windows.Forms.ComboBox();
            this.label13 = new System.Windows.Forms.Label();
            this.flowLayoutPanel4 = new System.Windows.Forms.FlowLayoutPanel();
            this.comboBoxRigidBody = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBoxMesh.SuspendLayout();
            this.flowLayoutPanel8.SuspendLayout();
            this.flowLayoutPanel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.translationZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.translationY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.translationX)).BeginInit();
            this.flowLayoutPanel2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.rotationZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.rotationY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.rotationX)).BeginInit();
            this.flowLayoutPanel3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scaleZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.scaleY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.scaleX)).BeginInit();
            this.flowLayoutPanel6.SuspendLayout();
            this.flowLayoutPanel9.SuspendLayout();
            this.flowLayoutPanel7.SuspendLayout();
            this.flowLayoutPanel14.SuspendLayout();
            this.flowLayoutPanel4.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBoxMesh
            // 
            this.groupBoxMesh.Controls.Add(this.flowLayoutPanel8);
            this.groupBoxMesh.Location = new System.Drawing.Point(3, 3);
            this.groupBoxMesh.Name = "groupBoxMesh";
            this.groupBoxMesh.Size = new System.Drawing.Size(419, 307);
            this.groupBoxMesh.TabIndex = 11;
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
            this.flowLayoutPanel8.Controls.Add(this.flowLayoutPanel14);
            this.flowLayoutPanel8.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.flowLayoutPanel8.Location = new System.Drawing.Point(13, 19);
            this.flowLayoutPanel8.Name = "flowLayoutPanel8";
            this.flowLayoutPanel8.Size = new System.Drawing.Size(391, 279);
            this.flowLayoutPanel8.TabIndex = 18;
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.AutoSize = true;
            this.flowLayoutPanel1.Controls.Add(this.translationZ);
            this.flowLayoutPanel1.Controls.Add(this.translationY);
            this.flowLayoutPanel1.Controls.Add(this.translationX);
            this.flowLayoutPanel1.Controls.Add(this.label1);
            this.flowLayoutPanel1.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel1.Location = new System.Drawing.Point(3, 3);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(375, 26);
            this.flowLayoutPanel1.TabIndex = 7;
            // 
            // translationZ
            // 
            this.translationZ.DecimalPlaces = 6;
            this.translationZ.Location = new System.Drawing.Point(272, 3);
            this.translationZ.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.translationZ.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.translationZ.Name = "translationZ";
            this.translationZ.Size = new System.Drawing.Size(100, 20);
            this.translationZ.TabIndex = 5;
            this.translationZ.ValueChanged += new System.EventHandler(this.translationZ_ValueChanged);
            // 
            // translationY
            // 
            this.translationY.DecimalPlaces = 6;
            this.translationY.Location = new System.Drawing.Point(166, 3);
            this.translationY.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.translationY.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.translationY.Name = "translationY";
            this.translationY.Size = new System.Drawing.Size(100, 20);
            this.translationY.TabIndex = 4;
            this.translationY.ValueChanged += new System.EventHandler(this.translationY_ValueChanged);
            // 
            // translationX
            // 
            this.translationX.DecimalPlaces = 6;
            this.translationX.Location = new System.Drawing.Point(60, 3);
            this.translationX.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.translationX.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.translationX.Name = "translationX";
            this.translationX.Size = new System.Drawing.Size(100, 20);
            this.translationX.TabIndex = 3;
            this.translationX.ValueChanged += new System.EventHandler(this.translationX_ValueChanged);
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
            this.flowLayoutPanel2.Controls.Add(this.rotationZ);
            this.flowLayoutPanel2.Controls.Add(this.rotationY);
            this.flowLayoutPanel2.Controls.Add(this.rotationX);
            this.flowLayoutPanel2.Controls.Add(this.label2);
            this.flowLayoutPanel2.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel2.Location = new System.Drawing.Point(3, 35);
            this.flowLayoutPanel2.Name = "flowLayoutPanel2";
            this.flowLayoutPanel2.Size = new System.Drawing.Size(363, 26);
            this.flowLayoutPanel2.TabIndex = 9;
            // 
            // rotationZ
            // 
            this.rotationZ.DecimalPlaces = 4;
            this.rotationZ.Location = new System.Drawing.Point(260, 3);
            this.rotationZ.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.rotationZ.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.rotationZ.Name = "rotationZ";
            this.rotationZ.Size = new System.Drawing.Size(100, 20);
            this.rotationZ.TabIndex = 5;
            this.rotationZ.ValueChanged += new System.EventHandler(this.rotationZ_ValueChanged);
            // 
            // rotationY
            // 
            this.rotationY.DecimalPlaces = 4;
            this.rotationY.Location = new System.Drawing.Point(154, 3);
            this.rotationY.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.rotationY.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.rotationY.Name = "rotationY";
            this.rotationY.Size = new System.Drawing.Size(100, 20);
            this.rotationY.TabIndex = 4;
            this.rotationY.ValueChanged += new System.EventHandler(this.rotationY_ValueChanged);
            // 
            // rotationX
            // 
            this.rotationX.DecimalPlaces = 4;
            this.rotationX.Location = new System.Drawing.Point(48, 3);
            this.rotationX.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.rotationX.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.rotationX.Name = "rotationX";
            this.rotationX.Size = new System.Drawing.Size(100, 20);
            this.rotationX.TabIndex = 3;
            this.rotationX.ValueChanged += new System.EventHandler(this.rotationX_ValueChanged);
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
            this.flowLayoutPanel3.Controls.Add(this.scaleZ);
            this.flowLayoutPanel3.Controls.Add(this.scaleY);
            this.flowLayoutPanel3.Controls.Add(this.scaleX);
            this.flowLayoutPanel3.Controls.Add(this.label3);
            this.flowLayoutPanel3.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel3.Location = new System.Drawing.Point(3, 67);
            this.flowLayoutPanel3.Name = "flowLayoutPanel3";
            this.flowLayoutPanel3.Size = new System.Drawing.Size(358, 26);
            this.flowLayoutPanel3.TabIndex = 9;
            // 
            // scaleZ
            // 
            this.scaleZ.DecimalPlaces = 4;
            this.scaleZ.Location = new System.Drawing.Point(255, 3);
            this.scaleZ.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.scaleZ.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.scaleZ.Name = "scaleZ";
            this.scaleZ.Size = new System.Drawing.Size(100, 20);
            this.scaleZ.TabIndex = 5;
            this.scaleZ.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // scaleY
            // 
            this.scaleY.DecimalPlaces = 4;
            this.scaleY.Location = new System.Drawing.Point(149, 3);
            this.scaleY.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.scaleY.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.scaleY.Name = "scaleY";
            this.scaleY.Size = new System.Drawing.Size(100, 20);
            this.scaleY.TabIndex = 4;
            this.scaleY.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // scaleX
            // 
            this.scaleX.DecimalPlaces = 4;
            this.scaleX.Location = new System.Drawing.Point(43, 3);
            this.scaleX.Maximum = new decimal(new int[] {
            50000,
            0,
            0,
            0});
            this.scaleX.Minimum = new decimal(new int[] {
            50000,
            0,
            0,
            -2147483648});
            this.scaleX.Name = "scaleX";
            this.scaleX.Size = new System.Drawing.Size(100, 20);
            this.scaleX.TabIndex = 3;
            this.scaleX.Value = new decimal(new int[] {
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
            // flowLayoutPanel6
            // 
            this.flowLayoutPanel6.AutoSize = true;
            this.flowLayoutPanel6.Controls.Add(this.texboxName);
            this.flowLayoutPanel6.Controls.Add(this.label6);
            this.flowLayoutPanel6.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel6.Location = new System.Drawing.Point(3, 99);
            this.flowLayoutPanel6.Name = "flowLayoutPanel6";
            this.flowLayoutPanel6.Size = new System.Drawing.Size(185, 26);
            this.flowLayoutPanel6.TabIndex = 19;
            // 
            // texboxName
            // 
            this.texboxName.Enabled = false;
            this.texboxName.Location = new System.Drawing.Point(47, 3);
            this.texboxName.MaxLength = 31;
            this.texboxName.Name = "texboxName";
            this.texboxName.Size = new System.Drawing.Size(135, 20);
            this.texboxName.TabIndex = 9;
            this.texboxName.TextChanged += new System.EventHandler(this.texboxName_TextChanged);
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
            this.flowLayoutPanel9.Controls.Add(this.comboBoxMesh);
            this.flowLayoutPanel9.Controls.Add(this.label8);
            this.flowLayoutPanel9.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel9.Location = new System.Drawing.Point(3, 131);
            this.flowLayoutPanel9.Name = "flowLayoutPanel9";
            this.flowLayoutPanel9.Size = new System.Drawing.Size(252, 27);
            this.flowLayoutPanel9.TabIndex = 21;
            // 
            // comboBoxMesh
            // 
            this.comboBoxMesh.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxMesh.FormattingEnabled = true;
            this.comboBoxMesh.Location = new System.Drawing.Point(45, 3);
            this.comboBoxMesh.Name = "comboBoxMesh";
            this.comboBoxMesh.Size = new System.Drawing.Size(204, 21);
            this.comboBoxMesh.TabIndex = 18;
            this.comboBoxMesh.SelectedIndexChanged += new System.EventHandler(this.comboBoxMesh_SelectedIndexChanged);
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
            this.flowLayoutPanel7.Controls.Add(this.comboBoxMaterial);
            this.flowLayoutPanel7.Controls.Add(this.label7);
            this.flowLayoutPanel7.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel7.Location = new System.Drawing.Point(3, 164);
            this.flowLayoutPanel7.Name = "flowLayoutPanel7";
            this.flowLayoutPanel7.Size = new System.Drawing.Size(252, 27);
            this.flowLayoutPanel7.TabIndex = 20;
            // 
            // comboBoxMaterial
            // 
            this.comboBoxMaterial.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxMaterial.FormattingEnabled = true;
            this.comboBoxMaterial.Location = new System.Drawing.Point(53, 3);
            this.comboBoxMaterial.Name = "comboBoxMaterial";
            this.comboBoxMaterial.Size = new System.Drawing.Size(196, 21);
            this.comboBoxMaterial.TabIndex = 18;
            this.comboBoxMaterial.SelectedIndexChanged += new System.EventHandler(this.comboBoxMaterial_SelectedIndexChanged);
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
            // flowLayoutPanel14
            // 
            this.flowLayoutPanel14.AutoSize = true;
            this.flowLayoutPanel14.Controls.Add(this.comboBoxAnimation);
            this.flowLayoutPanel14.Controls.Add(this.label13);
            this.flowLayoutPanel14.Controls.Add(this.flowLayoutPanel4);
            this.flowLayoutPanel14.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel14.Location = new System.Drawing.Point(3, 197);
            this.flowLayoutPanel14.Name = "flowLayoutPanel14";
            this.flowLayoutPanel14.Size = new System.Drawing.Size(275, 60);
            this.flowLayoutPanel14.TabIndex = 21;
            // 
            // comboBoxAnimation
            // 
            this.comboBoxAnimation.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxAnimation.FormattingEnabled = true;
            this.comboBoxAnimation.Location = new System.Drawing.Point(76, 3);
            this.comboBoxAnimation.Name = "comboBoxAnimation";
            this.comboBoxAnimation.Size = new System.Drawing.Size(196, 21);
            this.comboBoxAnimation.TabIndex = 18;
            this.comboBoxAnimation.SelectedIndexChanged += new System.EventHandler(this.comboBoxAnimation_SelectedIndexChanged);
            // 
            // label13
            // 
            this.label13.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(14, 0);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(56, 27);
            this.label13.TabIndex = 8;
            this.label13.Text = "Animation:";
            this.label13.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // flowLayoutPanel4
            // 
            this.flowLayoutPanel4.AutoSize = true;
            this.flowLayoutPanel4.Controls.Add(this.comboBoxRigidBody);
            this.flowLayoutPanel4.Controls.Add(this.label4);
            this.flowLayoutPanel4.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.flowLayoutPanel4.Location = new System.Drawing.Point(3, 30);
            this.flowLayoutPanel4.Name = "flowLayoutPanel4";
            this.flowLayoutPanel4.Size = new System.Drawing.Size(269, 27);
            this.flowLayoutPanel4.TabIndex = 22;
            // 
            // comboBoxRigidBody
            // 
            this.comboBoxRigidBody.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxRigidBody.FormattingEnabled = true;
            this.comboBoxRigidBody.Location = new System.Drawing.Point(70, 3);
            this.comboBoxRigidBody.Name = "comboBoxRigidBody";
            this.comboBoxRigidBody.Size = new System.Drawing.Size(196, 21);
            this.comboBoxRigidBody.TabIndex = 18;
            this.comboBoxRigidBody.SelectedIndexChanged += new System.EventHandler(this.comboBoxRigidBody_SelectedIndexChanged);
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(3, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(61, 27);
            this.label4.TabIndex = 8;
            this.label4.Text = "Rigid Body:";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // MeshInstanceDataControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBoxMesh);
            this.Name = "MeshInstanceDataControl";
            this.Size = new System.Drawing.Size(427, 319);
            this.groupBoxMesh.ResumeLayout(false);
            this.flowLayoutPanel8.ResumeLayout(false);
            this.flowLayoutPanel8.PerformLayout();
            this.flowLayoutPanel1.ResumeLayout(false);
            this.flowLayoutPanel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.translationZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.translationY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.translationX)).EndInit();
            this.flowLayoutPanel2.ResumeLayout(false);
            this.flowLayoutPanel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.rotationZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.rotationY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.rotationX)).EndInit();
            this.flowLayoutPanel3.ResumeLayout(false);
            this.flowLayoutPanel3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scaleZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.scaleY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.scaleX)).EndInit();
            this.flowLayoutPanel6.ResumeLayout(false);
            this.flowLayoutPanel6.PerformLayout();
            this.flowLayoutPanel9.ResumeLayout(false);
            this.flowLayoutPanel9.PerformLayout();
            this.flowLayoutPanel7.ResumeLayout(false);
            this.flowLayoutPanel7.PerformLayout();
            this.flowLayoutPanel14.ResumeLayout(false);
            this.flowLayoutPanel14.PerformLayout();
            this.flowLayoutPanel4.ResumeLayout(false);
            this.flowLayoutPanel4.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBoxMesh;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel8;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.NumericUpDown translationZ;
        private System.Windows.Forms.NumericUpDown translationY;
        private System.Windows.Forms.NumericUpDown translationX;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel2;
        private System.Windows.Forms.NumericUpDown rotationZ;
        private System.Windows.Forms.NumericUpDown rotationY;
        private System.Windows.Forms.NumericUpDown rotationX;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel3;
        private System.Windows.Forms.NumericUpDown scaleZ;
        private System.Windows.Forms.NumericUpDown scaleY;
        private System.Windows.Forms.NumericUpDown scaleX;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel6;
        private System.Windows.Forms.TextBox texboxName;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel9;
        private System.Windows.Forms.ComboBox comboBoxMesh;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel7;
        private System.Windows.Forms.ComboBox comboBoxMaterial;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel14;
        private System.Windows.Forms.ComboBox comboBoxAnimation;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel4;
        private System.Windows.Forms.ComboBox comboBoxRigidBody;
        private System.Windows.Forms.Label label4;
    }
}
