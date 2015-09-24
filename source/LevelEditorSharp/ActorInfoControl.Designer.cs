namespace LevelEditor
{
    partial class ActorInfoControl
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
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxActorName = new System.Windows.Forms.TextBox();
            this.comboBoxMesh = new System.Windows.Forms.ComboBox();
            this.comboBoxMaterial = new System.Windows.Forms.ComboBox();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(60, 60);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(38, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Name:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(62, 97);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(36, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Mesh:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(60, 129);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(47, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Material:";
            // 
            // textBoxActorName
            // 
            this.textBoxActorName.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.textBoxActorName.Location = new System.Drawing.Point(104, 57);
            this.textBoxActorName.MaxLength = 31;
            this.textBoxActorName.Name = "textBoxActorName";
            this.textBoxActorName.ReadOnly = true;
            this.textBoxActorName.Size = new System.Drawing.Size(128, 20);
            this.textBoxActorName.TabIndex = 3;
            // 
            // comboBoxMesh
            // 
            this.comboBoxMesh.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxMesh.FormattingEnabled = true;
            this.comboBoxMesh.Location = new System.Drawing.Point(104, 94);
            this.comboBoxMesh.Name = "comboBoxMesh";
            this.comboBoxMesh.Size = new System.Drawing.Size(121, 21);
            this.comboBoxMesh.TabIndex = 4;
            // 
            // comboBoxMaterial
            // 
            this.comboBoxMaterial.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxMaterial.FormattingEnabled = true;
            this.comboBoxMaterial.Location = new System.Drawing.Point(113, 126);
            this.comboBoxMaterial.Name = "comboBoxMaterial";
            this.comboBoxMaterial.Size = new System.Drawing.Size(121, 21);
            this.comboBoxMaterial.TabIndex = 5;
            // 
            // ActorInfoControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.comboBoxMaterial);
            this.Controls.Add(this.comboBoxMesh);
            this.Controls.Add(this.textBoxActorName);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "ActorInfoControl";
            this.Size = new System.Drawing.Size(319, 272);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxActorName;
        private System.Windows.Forms.ComboBox comboBoxMesh;
        private System.Windows.Forms.ComboBox comboBoxMaterial;
    }
}
