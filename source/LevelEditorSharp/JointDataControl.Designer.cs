namespace LevelEditor
{
    partial class JointDataControl
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
            this.numericUpDownP0X = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownP0Y = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownP0Z = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownP1Z = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownP1Y = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownP1X = new System.Windows.Forms.NumericUpDown();
            this.comboBoxInstance0 = new System.Windows.Forms.ComboBox();
            this.comboBoxInstance1 = new System.Windows.Forms.ComboBox();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP0X)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP0Y)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP0Z)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP1Z)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP1Y)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP1X)).BeginInit();
            this.SuspendLayout();
            // 
            // numericUpDownP0X
            // 
            this.numericUpDownP0X.DecimalPlaces = 3;
            this.numericUpDownP0X.Location = new System.Drawing.Point(27, 65);
            this.numericUpDownP0X.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownP0X.Minimum = new decimal(new int[] {
            1000,
            0,
            0,
            -2147483648});
            this.numericUpDownP0X.Name = "numericUpDownP0X";
            this.numericUpDownP0X.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownP0X.TabIndex = 0;
            this.numericUpDownP0X.ValueChanged += new System.EventHandler(this.numericUpDownP0X_ValueChanged);
            // 
            // numericUpDownP0Y
            // 
            this.numericUpDownP0Y.DecimalPlaces = 3;
            this.numericUpDownP0Y.Location = new System.Drawing.Point(133, 65);
            this.numericUpDownP0Y.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownP0Y.Minimum = new decimal(new int[] {
            1000,
            0,
            0,
            -2147483648});
            this.numericUpDownP0Y.Name = "numericUpDownP0Y";
            this.numericUpDownP0Y.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownP0Y.TabIndex = 1;
            this.numericUpDownP0Y.ValueChanged += new System.EventHandler(this.numericUpDownP0Y_ValueChanged);
            // 
            // numericUpDownP0Z
            // 
            this.numericUpDownP0Z.DecimalPlaces = 3;
            this.numericUpDownP0Z.Location = new System.Drawing.Point(239, 65);
            this.numericUpDownP0Z.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownP0Z.Minimum = new decimal(new int[] {
            1000,
            0,
            0,
            -2147483648});
            this.numericUpDownP0Z.Name = "numericUpDownP0Z";
            this.numericUpDownP0Z.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownP0Z.TabIndex = 2;
            this.numericUpDownP0Z.ValueChanged += new System.EventHandler(this.numericUpDownP0Z_ValueChanged);
            // 
            // numericUpDownP1Z
            // 
            this.numericUpDownP1Z.DecimalPlaces = 3;
            this.numericUpDownP1Z.Location = new System.Drawing.Point(239, 91);
            this.numericUpDownP1Z.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownP1Z.Minimum = new decimal(new int[] {
            1000,
            0,
            0,
            -2147483648});
            this.numericUpDownP1Z.Name = "numericUpDownP1Z";
            this.numericUpDownP1Z.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownP1Z.TabIndex = 5;
            this.numericUpDownP1Z.ValueChanged += new System.EventHandler(this.numericUpDownP1Z_ValueChanged);
            // 
            // numericUpDownP1Y
            // 
            this.numericUpDownP1Y.DecimalPlaces = 3;
            this.numericUpDownP1Y.Location = new System.Drawing.Point(133, 91);
            this.numericUpDownP1Y.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownP1Y.Minimum = new decimal(new int[] {
            1000,
            0,
            0,
            -2147483648});
            this.numericUpDownP1Y.Name = "numericUpDownP1Y";
            this.numericUpDownP1Y.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownP1Y.TabIndex = 4;
            this.numericUpDownP1Y.ValueChanged += new System.EventHandler(this.numericUpDownP1Y_ValueChanged);
            // 
            // numericUpDownP1X
            // 
            this.numericUpDownP1X.DecimalPlaces = 3;
            this.numericUpDownP1X.Location = new System.Drawing.Point(27, 91);
            this.numericUpDownP1X.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownP1X.Minimum = new decimal(new int[] {
            1000,
            0,
            0,
            -2147483648});
            this.numericUpDownP1X.Name = "numericUpDownP1X";
            this.numericUpDownP1X.Size = new System.Drawing.Size(100, 20);
            this.numericUpDownP1X.TabIndex = 3;
            this.numericUpDownP1X.ValueChanged += new System.EventHandler(this.numericUpDownP1X_ValueChanged);
            // 
            // comboBoxInstance0
            // 
            this.comboBoxInstance0.FormattingEnabled = true;
            this.comboBoxInstance0.Location = new System.Drawing.Point(66, 156);
            this.comboBoxInstance0.Name = "comboBoxInstance0";
            this.comboBoxInstance0.Size = new System.Drawing.Size(121, 21);
            this.comboBoxInstance0.TabIndex = 6;
            this.comboBoxInstance0.SelectedIndexChanged += new System.EventHandler(this.comboBoxInstance0_SelectedIndexChanged);
            // 
            // comboBoxInstance1
            // 
            this.comboBoxInstance1.FormattingEnabled = true;
            this.comboBoxInstance1.Location = new System.Drawing.Point(66, 197);
            this.comboBoxInstance1.Name = "comboBoxInstance1";
            this.comboBoxInstance1.Size = new System.Drawing.Size(121, 21);
            this.comboBoxInstance1.TabIndex = 7;
            this.comboBoxInstance1.SelectedIndexChanged += new System.EventHandler(this.comboBoxInstance1_SelectedIndexChanged);
            // 
            // JointDataControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.comboBoxInstance1);
            this.Controls.Add(this.comboBoxInstance0);
            this.Controls.Add(this.numericUpDownP1Z);
            this.Controls.Add(this.numericUpDownP1Y);
            this.Controls.Add(this.numericUpDownP1X);
            this.Controls.Add(this.numericUpDownP0Z);
            this.Controls.Add(this.numericUpDownP0Y);
            this.Controls.Add(this.numericUpDownP0X);
            this.Name = "JointDataControl";
            this.Size = new System.Drawing.Size(362, 282);
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP0X)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP0Y)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP0Z)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP1Z)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP1Y)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownP1X)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.NumericUpDown numericUpDownP0X;
        private System.Windows.Forms.NumericUpDown numericUpDownP0Y;
        private System.Windows.Forms.NumericUpDown numericUpDownP0Z;
        private System.Windows.Forms.NumericUpDown numericUpDownP1Z;
        private System.Windows.Forms.NumericUpDown numericUpDownP1Y;
        private System.Windows.Forms.NumericUpDown numericUpDownP1X;
        private System.Windows.Forms.ComboBox comboBoxInstance0;
        private System.Windows.Forms.ComboBox comboBoxInstance1;
    }
}
