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
    partial class CreateMeshInstanceForm
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
            this.comboBox_meshes = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.bindingSource1 = new System.Windows.Forms.BindingSource(this.components);
            this.comboBox_materials = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.button_create = new System.Windows.Forms.Button();
            this.button_cancel = new System.Windows.Forms.Button();
            this.textBox_name = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.numericUpDown_translation_x = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown_translation_y = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown_translation_z = new System.Windows.Forms.NumericUpDown();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            ((System.ComponentModel.ISupportInitialize)(this.bindingSource1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_x)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_y)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_z)).BeginInit();
            this.flowLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // comboBox_meshes
            // 
            this.comboBox_meshes.FormattingEnabled = true;
            this.comboBox_meshes.Location = new System.Drawing.Point(85, 45);
            this.comboBox_meshes.Name = "comboBox_meshes";
            this.comboBox_meshes.Size = new System.Drawing.Size(121, 21);
            this.comboBox_meshes.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(46, 48);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(36, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Mesh:";
            // 
            // comboBox_materials
            // 
            this.comboBox_materials.FormattingEnabled = true;
            this.comboBox_materials.Location = new System.Drawing.Point(85, 81);
            this.comboBox_materials.Name = "comboBox_materials";
            this.comboBox_materials.Size = new System.Drawing.Size(121, 21);
            this.comboBox_materials.TabIndex = 2;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(35, 89);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(47, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Material:";
            // 
            // button_create
            // 
            this.button_create.Location = new System.Drawing.Point(49, 315);
            this.button_create.Name = "button_create";
            this.button_create.Size = new System.Drawing.Size(75, 23);
            this.button_create.TabIndex = 4;
            this.button_create.Text = "Create";
            this.button_create.UseVisualStyleBackColor = true;
            this.button_create.Click += new System.EventHandler(this.button_create_Click);
            // 
            // button_cancel
            // 
            this.button_cancel.Location = new System.Drawing.Point(180, 314);
            this.button_cancel.Name = "button_cancel";
            this.button_cancel.Size = new System.Drawing.Size(75, 23);
            this.button_cancel.TabIndex = 5;
            this.button_cancel.Text = "Cancel";
            this.button_cancel.UseVisualStyleBackColor = true;
            this.button_cancel.Click += new System.EventHandler(this.button_cancel_Click);
            // 
            // textBox_name
            // 
            this.textBox_name.Location = new System.Drawing.Point(97, 128);
            this.textBox_name.Name = "textBox_name";
            this.textBox_name.Size = new System.Drawing.Size(100, 20);
            this.textBox_name.TabIndex = 6;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(53, 131);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(38, 13);
            this.label3.TabIndex = 7;
            this.label3.Text = "Name:";
            // 
            // numericUpDown_translation_x
            // 
            this.numericUpDown_translation_x.Location = new System.Drawing.Point(3, 3);
            this.numericUpDown_translation_x.Name = "numericUpDown_translation_x";
            this.numericUpDown_translation_x.Size = new System.Drawing.Size(79, 20);
            this.numericUpDown_translation_x.TabIndex = 8;
            // 
            // numericUpDown_translation_y
            // 
            this.numericUpDown_translation_y.Location = new System.Drawing.Point(88, 3);
            this.numericUpDown_translation_y.Name = "numericUpDown_translation_y";
            this.numericUpDown_translation_y.Size = new System.Drawing.Size(79, 20);
            this.numericUpDown_translation_y.TabIndex = 9;
            // 
            // numericUpDown_translation_z
            // 
            this.numericUpDown_translation_z.Location = new System.Drawing.Point(173, 3);
            this.numericUpDown_translation_z.Name = "numericUpDown_translation_z";
            this.numericUpDown_translation_z.Size = new System.Drawing.Size(79, 20);
            this.numericUpDown_translation_z.TabIndex = 10;
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.Controls.Add(this.numericUpDown_translation_x);
            this.flowLayoutPanel1.Controls.Add(this.numericUpDown_translation_y);
            this.flowLayoutPanel1.Controls.Add(this.numericUpDown_translation_z);
            this.flowLayoutPanel1.Location = new System.Drawing.Point(67, 185);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(263, 40);
            this.flowLayoutPanel1.TabIndex = 11;
            // 
            // CreateMeshInstanceForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(455, 410);
            this.Controls.Add(this.flowLayoutPanel1);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.textBox_name);
            this.Controls.Add(this.button_cancel);
            this.Controls.Add(this.button_create);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.comboBox_materials);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.comboBox_meshes);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "CreateMeshInstanceForm";
            this.Text = "CreateMeshInstanceForm";
            ((System.ComponentModel.ISupportInitialize)(this.bindingSource1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_x)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_y)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_translation_z)).EndInit();
            this.flowLayoutPanel1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox comboBox_meshes;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.BindingSource bindingSource1;
        private System.Windows.Forms.ComboBox comboBox_materials;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button button_create;
        private System.Windows.Forms.Button button_cancel;
        private System.Windows.Forms.TextBox textBox_name;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown numericUpDown_translation_x;
        private System.Windows.Forms.NumericUpDown numericUpDown_translation_y;
        private System.Windows.Forms.NumericUpDown numericUpDown_translation_z;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
    }
}