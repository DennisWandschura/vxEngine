using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LevelEditor
{
    public enum EditorState { EditMesh, EditNavMesh, EditLights, EditSpawns };

    public partial class Form1 : Form
    {
        const string s_textEditMesh = "Edit Mesh";
        const string s_textEditNavMesh = "Edit Nav Mesh";
        const string s_textEditLights = "Edit Lights";
        const string s_textEditSpawns = "Edit Spawns";
        const int s_groupBoxEditPositionX = 1380;
        const int s_groupBoxEditPositionY = 620;

        const UInt32 s_typeMesh = 0;
        const UInt32 s_typeMaterial = 1;
        const UInt32 s_typeMeshInstance = 2;
        const UInt32 s_typeScene = 3;

        static Form1 s_form;

        public delegate void LoadedFileFun(UInt64 sid, UInt32 type);

        public LoadedFileFun loadFileDelegate;
        TreeNode m_meshNode;
        TreeNode m_materialNode;
        TreeNode m_meshInstanceNode;
        TreeNode m_waypointsNode;
        bool m_keyDownAlt;
        int m_mouseX;
        int m_mouseY;
        bool m_isMouseDown;
        bool m_keyDownShift;
        MouseButtons m_lastClickedMouseButton;
        StateMachine m_selectItemStateMachine;
        EditorState m_editorState;
        int m_isMeshInstanceSelected;
        int m_keys;
        bool m_selectedNavMesh;
        string m_currentSceneFileName;

        Dictionary<UInt64, string> m_requestedFiles;

        public Form1()
        {
            InitializeComponent();

            m_meshNode = treeView_entities.Nodes.Add("Meshes");
            m_materialNode = treeView_entities.Nodes.Add("Materials");
            m_meshInstanceNode = treeView_entities.Nodes.Add("Mesh Instances");
            m_waypointsNode = treeView_entities.Nodes.Add("Waypoints");
            treeView_entities.Nodes.Add("Lights");
            m_currentSceneFileName = "untitled.scene";

            groupBoxMesh.Hide();

            m_requestedFiles = new Dictionary<ulong, string>();

            loadFileDelegate = new LoadedFileFun(loadedFile);

            m_keyDownAlt = false;
            m_isMouseDown = false;
            m_keyDownShift = false;
            m_selectedNavMesh = false;
            m_mouseX = 0;
            m_mouseY = 0;
            m_isMeshInstanceSelected = 0;
            m_keys = 0;

            createStateMachine();

            try
            {
                Panel tmp = new Panel();

                NativeMethods.initializeEditor(panel_render.Handle, tmp.Handle, (uint)panel_render.Width, (uint)panel_render.Height, s_typeMesh, s_typeMaterial, s_typeScene);
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString());
                throw;
            }

            s_form = this;

            this.KeyPreview = true;
            panel_render.MouseWheel += panel_render_MouseMove_MouseWheel;

            m_editorState = EditorState.EditMesh;
            comboBox_selectEditorMode.Items.Add(s_textEditMesh);
            comboBox_selectEditorMode.Items.Add(s_textEditNavMesh);
            comboBox_selectEditorMode.Items.Add(s_textEditLights);
            comboBox_selectEditorMode.Items.Add(s_textEditSpawns);
            comboBox_selectEditorMode.SelectedIndex = 0;

            Point p = new Point();
            p.X = s_groupBoxEditPositionX;
            p.Y = s_groupBoxEditPositionY;

            groupBoxNavMesh.Hide();
            groupBoxNavMesh.Location = p;
            groupBoxMesh.Location = p;
        }

        ~Form1()
        {
        }

        private ActionDecisionTree createActionOnMouseClickNavMesh(State stateEditNavMesh, ActionDeselectNavMesh actionDeselectNavMesh)
        {
            ActionSelectNavMesh actionSelectNavMesh = new ActionSelectNavMesh(this);
            ActionMultiSelectNavMesh actionMultiSelectNavMesh = new ActionMultiSelectNavMesh(this);

            TargetState stateSelectNavMeshVertex = new TargetState(stateEditNavMesh);
            stateSelectNavMeshVertex.addAction(actionSelectNavMesh);

            TargetState stateMultiSelectNavMeshVertex = new TargetState(stateEditNavMesh);
            stateMultiSelectNavMeshVertex.addAction(actionMultiSelectNavMesh);

            TargetState stateDeselectNavMeshVertex = new TargetState(stateEditNavMesh);
            stateDeselectNavMeshVertex.addAction(actionDeselectNavMesh);

            TargetState stateCreateVertex= new TargetState(stateEditNavMesh);
            stateCreateVertex.addAction(new ActionCreateNavMeshVertex(this));

            DecisionKeyDown decisionShiftKeyDown = new DecisionKeyDown(stateMultiSelectNavMeshVertex, stateSelectNavMeshVertex, System.Windows.Input.Key.LeftShift);
            DecisionKeyDown decisionCKeyDown = new DecisionKeyDown(stateCreateVertex, decisionShiftKeyDown, System.Windows.Input.Key.C);

            DecisionEditorMouseButtonPressed decisionMouseRightButton = new DecisionEditorMouseButtonPressed(stateDeselectNavMeshVertex, null, this, MouseButtons.Right);
            DecisionEditorMouseButtonPressed decisionMouseLeftButton = new DecisionEditorMouseButtonPressed(decisionCKeyDown, decisionMouseRightButton, this, MouseButtons.Left);

            ActionDecisionTree actionOnMouseClick = new ActionDecisionTree(decisionMouseLeftButton);

            return actionOnMouseClick;
        }

        private State createStateEditNavMesh()
        {
            ActionDeselectNavMesh actionDeselectNavMesh = new ActionDeselectNavMesh(this);

            State stateEditNavMesh = new State();

            var actionOnMouseClick = createActionOnMouseClickNavMesh(stateEditNavMesh, actionDeselectNavMesh);

            ActionCreateNavMeshTriangle actionCreateNavMeshTriangle = new ActionCreateNavMeshTriangle();
            TargetState stateKeyDown = new TargetState(stateEditNavMesh);
            stateKeyDown.addAction(actionCreateNavMeshTriangle);
            DecisionKeyDownNew keyDown = new DecisionKeyDownNew(stateKeyDown, null, this, Keys.C);
            ActionDecisionTree actionKeyDown = new ActionDecisionTree(keyDown);

            //stateEditNavMesh.addAction(actionKeyDown);
            stateEditNavMesh.addAction(actionOnMouseClick);
            stateEditNavMesh.addExitAction(actionDeselectNavMesh);

            return stateEditNavMesh;
        }

        private ActionDecisionTree createActionOnMouseClickMesh(State stateEditMesh, ActionDeselectMesh actionDeselectMesh)
        {
            ActionSelectMesh actionSelectMesh = new ActionSelectMesh(this);

            TargetState stateSelectMesh = new TargetState(stateEditMesh);
            stateSelectMesh.addAction(actionSelectMesh);

            TargetState stateDeselectMesh = new TargetState(stateEditMesh);
            stateDeselectMesh.addAction(actionDeselectMesh);

            DecisionEditorMouseButtonPressed decisionMouseRightButton = new DecisionEditorMouseButtonPressed(stateDeselectMesh, null, this, MouseButtons.Right);
            DecisionEditorMouseButtonPressed decisionMouseLeftButton = new DecisionEditorMouseButtonPressed(stateSelectMesh, decisionMouseRightButton, this, MouseButtons.Left);

            ActionDecisionTree actionOnMouseClick = new ActionDecisionTree(decisionMouseLeftButton);

            return actionOnMouseClick;
        }

        private State createStateEditMesh()
        {
            ActionDeselectMesh actionDeselectMesh = new ActionDeselectMesh(this);

            State stateEditMesh = new State();

            var actionOnMouseClick = createActionOnMouseClickMesh(stateEditMesh, actionDeselectMesh);

            stateEditMesh.addAction(actionOnMouseClick);
            stateEditMesh.addExitAction(actionDeselectMesh);

            return stateEditMesh;
        }

        private void createActionOnKeyboardNavMesh(State stateEditNavMesh)
        {
            ActionCreateNavMeshTriangle actionCreateNavMeshTriangle = new ActionCreateNavMeshTriangle();

            TargetState stateCreateNavMeshTriangle = new TargetState(stateEditNavMesh);
            stateCreateNavMeshTriangle.addAction(actionCreateNavMeshTriangle);
        }

        void createStateMachine()
        {
            m_selectItemStateMachine = new StateMachine();

            ConditionEditorStateEditMesh conditionEditorStateEditMesh = new ConditionEditorStateEditMesh(this);
            ConditionEditorStateEditNavMesh conditionEditorStateEditNavMesh = new ConditionEditorStateEditNavMesh(this);

            var stateEditNavMesh = createStateEditNavMesh();
            var stateEditMesh = createStateEditMesh();

            Transition transitionEditMesh = new Transition(conditionEditorStateEditMesh, stateEditMesh, "transitionEditMesh");
            Transition transitionEditNavMesh = new Transition(conditionEditorStateEditNavMesh, stateEditNavMesh, "transitionEditNavMesh");

            stateEditNavMesh.addTransition(transitionEditMesh);
            stateEditMesh.addTransition(transitionEditNavMesh);

            m_selectItemStateMachine.addState(stateEditNavMesh);
            m_selectItemStateMachine.addState(stateEditMesh);

            m_selectItemStateMachine.setCurrentState(stateEditNavMesh);
        }

        public IntPtr getDisplayPanelHandle()
        {
            return panel_render.Handle;
        }

        static void loadFileCallback(UInt64 sid, UInt32 type)
        {
            if (sid != 0)
            {
                s_form.Invoke(s_form.loadFileDelegate, sid, type);
            }
        }

        void loadedFile(UInt64 sid, UInt32 type)
        {
            string str;
            if (m_requestedFiles.TryGetValue(sid, out str))
            {
                m_requestedFiles.Remove(sid);

                if (type == s_typeMesh)
                {
                    m_meshNode.Nodes.Add(new EditorNodeEntry(sid, s_typeMesh, str));
                }
                else if (type == s_typeMaterial)
                {
                    m_materialNode.Nodes.Add(new EditorNodeEntry(sid, s_typeMaterial, str));
                }
                else
                {
                    Console.Write("loaded unknown file type\n");
                }
            }
        }

        public void addMeshInstance(UInt64 sid, string name)
        {
            m_meshInstanceNode.Nodes.Add(new EditorNodeEntry(sid, s_typeMeshInstance, name));
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void createMeshInstanceToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (m_meshNode.Nodes.Count > 0 && m_materialNode.Nodes.Count > 0)
            {
                CreateMeshInstanceForm createMeshInstanceForm = new CreateMeshInstanceForm(this, m_meshNode, m_materialNode);
                createMeshInstanceForm.Show();
            }
        }

        private void createLightToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void loadMeshToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog1_loadScene.ShowDialog();
        }

        private void loadMaterialToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void openFileDialog_import_FileOk(object sender, CancelEventArgs e)
        {
            string filename = openFileDialog_importAsset.SafeFileName;
            string ext = System.IO.Path.GetExtension(filename);

            UInt64 sid = NativeMethods.getSid(filename);

            if (ext == ".mesh")
            {
                m_requestedFiles.Add(sid, filename);

                NativeMethods.loadFile(filename, s_typeMesh, Form1.loadFileCallback);
            }
            else if (ext == ".material")
            {
                m_requestedFiles.Add(sid, filename);

                NativeMethods.loadFile(filename, s_typeMaterial, Form1.loadFileCallback);
            }
        }

        void updateSelectedMeshInstanceTransform(Float3 translation)
        {
            NativeMethods.updateSelectedMeshInstanceTransform(ref translation);
        }

        void setNumericUpDownTranslation(Float3 translation)
        {
            numericUpDown_translation_x.Value = (decimal)translation.x;
            numericUpDown_translation_y.Value = (decimal)translation.y;
            numericUpDown_translation_z.Value = (decimal)translation.z;
        }

        void setNumericUpDownNavMeshPosition(Float3 translation)
        {
            numericUpDownNavmeshPositionX.Value = (decimal)translation.x;
            numericUpDownNavmeshPositionY.Value = (decimal)translation.y;
            numericUpDownNavmeshPositionZ.Value = (decimal)translation.z;
        }

        private void updateTranslation()
        {
            Float3 translation;
            translation.x = (float)numericUpDown_translation_x.Value;
            translation.y = (float)numericUpDown_translation_y.Value;
            translation.z = (float)numericUpDown_translation_z.Value;

            if (m_isMeshInstanceSelected == 1)
            {
                updateSelectedMeshInstanceTransform(translation);
            }
            else
            {
                EditorNodeEntry entry = (EditorNodeEntry)treeView_entities.SelectedNode;

                if (entry.type == s_typeMeshInstance)
                {
                    //NativeMethods.updateTranslation(entry.sid, translation);
                }
            }
        }

        private void treeView_entities_AfterSelect(object sender, TreeViewEventArgs e)
        {
            try
            {
                EditorNodeEntry entry = (EditorNodeEntry)treeView_entities.SelectedNode;

                if (entry.type == s_typeMeshInstance)
                {
                    Float3 translation;
                    translation.x = translation.y = translation.z = 0.0f;

                    Float3 rotation;
                    rotation.x = rotation.y = rotation.z = 0.0f;

                    float scaling = 1.0f;

                    /*if (NativeMethods.getTransform(entry.sid, ref translation, ref rotation, ref scaling) != 0)
                    {
                        numericUpDown_translation_x.Value = (decimal)translation.x;
                        numericUpDown_translation_y.Value = (decimal)translation.y;
                        numericUpDown_translation_z.Value = (decimal)translation.z;
                    }*/

                    groupBoxMesh.Show();
                }
            }
            catch
            {
                groupBoxMesh.Hide();
            }
        }

        private void numericUpDown_translation_x_ValueChanged(object sender, EventArgs e)
        {
            updateTranslation();
        }

        private void numericUpDown_translation_y_ValueChanged(object sender, EventArgs e)
        {
            updateTranslation();
        }

        private void numericUpDown_translation_z_ValueChanged(object sender, EventArgs e)
        {
            updateTranslation();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void saveSceneToolStripMenuItem_Click(object sender, EventArgs e)
        {
            saveFileDialog_scene.ShowDialog();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                NativeMethods.shutdownEditor();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void loadSceneToolStripMenuItem_Click(object sender, EventArgs e)
        {
        }

        private void panel_render_MouseEnter(object sender, EventArgs e)
        {
            panel_render.Focus();
        }

        private void selectMesh(int x, int y)
        {
            Float3 translation;
            translation.x = translation.y = translation.z = 0.0f;
            NativeMethods.selectMesh(x, y);
        }

        public void selectMesh()
        {
            selectMesh(m_mouseX, m_mouseY);
        }

        public void deselectMesh()
        {
            NativeMethods.deselectMesh();
        }

        public void selectNavMeshVertex()
        {
            if(NativeMethods.selectNavMeshVertex(m_mouseX, m_mouseY))
            {
                m_selectedNavMesh = true;

                Float3 position;
                position.x = position.y = position.z = 0;
                NativeMethods.getSelectNavMeshVertexPosition(ref position);
                setNumericUpDownNavMeshPosition(position);

                groupBoxNavMesh.Show();
                m_selectedNavMesh = false;
            }
            else
            {
                groupBoxNavMesh.Hide();
            }
        }

        public void multiSelectNavMeshVertex()
        {
            NativeMethods.multiSelectNavMeshVertex(m_mouseX, m_mouseY);
            groupBoxMesh.Hide();
        }

        public void deselectNavMeshVertex()
        {
            NativeMethods.deselectNavMeshVertex();
            groupBoxNavMesh.Hide();
        }

        private void addNavMeshVertex(int x, int y)
        {
            NativeMethods.addNavMeshVertex(x, y);
        }

        private void updateStateMachine()
        {
            List<Action> actions = m_selectItemStateMachine.update();
            foreach (var item in actions)
            {
                item.run();
            }
        }

        private void panel_render_MouseDown(object sender, MouseEventArgs e)
        {
            m_mouseX = e.X;
            m_mouseY = e.Y;
            m_lastClickedMouseButton = e.Button;
            m_isMouseDown = true;
        }

        private void panel_render_MouseUp(object sender, MouseEventArgs e)
        {
            updateStateMachine();
            m_isMouseDown = false;
        }

        private void panel_render_MouseMove(object sender, MouseEventArgs e)
        {
            int dx = e.X - m_mouseX;
            int dy = e.Y - m_mouseY;

            if (dx < 0)
                dx = -1;
            else if (dx > 0)
                dx = 1;

            if (dy < 0)
                dy = -1;
            else if (dy > 0)
                dy = 1;

            if (m_keyDownAlt)
            {
                if (e.Button == MouseButtons.Middle)
                {
                    NativeMethods.moveCamera(dx, dy, 0);
                }
                else if (e.Button == MouseButtons.Left)
                {
                    // rotate
                    NativeMethods.rotateCamera(-dx, -dy, 0);
                }
            }

            m_mouseX = e.X;
            m_mouseY = e.Y;
        }

        private void panel_render_MouseMove_MouseWheel(object sender, MouseEventArgs e)
        {
            if (e.Delta > 0)
            {
                NativeMethods.moveCamera(0, 0, -1);
            }
            else if (e.Delta < 0)
            {
                NativeMethods.moveCamera(0, 0, 1);
            }
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.ShiftKey)
            {
                m_keyDownShift = false;
            }
            else if(e.KeyCode == Keys.C)
            {
                NativeMethods.createNavMeshTriangleFromSelectedVertices();
            }
            else if(e.KeyCode == Keys.Delete)
            {
                if (m_editorState == EditorState.EditNavMesh)
                    NativeMethods.deleteSelectedNavMeshVertex();
            }

            if (e.Alt)
            {
                e.Handled = true;
            }

            m_keyDownAlt = e.Alt;

            m_keys ^= (1 << (int)e.KeyCode);
        }

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.ShiftKey)
            {
                m_keyDownShift = true;
            }
            else if (e.Alt)
            {
                e.Handled = true;
            }

            m_keyDownAlt = e.Alt;
            m_keys ^= (1 << (int)e.KeyCode);
        }

        private void toolStripButton1_Click(object sender, EventArgs e)
        {
            m_editorState = EditorState.EditNavMesh;
        }

        private void toolStripButton2_Click(object sender, EventArgs e)
        {

        }

        private void importAssetToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog_importAsset.ShowDialog();
        }

        private void openFileDialog1_loadScene_FileOk(object sender, CancelEventArgs e)
        {
            string filename = openFileDialog1_loadScene.SafeFileName;
            string ext = System.IO.Path.GetExtension(filename);

            UInt64 sid = NativeMethods.getSid(filename);

            if (ext == ".scene")
            {
                m_requestedFiles.Add(sid, filename);

                NativeMethods.loadFile(filename, s_typeScene, Form1.loadFileCallback);
                m_currentSceneFileName = filename;

                this.Text = "vxEditor: " + filename;
            }
        }

        private void panel_render_Paint(object sender, PaintEventArgs e)
        {
        }

        private void saveFileDialog_scene_FileOk(object sender, CancelEventArgs e)
        {

            string filename = saveFileDialog_scene.FileName;
            string filename_with_ext = System.IO.Path.GetFileName(filename);
            string ext = System.IO.Path.GetExtension(filename);
            if (ext == ".scene")
            {
                NativeMethods.saveScene(filename_with_ext);
            }
            //  
        }

        public EditorState getEditorState()
        {
            return m_editorState;
        }

        private void comboBox_selectEditorMode_SelectedIndexChanged(object sender, EventArgs e)
        {
            var selectedString = comboBox_selectEditorMode.SelectedItem.ToString();
            if (selectedString == s_textEditMesh)
            {
                m_editorState = EditorState.EditMesh;
            }
            else if (selectedString == s_textEditNavMesh)
            {
                m_editorState = EditorState.EditNavMesh;
            }
            else if (selectedString == s_textEditLights)
            {
                m_editorState = EditorState.EditLights;
            }
            else if (selectedString == s_textEditSpawns)
            {
                m_editorState = EditorState.EditSpawns;
            }

            updateStateMachine();
        }

        public MouseButtons getLastClickedMouseButton()
        {
            return m_lastClickedMouseButton;
        }

        private void comboBox_selectEditorMode_Click(object sender, EventArgs e)
        {

        }

        public bool isMouseDown()
        {
            return m_isMouseDown;
        }

        public bool isShiftDown()
        {
            return m_keyDownShift;
        }

        private void toolStripButton1_Click_1(object sender, EventArgs e)
        {
            NativeMethods.createNavMeshTriangleFromSelectedVertices();
        }

        public void createNavMeshVertex()
        {
            NativeMethods.addNavMeshVertex(m_mouseX, m_mouseY);
        }

        public bool isKeyDown(System.Windows.Forms.Keys key)
        {
            var mask = (1 << (int)key);

            return ((m_keys & mask) == mask);
        }

        private void toolStripButton1_Click_2(object sender, EventArgs e)
        {
            NativeMethods.createNavMeshTriangleFromSelectedVertices();
        }

        void setSelectedNavMeshVertexPosition()
        {
            Float3 position;
            position.x = (float)numericUpDownNavmeshPositionX.Value;
            position.y = (float)numericUpDownNavmeshPositionY.Value;
            position.z = (float)numericUpDownNavmeshPositionZ.Value;

            if (!m_selectedNavMesh)
                NativeMethods.setSelectNavMeshVertexPosition(ref position);
        }

        private void numericUpDownNavmeshPositionX_ValueChanged(object sender, EventArgs e)
        {
            setSelectedNavMeshVertexPosition();
        }

        private void numericUpDownNavmeshPositionY_ValueChanged(object sender, EventArgs e)
        {
            setSelectedNavMeshVertexPosition();
        }

        private void numericUpDownNavmeshPositionZ_ValueChanged(object sender, EventArgs e)
        {
            setSelectedNavMeshVertexPosition();
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            NativeMethods.saveScene(m_currentSceneFileName);
        }
    }
}
