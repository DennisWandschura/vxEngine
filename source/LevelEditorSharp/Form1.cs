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
        bool m_selectedNavMesh;
        string m_currentSceneFileName;
        ulong m_selectedMeshInstanceSid;

        Dictionary<ulong, string> m_requestedFiles;
        Dictionary<ulong, EditorNodeEntry> m_sortedMeshInstances;
        Dictionary<ulong, EditorEntry> m_sortedMeshes;
        Dictionary<ulong, EditorEntry> m_sortedMaterials;
        Dictionary<Keys, bool> m_keys;
        ActionList m_actionListHead;

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
            m_sortedMeshInstances = new Dictionary<ulong, EditorNodeEntry>();
            m_sortedMeshes = new Dictionary<ulong, EditorEntry>();
            m_sortedMaterials = new Dictionary<ulong, EditorEntry>();
            m_keys = new Dictionary<Keys, bool>();

            loadFileDelegate = new LoadedFileFun(loadedFile);

            m_keyDownAlt = false;
            m_isMouseDown = false;
            m_keyDownShift = false;
            m_selectedNavMesh = false;
            m_mouseX = 0;
            m_mouseY = 0;
            m_selectedMeshInstanceSid = 0;

            m_actionListHead = new ActionList(null, null);

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

            groupBoxLight.Hide();
            groupBoxLight.Location = p;
        }

        ~Form1()
        {
        }

        private ActionDecisionTree createActionOnMouseClickNavMesh(State stateEditNavMesh, ActionDeselectNavMesh actionDeselectNavMesh)
        {
            ActionSelectNavMesh actionSelectNavMesh = new ActionSelectNavMesh(this);
            ActionMultiSelectNavMesh actionMultiSelectNavMesh = new ActionMultiSelectNavMesh(this);

            TargetState stateSelectNavMeshVertex = new TargetState(stateEditNavMesh, "stateSelectNavMeshVertex");
            stateSelectNavMeshVertex.addAction(actionSelectNavMesh);

            TargetState stateMultiSelectNavMeshVertex = new TargetState(stateEditNavMesh, "stateMultiSelectNavMeshVertex");
            stateMultiSelectNavMeshVertex.addAction(actionMultiSelectNavMesh);

            TargetState stateDeselectNavMeshVertex = new TargetState(stateEditNavMesh, "stateDeselectNavMeshVertex");
            stateDeselectNavMeshVertex.addAction(actionDeselectNavMesh);

            TargetState stateCreateVertex = new TargetState(stateEditNavMesh, "stateCreateNavMeshVertex");
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

            TargetState stateKeyUp = new TargetState(stateEditNavMesh, "stateCreateNavMeshTriangle");
            stateKeyUp.addAction(actionCreateNavMeshTriangle);

            DecisionSelectedNavMesh decisionSelectedNavMesh = new DecisionSelectedNavMesh(stateKeyUp, null);
            DecisionKeyUp decKeyUp = new DecisionKeyUp(decisionSelectedNavMesh, null, this, Keys.C);

            ActionDecisionTree actionKeyUp = new ActionDecisionTree(decKeyUp);

            stateEditNavMesh.addAction(actionKeyUp);
            stateEditNavMesh.addAction(actionOnMouseClick);
            stateEditNavMesh.addExitAction(actionDeselectNavMesh);

            return stateEditNavMesh;
        }

        private ActionDecisionTree createActionOnMouseClickMesh(State stateEditMesh, ActionDeselectMesh actionDeselectMesh)
        {
            ActionSelectMesh actionSelectMesh = new ActionSelectMesh(this);

            TargetState stateSelectMesh = new TargetState(stateEditMesh, "stateSelectMesh");
            stateSelectMesh.addAction(actionSelectMesh);

            TargetState stateDeselectMesh = new TargetState(stateEditMesh, "destateSelectMesh");
            stateDeselectMesh.addAction(actionDeselectMesh);

            DecisionEditorMouseButtonPressed decisionMouseRightButton = new DecisionEditorMouseButtonPressed(stateDeselectMesh, null, this, MouseButtons.Right);
            DecisionEditorMouseButtonPressed decisionMouseLeftButton = new DecisionEditorMouseButtonPressed(stateSelectMesh, decisionMouseRightButton, this, MouseButtons.Left);

            ActionDecisionTree actionOnMouseClick = new ActionDecisionTree(decisionMouseLeftButton);

            return actionOnMouseClick;
        }

        private State createStateEditLights()
        {
            ActionCallFunction actionDeselectLight = new ActionCallFunction(deselectLight, selectLight, true);
            ActionCallFunction actionSelectLight = new ActionCallFunction(selectLight, deselectLight, true);
            State stateEditLights = new State();

            TargetState stateSelectLight = new TargetState(stateEditLights, "stateSelectLight");
            stateSelectLight.addAction(actionSelectLight);

            TargetState stateDeselectLight = new TargetState(stateEditLights, "destateSelectLight");
            stateDeselectLight.addAction(actionDeselectLight);

            DecisionEditorMouseButtonPressed decisionMouseRightButton = new DecisionEditorMouseButtonPressed(stateDeselectLight, null, this, MouseButtons.Right);
            DecisionEditorMouseButtonPressed decisionMouseLeftButton = new DecisionEditorMouseButtonPressed(stateSelectLight, decisionMouseRightButton, this, MouseButtons.Left);
            ActionDecisionTree actionOnMouseClick = new ActionDecisionTree(decisionMouseLeftButton);

            ActionCallFunction actionShowGui = new ActionCallFunction(showLightGui, hideLightGui, false);
            ActionCallFunction actionHideGui = new ActionCallFunction(hideLightGui, showLightGui, false);

            stateEditLights.addEntryAction(actionShowGui);
            stateEditLights.addAction(actionOnMouseClick);
            stateEditLights.addExitAction(actionHideGui);
            stateEditLights.addExitAction(actionDeselectLight);

            return stateEditLights;
        }

        private State createStateEditMesh()
        {
            ActionDeselectMesh actionDeselectMesh = new ActionDeselectMesh(this);

            ActionCallFunction actionShowGui = new ActionCallFunction(showMeshGui, hideMeshGui, false);
            ActionCallFunction actionHideGui = new ActionCallFunction(hideMeshGui, showMeshGui, false);

            State stateEditMesh = new State();

            var actionOnMouseClick = createActionOnMouseClickMesh(stateEditMesh, actionDeselectMesh);

            stateEditMesh.addEntryAction(actionShowGui);
            stateEditMesh.addAction(actionOnMouseClick);
            stateEditMesh.addExitAction(actionHideGui);
            stateEditMesh.addExitAction(actionDeselectMesh);

            return stateEditMesh;
        }

        void createStateMachine()
        {
            m_selectItemStateMachine = new StateMachine();

            ConditionEditorState conditionEditorStateEditMesh = new ConditionEditorState(this, EditorState.EditMesh);
            ConditionEditorState conditionEditorStateEditLights = new ConditionEditorState(this, EditorState.EditLights);
            ConditionEditorState conditionEditorStateEditNavMesh = new ConditionEditorState(this, EditorState.EditNavMesh);

            var stateEditNavMesh = createStateEditNavMesh();
            var stateEditMesh = createStateEditMesh();
            var stateEditLights = createStateEditLights();

            Transition transitionEditMesh = new Transition(conditionEditorStateEditMesh, stateEditMesh, "transitionEditMesh");
            Transition transitionEditNavMesh = new Transition(conditionEditorStateEditNavMesh, stateEditNavMesh, "transitionEditNavMesh");
            Transition transitionEditLights = new Transition(conditionEditorStateEditLights, stateEditLights, "transitionEditLights");

            stateEditNavMesh.addTransition(transitionEditMesh);
            stateEditNavMesh.addTransition(transitionEditLights);

            stateEditMesh.addTransition(transitionEditNavMesh);
            stateEditMesh.addTransition(transitionEditLights);

            stateEditLights.addTransition(transitionEditNavMesh);
            stateEditLights.addTransition(transitionEditMesh);

            m_selectItemStateMachine.addState(stateEditNavMesh);
            m_selectItemStateMachine.addState(stateEditMesh);
            m_selectItemStateMachine.addState(stateEditLights);

            State emptyState = new State();
            emptyState.addTransition(transitionEditMesh);
            emptyState.addTransition(transitionEditNavMesh);
            emptyState.addTransition(transitionEditLights);

            m_selectItemStateMachine.setCurrentState(emptyState);
        }

        void showLightGui()
        {
            toolStripButtonCreateLight.Visible = true;

        }
        void hideLightGui()
        {
            toolStripButtonCreateLight.Visible = false;
        }

        void getSelectedLightPosition()
        {
            Float3 position;
            position.x = position.y = position.z = 0;
            NativeMethods.getSelectLightPosition(ref position);
            setNumericUpDownLightPosition(position);
            groupBoxLight.Show();
        }

        public void selectLight()
        {
            if (NativeMethods.selectLight(m_mouseX, m_mouseY))
            {
                getSelectedLightPosition();
            }
        }

        public void deselectLight()
        {
            NativeMethods.deselectLight();
            groupBoxLight.Hide();
        }

        void setNumericUpDownLightPosition(Float3 position)
        {
            numericUpDownLightX.Value = (decimal)position.x;
            numericUpDownLightY.Value = (decimal)position.y;
            numericUpDownLightZ.Value = (decimal)position.z;
        }

        void setSelectedLightPosition()
        {
            Float3 position;
            position.x = position.y = position.z = 0;

            position.x = (float)numericUpDownLightX.Value;
            position.y = (float)numericUpDownLightY.Value;
            position.z = (float)numericUpDownLightZ.Value;
            NativeMethods.setSelectLightPosition(ref position);
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

        void addSceneMeshes()
        {
            meshInstanceComboBoxMesh.Items.Clear();
            m_sortedMeshes.Clear();
            m_meshNode.Nodes.Clear();

            var meshCount = NativeMethods.getMeshCount();
            for (uint i = 0; i < meshCount; ++i)
            {
                var meshName = NativeMethods.getMeshName(i);
                var meshSid = NativeMethods.getMeshSid(i);
                var meshEntry = new EditorNodeEntry(meshSid, s_typeMesh, meshName);

                m_meshNode.Nodes.Add(meshEntry);

                EditorEntry entry = new EditorEntry(meshName, meshSid);

                meshInstanceComboBoxMesh.Items.Add(entry);
                m_sortedMeshes.Add(meshEntry.sid, entry);
            }
        }

        void addSceneMeshInstances()
        {
            m_sortedMeshInstances.Clear();
            m_meshInstanceNode.Nodes.Clear();

            var meshInstanceCount = NativeMethods.getMeshInstanceCount();
            for (uint i = 0; i < meshInstanceCount; ++i)
            {
                var meshInstanceName = NativeMethods.getMeshInstanceName(i);
                var meshInstanceSid = NativeMethods.getMeshInstanceSid(i);

                var entry = new EditorNodeEntry(meshInstanceSid, s_typeMeshInstance, meshInstanceName);

                m_sortedMeshInstances.Add(entry.sid, entry);

                m_meshInstanceNode.Nodes.Add(entry);
            }
        }

        void addSceneMaterials()
        {
            meshInstanceComboBoxMaterial.Items.Clear();
            m_sortedMaterials.Clear();
            m_materialNode.Nodes.Clear();

            var count = NativeMethods.getMaterialCount();
            for (uint i = 0; i < count; ++i)
            {
                var name = NativeMethods.getMaterialName(i);
                var sid = NativeMethods.getMaterialSid(i);
                var nodeEntry = new EditorNodeEntry(sid, s_typeMesh, name);

                m_materialNode.Nodes.Add(nodeEntry);

                EditorEntry test = new EditorEntry(name, sid);

                meshInstanceComboBoxMaterial.Items.Add(test);
                m_sortedMaterials.Add(nodeEntry.sid, test);
            }
        }

        void addMesh(ulong sid, string name)
        {
            EditorEntry entry;
            if(!m_sortedMeshes.TryGetValue(sid, out entry))
            {
                var nodeEntry = new EditorNodeEntry(sid, s_typeMesh, name);
                EditorEntry editorEntry = new EditorEntry(name, sid);

                meshInstanceComboBoxMesh.Items.Add(nodeEntry);


                meshInstanceComboBoxMesh.Items.Add(editorEntry);
                m_sortedMeshes.Add(sid, editorEntry);
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
                    addMesh(sid, str);
                }
                else if (type == s_typeMaterial)
                {
                   // m_materialNode.Nodes.Add(new EditorNodeEntry(sid, s_typeMaterial, str));
                }
                else if (type == s_typeScene)
                {
                    addSceneMeshes();
                    addSceneMaterials();
                    addSceneMeshInstances();
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

        void setNumericUpDownTranslation(Float3 translation)
        {
            numericUpDown_translation_x.Value = (decimal)translation.x;
            numericUpDown_translation_y.Value = (decimal)translation.y;
            numericUpDown_translation_z.Value = (decimal)translation.z;
        }

        void setNumericUpDownRotation(Float3 rotation)
        {
            numericUpDown_rot_x.Value = (decimal)rotation.x;
            numericUpDown_rot_y.Value = (decimal)rotation.y;
            numericUpDown_rot_z.Value = (decimal)rotation.z;
        }

        void setNumericUpDownNavMeshPosition(Float3 translation)
        {
            numericUpDownNavmeshPositionX.Value = (decimal)translation.x;
            numericUpDownNavmeshPositionY.Value = (decimal)translation.y;
            numericUpDownNavmeshPositionZ.Value = (decimal)translation.z;
        }

        private void getMeshInstanceTransform(ulong sid)
        {
            Float3 translation;
            translation.x = translation.y = translation.z = 0.0f;

            NativeMethods.getMeshInstancePosition(sid, ref translation);

            Float3 rotation;
            rotation.x = rotation.y = rotation.z = 0.0f;

            NativeMethods.getMeshInstanceRotation(sid, ref rotation);

            setNumericUpDownTranslation(translation);
            setNumericUpDownRotation(rotation);
        }

        void updateGuiSelectedMeshInstance(ulong sid)
        {
            getMeshInstanceTransform(sid);

            textBoxMeshName.Text = NativeMethods.getSelectedMeshInstanceName();

            var materialSid = NativeMethods.getMeshInstanceMaterialSid(sid);
            var meshSid = NativeMethods.getMeshInstanceMeshSid(sid);

            EditorEntry entry;
            if (m_sortedMeshes.TryGetValue(meshSid, out entry))
            {
                meshInstanceComboBoxMesh.SelectedItem = entry;
            }

            if (m_sortedMaterials.TryGetValue(materialSid, out entry))
            {
                meshInstanceComboBoxMaterial.SelectedItem = entry;
            }
        }

        void showMeshGui()
        {
            toolStripButtonCreateMeshInstance.Visible = true;
        }

        void hideMeshGui()
        {
            toolStripButtonCreateMeshInstance.Visible = false;
        }

        private void setMeshInstancePosition()
        {
            Float3 translation;
            translation.x = (float)numericUpDown_translation_x.Value;
            translation.y = (float)numericUpDown_translation_y.Value;
            translation.z = (float)numericUpDown_translation_z.Value;

            var sid = NativeMethods.getSelectedMeshInstanceSid();

            if (m_selectedMeshInstanceSid == sid)
            {
                ActionSetMeshInstancePosition actionSetMeshInstancePosition = new ActionSetMeshInstancePosition(sid, translation);
                runAction(actionSetMeshInstancePosition);
            }
        }

        private void setMeshInstanceRotation()
        {
            Float3 rotation;
            rotation.x = (float)numericUpDown_rot_x.Value;
            rotation.y = (float)numericUpDown_rot_y.Value;
            rotation.z = (float)numericUpDown_rot_z.Value;

            var sid = NativeMethods.getSelectedMeshInstanceSid();

            if (m_selectedMeshInstanceSid == sid)
            {
                ActionSetMeshInstanceRotation actionSetMeshInstanceRotation = new ActionSetMeshInstanceRotation(sid, rotation);
                runAction(actionSetMeshInstanceRotation);
            }
        }

        private void treeView_entities_AfterSelect(object sender, TreeViewEventArgs e)
        {
            try
            {
                EditorNodeEntry entry = (EditorNodeEntry)treeView_entities.SelectedNode;

                if (entry.type == s_typeMeshInstance)
                {
                    uint index = (uint)entry.Index;

                    NativeMethods.selectMeshInstanceIndex(index);
                    var newSelectedSid = NativeMethods.getSelectedMeshInstanceSid();

                    updateGuiSelectedMeshInstance(newSelectedSid);

                    m_selectedMeshInstanceSid = newSelectedSid;

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
            setMeshInstancePosition();
        }

        private void numericUpDown_translation_y_ValueChanged(object sender, EventArgs e)
        {
            setMeshInstancePosition();
        }

        private void numericUpDown_translation_z_ValueChanged(object sender, EventArgs e)
        {
            setMeshInstancePosition();
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

        void setSelectedMeshInstanceNode()
        {
            var instanceSid = NativeMethods.getSelectedMeshInstanceSid();

            EditorNodeEntry entry;
            if (m_sortedMeshInstances.TryGetValue(instanceSid, out entry))
            {
                treeView_entities.SelectedNode = entry;
            }
        }

        public bool selectMesh(int mouseX, int mouseY)
        {
            bool result = false;
            Float3 translation;
            translation.x = translation.y = translation.z = 0.0f;
            if (NativeMethods.selectMeshInstance(mouseX, mouseY))
            {
                setSelectedMeshInstanceNode();
                result = true;
            }

            return result;
        }

        public bool selectMesh(int mouseX, int mouseY, out ulong selectedSid)
        {
            selectedSid = NativeMethods.getSelectedMeshInstanceSid();

            return selectMesh(mouseX, mouseY);
        }

        public void selectMesh(ulong sid)
        {
            Float3 translation;
            translation.x = translation.y = translation.z = 0.0f;

            if (NativeMethods.selectMeshInstanceSid(sid))
            {
                setSelectedMeshInstanceNode();
            }
        }

        public ulong deselectMesh()
        {
            var sid = NativeMethods.deselectMeshInstance();

            treeView_entities.SelectedNode = null;
            groupBoxMesh.Hide();

            return sid;
        }

        void updateNavMeshVertexGui()
        {
            m_selectedNavMesh = true;

            Float3 position;
            position.x = position.y = position.z = 0;
            NativeMethods.getSelectNavMeshVertexPosition(ref position);
            setNumericUpDownNavMeshPosition(position);

            groupBoxNavMesh.Show();
            m_selectedNavMesh = false;
        }

        public void selectNavMeshVertex(int mouseX, int mouseY)
        {
            if (NativeMethods.selectNavMeshVertex(mouseX, mouseY))
            {
                updateNavMeshVertexGui();
            }
            else
            {
                groupBoxNavMesh.Hide();
            }
        }

        public void selectNavMeshVertex(ref Float3 position)
        {
            if (NativeMethods.selectNavMeshVertexPosition(ref position))
            {
                updateNavMeshVertexGui();
            }
            else
            {
                groupBoxNavMesh.Hide();
            }
        }

        public bool selectNavMeshVertex(int mouseX, int mouseY, out int hasPreviousSelected, ref Float3 position)
        {
            hasPreviousSelected = 0;

            if (NativeMethods.getSelectNavMeshVertexPosition(ref position))
            {
                hasPreviousSelected = 1;
            }

            bool result = false;
            if (NativeMethods.selectNavMeshVertex(mouseX, mouseY))
            {
                updateNavMeshVertexGui();
                result = true;
            }
            else
            {
                groupBoxNavMesh.Hide();
            }

            return result;
        }

        public void selectNavMeshVertex(uint index, out int mm)
        {
            mm = 0;
            if (NativeMethods.selectNavMeshVertexIndex(index))
            {
                updateNavMeshVertexGui();
            }
            else
            {
                groupBoxNavMesh.Hide();
            }
        }

        public void multiSelectNavMeshVertex()
        {
            NativeMethods.multiSelectNavMeshVertex(m_mouseX, m_mouseY);
            groupBoxNavMesh.Hide();
        }

        public uint deselectNavMeshVertex()
        {
            var index = NativeMethods.deselectNavMeshVertex();
            groupBoxNavMesh.Hide();

            return index;
        }

        void runAction(Action action)
        {
            action.run();
            var clonedAction = action.clone();
            if (clonedAction != null)
            {
                ActionList actionList = new ActionList(clonedAction, m_actionListHead);

                treeViewActionList.Nodes.Add(actionList.ToString());
                m_actionListHead.setNext(actionList);
                m_actionListHead = actionList;
            }
        }

        private void updateStateMachine()
        {
            List<Action> actions = m_selectItemStateMachine.update();
            foreach (var item in actions)
            {
                runAction(item);
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
            else if (e.KeyCode == Keys.C)
            {
                // NativeMethods.createNavMeshTriangleFromSelectedVertices();
            }
            else if (e.KeyCode == Keys.Delete)
            {
                //if (m_editorState == EditorState.EditNavMesh)
                //   NativeMethods.removeSelectedNavMeshVertex();
            }

            if (e.Alt)
            {
                e.Handled = true;
            }

            m_keyDownAlt = e.Alt;

            updateStateMachine();

            m_keys[Keys.C] = false;
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

            m_keys[e.KeyCode] = true;

            m_keyDownAlt = e.Alt;
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

        public int getMouseX()
        {
            return m_mouseX;
        }

        public int getMouseY()
        {
            return m_mouseY;
        }

        public bool isKeyDown(System.Windows.Forms.Keys key)
        {
            bool isDown = false;
            //m_keys.TryGetValue(key, out isDown);

            return isDown;
        }

        public bool isKeyUp(System.Windows.Forms.Keys key)
        {
            bool isDown = false;
            bool isUp = false;
            if (m_keys.TryGetValue(key, out isDown))
            {
                isUp = isDown;
            }

            return isUp;
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

        private void itemShotNavmesh_Click(object sender, EventArgs e)
        {
            NativeMethods.showNavmesh(itemShotNavmesh.Checked);
        }

        private void itemInfluenceMap_Click(object sender, EventArgs e)
        {
            NativeMethods.showInfluenceMap(itemInfluenceMap.Checked);
        }

        private void numericUpDownLightX_ValueChanged(object sender, EventArgs e)
        {
            setSelectedLightPosition();
        }

        private void numericUpDownLightY_ValueChanged(object sender, EventArgs e)
        {
            setSelectedLightPosition();
        }

        private void numericUpDownLightZ_ValueChanged(object sender, EventArgs e)
        {
            setSelectedLightPosition();
        }

        private void toolStripButtonCreateLight_Click(object sender, EventArgs e)
        {
            NativeMethods.createLight();
            getSelectedLightPosition();
        }

        private void meshInstanceComboBoxMaterial_SelectedIndexChanged(object sender, EventArgs e)
        {
            EditorEntry item = (EditorEntry)meshInstanceComboBoxMaterial.SelectedItem;

            setMeshInstanceMaterial(item.m_sid);
        }

        void setMeshInstanceMaterial(ulong materialSid)
        {
           var instanceSid = NativeMethods.getSelectedMeshInstanceSid();

           if (instanceSid != 0 && m_selectedMeshInstanceSid == instanceSid)
           {
              var oldSid = NativeMethods.getMeshInstanceMaterialSid(instanceSid);
              var action = new ActionSetMeshInstanceMaterial(instanceSid, oldSid, materialSid);
              runAction(action);
           }
        }

        public void onRenameMeshInstance(ulong oldSid, ulong newSid, string newName)
        {
            EditorNodeEntry nodeEntry;
            if (m_sortedMeshInstances.TryGetValue(oldSid, out nodeEntry))
            {
                m_sortedMeshInstances.Remove(oldSid);
                nodeEntry.sid = newSid;
                nodeEntry.Text = newName;
                m_sortedMeshInstances.Add(newSid, nodeEntry);
            }
        }

        private void textBoxMeshName_TextChanged(object sender, EventArgs e)
        {
            var oldSid = NativeMethods.getSelectedMeshInstanceSid();

            if (oldSid != 0)
            {
                var oldName = NativeMethods.getSelectedMeshInstanceName();

                var newName = textBoxMeshName.Text;
                var newSid = NativeMethods.getSid(newName);

                if (newSid != oldSid)
                {
                    ActionRenameSelectedMeshInstance action = new ActionRenameSelectedMeshInstance(oldName, oldSid, newName, newSid, this);

                    runAction(action);
                }
            }
        }

        private void textBoxMeshName_MouseLeave(object sender, EventArgs e)
        {
            label6.Focus();
        }

        private void toolStripButtonCreateMeshInstance_Click(object sender, EventArgs e)
        {
            NativeMethods.createMeshInstance();
            addSceneMeshInstances();
        }

        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_actionListHead.undo();

            var prev = m_actionListHead.prev();
            var tmp = prev.next();
            if (prev == null)
            {
                Console.WriteLine("Error undo, null");
            }
            else if (prev == m_actionListHead)
            {
                Console.WriteLine("Error undo");
            }
            else if (tmp != m_actionListHead)
            {
                Console.WriteLine("Error undo");
            }

            if (prev != null)
            {
                m_actionListHead = prev;
            }
        }

        private void redoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (m_actionListHead != null)
            {
                var next = m_actionListHead.next();
                if (next != null)
                {
                    m_actionListHead = next;
                }

                m_actionListHead.redo();
            }
        }

        private void Form1_KeyPress(object sender, KeyPressEventArgs e)
        {
        }

        private void numericUpDown_rot_x_ValueChanged(object sender, EventArgs e)
        {
            setMeshInstanceRotation();
        }

        private void numericUpDown_rot_y_ValueChanged(object sender, EventArgs e)
        {
            setMeshInstanceRotation();
        }

        private void numericUpDown_rot_z_ValueChanged(object sender, EventArgs e)
        {
            setMeshInstanceRotation();
        }

        private void meshInstanceComboBoxMesh_SelectedIndexChanged(object sender, EventArgs e)
        {
            EditorEntry entry = (EditorEntry)meshInstanceComboBoxMesh.SelectedItem;

            var newMeshSid = entry.m_sid;

            var meshInstanceSid = NativeMethods.getSelectedMeshInstanceSid();

            setMeshInstanceMesh(meshInstanceSid, newMeshSid);
        }

        void setMeshInstanceMesh(ulong meshInstanceSid, ulong newMeshSid)
        {
            if(m_selectedMeshInstanceSid == meshInstanceSid)
            {
                var oldMeshSid = NativeMethods.getMeshInstanceMeshSid(meshInstanceSid);

                if(oldMeshSid != newMeshSid)
                {
                    var action = new ActionSetMeshInstanceMesh(meshInstanceSid, oldMeshSid, newMeshSid);
                    runAction(action);
                }
            }
        }
    }
}
