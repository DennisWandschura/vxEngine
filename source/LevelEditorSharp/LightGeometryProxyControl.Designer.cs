namespace LevelEditor
{
    partial class LightGeometryProxyControl
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
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.numericUpDownCenterX = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownCenterY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownCenterZ = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownHalfdimZ = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownHalfdimY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownHalfdimX = new System.Windows.Forms.NumericUpDown();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownCenterX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownCenterY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownCenterZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownHalfdimZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownHalfdimY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownHalfdimX)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 41);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(38, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Center";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 158);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(42, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Halfdim";
            // 
            // numericUpDownCenterX
            // 
            this.numericUpDownCenterX.DecimalPlaces = 4;
            this.numericUpDownCenterX.Location = new System.Drawing.Point(57, 39);
            this.numericUpDownCenterX.Maximum = new decimal(new int[] {
            99999,
            0,
            0,
            0});
            this.numericUpDownCenterX.Minimum = new decimal(new int[] {
            99999,
            0,
            0,
            -2147483648});
            this.numericUpDownCenterX.Name = "numericUpDownCenterX";
            this.numericUpDownCenterX.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownCenterX.TabIndex = 3;
            this.numericUpDownCenterX.ValueChanged += new System.EventHandler(this.numericUpDownCenterX_ValueChanged);
            // 
            // numericUpDownCenterY
            // 
            this.numericUpDownCenterY.DecimalPlaces = 4;
            this.numericUpDownCenterY.Location = new System.Drawing.Point(183, 39);
            this.numericUpDownCenterY.Maximum = new decimal(new int[] {
            99999,
            0,
            0,
            0});
            this.numericUpDownCenterY.Minimum = new decimal(new int[] {
            99999,
            0,
            0,
            -2147483648});
            this.numericUpDownCenterY.Name = "numericUpDownCenterY";
            this.numericUpDownCenterY.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownCenterY.TabIndex = 8;
            this.numericUpDownCenterY.ValueChanged += new System.EventHandler(this.numericUpDownCenterY_ValueChanged);
            // 
            // numericUpDownCenterZ
            // 
            this.numericUpDownCenterZ.DecimalPlaces = 4;
            this.numericUpDownCenterZ.Location = new System.Drawing.Point(309, 39);
            this.numericUpDownCenterZ.Maximum = new decimal(new int[] {
            99999,
            0,
            0,
            0});
            this.numericUpDownCenterZ.Minimum = new decimal(new int[] {
            99999,
            0,
            0,
            -2147483648});
            this.numericUpDownCenterZ.Name = "numericUpDownCenterZ";
            this.numericUpDownCenterZ.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownCenterZ.TabIndex = 9;
            this.numericUpDownCenterZ.ValueChanged += new System.EventHandler(this.numericUpDownCenterZ_ValueChanged);
            // 
            // numericUpDownHalfdimZ
            // 
            this.numericUpDownHalfdimZ.DecimalPlaces = 4;
            this.numericUpDownHalfdimZ.Location = new System.Drawing.Point(313, 158);
            this.numericUpDownHalfdimZ.Maximum = new decimal(new int[] {
            99999,
            0,
            0,
            0});
            this.numericUpDownHalfdimZ.Minimum = new decimal(new int[] {
            99999,
            0,
            0,
            -2147483648});
            this.numericUpDownHalfdimZ.Name = "numericUpDownHalfdimZ";
            this.numericUpDownHalfdimZ.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownHalfdimZ.TabIndex = 12;
            this.numericUpDownHalfdimZ.ValueChanged += new System.EventHandler(this.numericUpDownHalfdimZ_ValueChanged);
            // 
            // numericUpDownHalfdimY
            // 
            this.numericUpDownHalfdimY.DecimalPlaces = 4;
            this.numericUpDownHalfdimY.Location = new System.Drawing.Point(187, 158);
            this.numericUpDownHalfdimY.Maximum = new decimal(new int[] {
            99999,
            0,
            0,
            0});
            this.numericUpDownHalfdimY.Minimum = new decimal(new int[] {
            99999,
            0,
            0,
            -2147483648});
            this.numericUpDownHalfdimY.Name = "numericUpDownHalfdimY";
            this.numericUpDownHalfdimY.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownHalfdimY.TabIndex = 11;
            this.numericUpDownHalfdimY.ValueChanged += new System.EventHandler(this.numericUpDownHalfdimY_ValueChanged);
            // 
            // numericUpDownHalfdimX
            // 
            this.numericUpDownHalfdimX.DecimalPlaces = 4;
            this.numericUpDownHalfdimX.Location = new System.Drawing.Point(61, 158);
            this.numericUpDownHalfdimX.Maximum = new decimal(new int[] {
            99999,
            0,
            0,
            0});
            this.numericUpDownHalfdimX.Minimum = new decimal(new int[] {
            99999,
            0,
            0,
            -2147483648});
            this.numericUpDownHalfdimX.Name = "numericUpDownHalfdimX";
            this.numericUpDownHalfdimX.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownHalfdimX.TabIndex = 10;
            this.numericUpDownHalfdimX.ValueChanged += new System.EventHandler(this.numericUpDownHalfdimX_ValueChanged);
            // 
            // LightGeometryProxyControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.numericUpDownHalfdimZ);
            this.Controls.Add(this.numericUpDownHalfdimY);
            this.Controls.Add(this.numericUpDownHalfdimX);
            this.Controls.Add(this.numericUpDownCenterZ);
            this.Controls.Add(this.numericUpDownCenterY);
            this.Controls.Add(this.numericUpDownCenterX);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "LightGeometryProxyControl";
            this.Size = new System.Drawing.Size(450, 229);
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownCenterX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownCenterY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownCenterZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownHalfdimZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownHalfdimY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownHalfdimX)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown numericUpDownCenterX;
        private System.Windows.Forms.NumericUpDown numericUpDownCenterY;
        private System.Windows.Forms.NumericUpDown numericUpDownCenterZ;
        private System.Windows.Forms.NumericUpDown numericUpDownHalfdimZ;
        private System.Windows.Forms.NumericUpDown numericUpDownHalfdimY;
        private System.Windows.Forms.NumericUpDown numericUpDownHalfdimX;
    }
}
