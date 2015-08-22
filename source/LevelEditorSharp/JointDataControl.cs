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
    public partial class JointDataControl : UserControl
    {
        EditorForm m_editorForm;
        Dictionary<ulong, EditorEntry> m_sortedMeshInstances;
        uint m_selectedJoint;
        bool m_selected;

        public JointDataControl(EditorForm editorForm)
        {
            m_editorForm = editorForm;
            m_selectedJoint = 0;
            m_selected = false;

            m_sortedMeshInstances = new Dictionary<ulong, EditorEntry>();

            InitializeComponent();
        }

        public void clearMeshInstances()
        {
            comboBoxInstance0.Items.Clear();
            comboBoxInstance1.Items.Clear();
            m_sortedMeshInstances.Clear();

            var nullEntry = new EditorEntry("null", 0);
            addMeshInstance(nullEntry);
        }

        public void addMeshInstance(EditorEntry entry)
        {
            comboBoxInstance0.Items.Add(entry);
            comboBoxInstance1.Items.Add(entry);

            m_sortedMeshInstances.Add(entry.m_sid, entry);
        }

        public void setPosition0(Float3 p)
        {
            numericUpDownP0X.Value = (decimal)p.x;
            numericUpDownP0Y.Value = (decimal)p.y;
            numericUpDownP0Z.Value = (decimal)p.z;
        }

        public void setPosition1(Float3 p)
        {
            numericUpDownP1X.Value = (decimal)p.x;
            numericUpDownP1Y.Value = (decimal)p.y;
            numericUpDownP1Z.Value = (decimal)p.z;
        }
  
        public void setRotation0(Float3 q)
        {
            numericUpDownQ0X.Value = (decimal)q.x;
            numericUpDownQ0Y.Value = (decimal)q.y;
            numericUpDownQ0Z.Value = (decimal)q.z;
        }

        public void setRotation1(Float3 q)
        {
            numericUpDownQ1X.Value = (decimal)q.x;
            numericUpDownQ1Y.Value = (decimal)q.y;
            numericUpDownQ1Z.Value = (decimal)q.z;
        }

        void setLimitData(uint enabled, float min, float max)
        {
            numericUpDownLimitMin.Value = (decimal)min;
            numericUpDownLimitMax.Value = (decimal)max;

            if (enabled == 0)
                checkBoxLimit.Checked = false;
            else
                checkBoxLimit.Checked = true;
        }

        public void setSelectedJoint(uint index)
        {
            m_selectedJoint = index;

            var p0 = new Float3();
            var p1 = new Float3();
            var q0 = new Float3();
            var q1 = new Float3();
            ulong sid0, sid1;
            uint limitEnabled = 0;
            float limitMin, limitMax;

            NativeMethods.getJointData(index, out p0, out q0, out p1, out q1, out sid0, out sid1, out limitEnabled, out limitMin, out limitMax);
            setPosition0(p0);
            setPosition1(p1);

            setRotation0(q0);
            setRotation1(q1);

            setLimitData(limitEnabled, limitMin, limitMax);

            EditorEntry entry;
            if (m_sortedMeshInstances.TryGetValue(sid0, out entry))
            {
                comboBoxInstance0.SelectedItem = entry;
            }

            if (m_sortedMeshInstances.TryGetValue(sid1, out entry))
            {
                comboBoxInstance1.SelectedItem = entry;
            }

            m_selected = true;
        }

        public void deselect()
        {
            m_selected = false;
        }

        void setJointPosition0()
        {
            if (m_selected)
            {
                var p0 = new Float3();
                p0.x = (float)numericUpDownP0X.Value;
                p0.y = (float)numericUpDownP0Y.Value;
                p0.z = (float)numericUpDownP0Z.Value;

                NativeMethods.setJointPosition0(m_selectedJoint, ref p0);
            }
        }

        void setJointPosition1()
        {
            if (m_selected)
            {
                var p0 = new Float3();
                p0.x = (float)numericUpDownP1X.Value;
                p0.y = (float)numericUpDownP1Y.Value;
                p0.z = (float)numericUpDownP1Z.Value;

                NativeMethods.setJointPosition1(m_selectedJoint, ref p0);
            }
        }

        void setJointRotation0()
        {
            if (m_selected)
            {
                var q0 = new Float3();
                q0.x = (float)numericUpDownQ0X.Value;
                q0.y = (float)numericUpDownQ0Y.Value;
                q0.z = (float)numericUpDownQ0Z.Value;

                NativeMethods.setJointRotation0(m_selectedJoint, ref q0);
            }
        }

        void setJointRotation1()
        {
            if (m_selected)
            {
                var q0 = new Float3();
                q0.x = (float)numericUpDownQ1X.Value;
                q0.y = (float)numericUpDownQ1Y.Value;
                q0.z = (float)numericUpDownQ1Z.Value;

                NativeMethods.setJointRotation1(m_selectedJoint, ref q0);
            }
        }

        void setJointLimitData()
        {
            if (m_selected)
            {
                var checkChecked = checkBoxLimit.Checked;
                uint limitEnabled = 0;
                if (checkChecked)
                {
                    limitEnabled = 1;
                }

                float lmin = (float)numericUpDownLimitMin.Value;
                float lmax = (float)numericUpDownLimitMax.Value;

                if(lmin >= lmax)
                {
                    lmin = lmax - 0.1f;
                }

                NativeMethods.setJointLimit(m_selectedJoint, limitEnabled, lmin, lmax);
            }
        }

        public void removeJoint()
        {
            if (m_selected)
            {
                deselect();
                NativeMethods.removeJoint(m_selectedJoint);
            }
        }

        private void numericUpDownP0X_ValueChanged(object sender, EventArgs e)
        {
            setJointPosition0();
        }

        private void numericUpDownP0Y_ValueChanged(object sender, EventArgs e)
        {
            setJointPosition0();
        }

        private void numericUpDownP0Z_ValueChanged(object sender, EventArgs e)
        {
            setJointPosition0();
        }

        private void numericUpDownP1X_ValueChanged(object sender, EventArgs e)
        {
            setJointPosition1();
        }

        private void numericUpDownP1Y_ValueChanged(object sender, EventArgs e)
        {
            setJointPosition1();
        }

        private void numericUpDownP1Z_ValueChanged(object sender, EventArgs e)
        {
            setJointPosition1();
        }

        private void comboBoxInstance0_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (m_selected)
            {
                var entry = (EditorEntry)comboBoxInstance0.SelectedItem;
                NativeMethods.setJointBody0(m_selectedJoint, entry.m_sid);
            }
        }

        private void comboBoxInstance1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (m_selected)
            {
                var entry = (EditorEntry)comboBoxInstance1.SelectedItem;
                NativeMethods.setJointBody1(m_selectedJoint, entry.m_sid);
            }
        }

        private void numericUpDownQ0X_ValueChanged(object sender, EventArgs e)
        {
            setJointRotation0();
        }

        private void numericUpDownQ0Y_ValueChanged(object sender, EventArgs e)
        {
            setJointRotation0();
        }

        private void numericUpDownQ0Z_ValueChanged(object sender, EventArgs e)
        {
            setJointRotation0();
        }

        private void numericUpDownQ1X_ValueChanged(object sender, EventArgs e)
        {
            setJointRotation1();
        }

        private void numericUpDownQ1Y_ValueChanged(object sender, EventArgs e)
        {
            setJointRotation1();
        }

        private void numericUpDownQ1Z_ValueChanged(object sender, EventArgs e)
        {
            setJointRotation1();
        }

        private void checkBoxLimit_CheckedChanged(object sender, EventArgs e)
        {
            setJointLimitData();
        }

        private void numericUpDownLimitMin_ValueChanged(object sender, EventArgs e)
        {
            setJointLimitData();
        }

        private void numericUpDownLimitMax_ValueChanged(object sender, EventArgs e)
        {
            setJointLimitData();
        }
    }
}
