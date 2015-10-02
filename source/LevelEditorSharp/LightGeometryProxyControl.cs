using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LevelEditor
{
    public partial class LightGeometryProxyControl : UserControl
    {
        EditorForm m_parent;
        uint m_selectedIndex;

        public LightGeometryProxyControl(EditorForm parent)
        {
            InitializeComponent();

            m_parent = parent;
            m_selectedIndex = 0xffffffff;
        }

        public void setSelectedIndex(uint index)
        {
            m_selectedIndex = index;
        }

        public void getData()
        {
            Float3 center;
            Float3 halfDim;

            NativeMethods.getLightGeometryProxyBounds(m_selectedIndex, out center, out halfDim);

            setControlCenter(center);
            setControlHalfdim(halfDim);
        }

        void setBounds()
        {
            Float3 center;
            center.x = (float)numericUpDownCenterX.Value;
            center.y = (float)numericUpDownCenterY.Value;
            center.z = (float)numericUpDownCenterZ.Value;

            Float3 halfDim;
            halfDim.x = (float)numericUpDownHalfdimX.Value;
            halfDim.y = (float)numericUpDownHalfdimY.Value;
            halfDim.z = (float)numericUpDownHalfdimZ.Value;

            NativeMethods.setLightGeometryProxyBounds(m_selectedIndex, ref center, ref halfDim);
        }

        void setControlCenter(Float3 p)
        {
            numericUpDownCenterX.Value = (decimal)p.x;
            numericUpDownCenterY.Value = (decimal)p.y;
            numericUpDownCenterZ.Value = (decimal)p.z;
        }

        void setControlHalfdim(Float3 hd)
        {
            numericUpDownHalfdimX.Value = (decimal)hd.x;
            numericUpDownHalfdimY.Value = (decimal)hd.y;
            numericUpDownHalfdimZ.Value = (decimal)hd.z;
        }

        private void numericUpDownCenterX_ValueChanged(object sender, EventArgs e)
        {
            setBounds();
        }

        private void numericUpDownCenterY_ValueChanged(object sender, EventArgs e)
        {
            setBounds();
        }

        private void numericUpDownCenterZ_ValueChanged(object sender, EventArgs e)
        {
            setBounds();
        }

        private void numericUpDownHalfdimX_ValueChanged(object sender, EventArgs e)
        {
            setBounds();
        }

        private void numericUpDownHalfdimY_ValueChanged(object sender, EventArgs e)
        {
            setBounds();
        }

        private void numericUpDownHalfdimZ_ValueChanged(object sender, EventArgs e)
        {
            setBounds();
        }
    }
}
