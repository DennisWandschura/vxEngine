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
    public enum EditorState { EditMesh, EditNavMesh, EditLights, EditSpawns, EditWaypoints, EditJoints };

    public partial class EditorForm : Form
    {
        const string s_textEditMesh = "Edit Mesh";
        const string s_textEditNavMesh = "Edit Nav Mesh";
        const string s_textEditLights = "Edit Lights";
        const string s_textEditSpawns = "Edit Spawns";
        const string s_textEditWaypoints = "Edit Waypoints";
        const string s_textEditJoints = "Edit Joints";
        const int s_groupBoxEditPositionX = 2000;
        const int s_groupBoxEditPositionY = 680;

        const uint s_typeMesh = 0;
        const uint s_typeMaterial = 1;
        const uint s_typeMeshInstance = 2;
        const uint s_typeScene = 3;
        const uint s_typeFbx = 4;
        const uint s_typeAnimation = 5;
        const uint s_typeJoint = 6;

        static EditorForm s_form;

        public delegate void LoadedFileFun(UInt64 sid, UInt32 type);

        public LoadedFileFun loadFileDelegate;
        TreeNode m_meshNode;
        TreeNode m_materialNode;
        TreeNode m_meshInstanceNode;
        TreeNode m_waypointsNode;
        TreeNode m_animationsNode;
        TreeNode m_jointsNode;
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
        FileBrowser m_fileBrowser;
        MeshInfoControl m_meshInfoControl;
        MeshInstanceDataControl m_meshInstanceDataControl;
        JointDataControl m_jointDataControl;

        Dictionary<ulong, string> m_requestedFiles;
        Dictionary<ulong, EditorNodeEntry> m_sortedMeshInstanceNodes;
        Dictionary<ulong, EditorEntry> m_sortedMeshes;
        Dictionary<ulong, EditorEntry> m_sortedMaterials;
        Dictionary<ulong, EditorEntry> m_sortedAnimations;
        Dictionary<Keys, bool> m_keys;
        ActionList m_actionListHead;
        uint m_selectedSpawn;

        public EditorForm()
        {
            InitializeComponent();

            m_meshInfoControl = new MeshInfoControl(this);
            m_meshInstanceDataControl = new MeshInstanceDataControl(this);
            m_jointDataControl = new JointDataControl(this);

            m_meshNode = treeView_entities.Nodes.Add("Meshes");
            m_materialNode = treeView_entities.Nodes.Add("Materials");
            m_meshInstanceNode = treeView_entities.Nodes.Add("Mesh Instances");
            m_waypointsNode = treeView_entities.Nodes.Add("Waypoints");
            m_animationsNode = treeView_entities.Nodes.Add("Animations");
            m_jointsNode = treeView_entities.Nodes.Add("Joints");

            treeView_entities.Nodes.Add("Lights");
            m_currentSceneFileName = "untitled.scene";

            m_requestedFiles = new Dictionary<ulong, string>();
            m_sortedMeshInstanceNodes = new Dictionary<ulong, EditorNodeEntry>();
            m_sortedMeshes = new Dictionary<ulong, EditorEntry>();
            m_sortedMaterials = new Dictionary<ulong, EditorEntry>();
            m_sortedAnimations = new Dictionary<ulong, EditorEntry>();
            m_keys = new Dictionary<Keys, bool>();

            loadFileDelegate = new LoadedFileFun(loadedFile);

            m_keyDownAlt = false;
            m_isMouseDown = false;
            m_keyDownShift = false;
            m_selectedNavMesh = false;
            m_mouseX = 0;
            m_mouseY = 0;
            m_selectedMeshInstanceSid = 0;
            m_selectedSpawn = 0;

            m_fileBrowser = new FileBrowser(this);
            m_fileBrowser.Hide();
            m_fileBrowser.Owner = this;

            m_actionListHead = new ActionList(null, null);

            createStateMachine();

            Panel tmp = new Panel();

            if (!NativeMethods.initializeEditor(panel_render.Handle, tmp.Handle, (uint)panel_render.Width, (uint)panel_render.Height, s_typeMesh, s_typeMaterial, s_typeScene, s_typeFbx, s_typeAnimation))
            {
                throw new Exception();
            }

            s_form = this;

            this.KeyPreview = true;
            panel_render.MouseWheel += panel_render_MouseMove_MouseWheel;

            m_editorState = EditorState.EditMesh;
            comboBox_selectEditorMode.Items.Add(s_textEditMesh);
            comboBox_selectEditorMode.Items.Add(s_textEditNavMesh);
            comboBox_selectEditorMode.Items.Add(s_textEditLights);
            comboBox_selectEditorMode.Items.Add(s_textEditSpawns);
            comboBox_selectEditorMode.Items.Add(s_textEditWaypoints);
            comboBox_selectEditorMode.Items.Add(s_textEditJoints);
            comboBox_selectEditorMode.SelectedIndex = 0;

            Point p = new Point();
            p.X = s_groupBoxEditPositionX;
            p.Y = s_groupBoxEditPositionY;

            groupBoxNavMesh.Hide();
            groupBoxNavMesh.Location = p;

            m_meshInstanceDataControl.Parent = this;
            m_meshInstanceDataControl.Hide();
            m_meshInstanceDataControl.Location = p;

            m_jointDataControl.Parent = this;
            m_jointDataControl.Hide();
            m_jointDataControl.Location = p;

            groupBoxLight.Hide();
            groupBoxLight.Location = p;

            groupBoxSpawn.Hide();
            groupBoxSpawn.Location = p;

            m_meshInfoControl.Parent = this;
            m_meshInfoControl.Hide();
            m_meshInfoControl.Location = p;
        }

        ~EditorForm()
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
            ActionRemoveNavMeshVertex actionRemoveNavMeshVertex = new ActionRemoveNavMeshVertex();

            State stateEditNavMesh = new State();

            var actionOnMouseClick = createActionOnMouseClickNavMesh(stateEditNavMesh, actionDeselectNavMesh);

            ActionCreateNavMeshTriangle actionCreateNavMeshTriangle = new ActionCreateNavMeshTriangle();

            TargetState stateKeyUp = new TargetState(stateEditNavMesh, "stateCreateNavMeshTriangle");
            stateKeyUp.addAction(actionCreateNavMeshTriangle);

            TargetState stateRemoveVertex = new TargetState(stateEditNavMesh, "stateRemoveNavMeshTriangle");
            stateRemoveVertex.addAction(actionRemoveNavMeshVertex);

            //DecisionKeyUp decisionDelKeyDown = new DecisionKeyUp(stateRemoveVertex, null, this, Keys.Delete);

            DecisionSelectedNavMesh decisionSelectedNavMesh = new DecisionSelectedNavMesh(stateKeyUp, null);
            DecisionKeyUp decKeyUpC = new DecisionKeyUp(decisionSelectedNavMesh, null, this, Keys.C);

            ActionDecisionTree actionKeyUp = new ActionDecisionTree(decKeyUpC);

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
            ActionCallFunction actionDeselectLight = new ActionCallFunction(deselectLight);
            ActionCallFunction actionSelectLight = new ActionCallFunction(selectLight);
            State stateEditLights = new State();

            TargetState stateSelectLight = new TargetState(stateEditLights, "stateSelectLight");
            stateSelectLight.addAction(actionSelectLight);

            TargetState stateDeselectLight = new TargetState(stateEditLights, "destateSelectLight");
            stateDeselectLight.addAction(actionDeselectLight);

            DecisionEditorMouseButtonPressed decisionMouseRightButton = new DecisionEditorMouseButtonPressed(stateDeselectLight, null, this, MouseButtons.Right);
            DecisionEditorMouseButtonPressed decisionMouseLeftButton = new DecisionEditorMouseButtonPressed(stateSelectLight, decisionMouseRightButton, this, MouseButtons.Left);
            ActionDecisionTree actionOnMouseClick = new ActionDecisionTree(decisionMouseLeftButton);

            ActionCallFunction actionShowGui = new ActionCallFunction(showLightGui);
            ActionCallFunction actionHideGui = new ActionCallFunction(hideLightGui);

            stateEditLights.addEntryAction(actionShowGui);
            stateEditLights.addAction(actionOnMouseClick);
            stateEditLights.addExitAction(actionHideGui);
            stateEditLights.addExitAction(actionDeselectLight);

            return stateEditLights;
        }

        private State createStateEditMesh()
        {
            ActionCallFunction actionShowGui = new ActionCallFunction(showMeshGui);
            ActionCallFunction actionHideGui = new ActionCallFunction(hideMeshGui);

            State stateEditMesh = new State();

            ActionDeselectMesh actionDeselectMesh = new ActionDeselectMesh(this);
            ActionSelectMesh actionSelectMesh = new ActionSelectMesh(this);

            TargetState stateSelectMesh = new TargetState(stateEditMesh, "stateSelectMesh");
            stateSelectMesh.addAction(actionSelectMesh);

            TargetState stateDeselectMesh = new TargetState(stateEditMesh, "destateSelectMesh");
            stateDeselectMesh.addAction(actionDeselectMesh);

            DecisionEditorMouseButtonPressed decisionMouseRightButton = new DecisionEditorMouseButtonPressed(stateDeselectMesh, null, this, MouseButtons.Right);
            DecisionEditorMouseButtonPressed decisionMouseLeftButton = new DecisionEditorMouseButtonPressed(stateSelectMesh, decisionMouseRightButton, this, MouseButtons.Left);

            ActionDecisionTree actionOnMouseClick = new ActionDecisionTree(decisionMouseLeftButton);

            stateEditMesh.addEntryAction(actionShowGui);
            stateEditMesh.addAction(actionOnMouseClick);
            stateEditMesh.addExitAction(actionHideGui);
            stateEditMesh.addExitAction(actionDeselectMesh);

            return stateEditMesh;
        }

        State createStateEditWaypoints()
        {
            State stateEditWaypoints = new State();

            ActionAddWaypoint actionAddWaypoint = new ActionAddWaypoint(this);

            TargetState stateKeyUp = new TargetState(stateEditWaypoints, "stateEditWaypoints");
            stateKeyUp.addAction(actionAddWaypoint);

            DecisionKeyDown decisionCKeyDown = new DecisionKeyDown(stateKeyUp, null, System.Windows.Input.Key.C);
            DecisionEditorMouseButtonPressed decisionMouseLeftButton = new DecisionEditorMouseButtonPressed(decisionCKeyDown, null, this, MouseButtons.Left);

            ActionDecisionTree actionMouseLeftDown = new ActionDecisionTree(decisionMouseLeftButton);

            stateEditWaypoints.addAction(actionMouseLeftDown);

            return stateEditWaypoints;
        }

        State createStateEditSpawns()
        {
            State state = new State();

            ActionCallFunction actionShowGui = new ActionCallFunction(showSpawnGui);
            ActionCallFunction actionHideGui = new ActionCallFunction(hideSpawnGui);

            ActionCallFunction actionSelectSpawn = new ActionCallFunction(selectSpawn);
            ActionCallFunction actionDeselectSpawn = new ActionCallFunction(deselectSpawn);

            TargetState stateSelectSpawn = new TargetState(state, "stateSelectSpawn");
            stateSelectSpawn.addAction(actionSelectSpawn);

            TargetState stateDeselectSpawn = new TargetState(state, "stateDeselectSpawn");
            stateDeselectSpawn.addAction(actionDeselectSpawn);

            DecisionEditorMouseButtonPressed decisionMouseRightButton = new DecisionEditorMouseButtonPressed(stateDeselectSpawn, null, this, MouseButtons.Right);
            DecisionEditorMouseButtonPressed decisionMouseLeftButton = new DecisionEditorMouseButtonPressed(stateSelectSpawn, decisionMouseRightButton, this, MouseButtons.Left);
            ActionDecisionTree actionOnMouseClick = new ActionDecisionTree(decisionMouseLeftButton);

            state.addEntryAction(actionShowGui);
            state.addExitAction(actionHideGui);
            state.addAction(actionOnMouseClick);

            return state;
        }

        State createStateEditJoints()
        {
            State state = new State();

            ActionCallFunction actionShowGui = new ActionCallFunction(showJointGui);
            ActionCallFunction actionHideGui = new ActionCallFunction(hideJointGui);

            state.addEntryAction(actionShowGui);
            state.addExitAction(actionHideGui);

            ActionDeselectJointOrMeshInstance actionDeselectJointOrMesh = new ActionDeselectJointOrMeshInstance(this);
            ActionSelectJointOrMeshInstance actionSelectJointOrMeshInstance = new ActionSelectJointOrMeshInstance(this);

            TargetState stateSelectJointOrMesh = new TargetState(state, "stateSelectJointOrMesh");
            stateSelectJointOrMesh.addAction(actionSelectJointOrMeshInstance);

            TargetState stateDeselectJointOrMesh = new TargetState(state, "stateDeselectJointOrMesh");
            stateDeselectJointOrMesh.addAction(actionDeselectJointOrMesh);

            DecisionEditorMouseButtonPressed decisionMouseRightButton = new DecisionEditorMouseButtonPressed(stateDeselectJointOrMesh, null, this, MouseButtons.Right);
            DecisionEditorMouseButtonPressed decisionMouseLeftButton = new DecisionEditorMouseButtonPressed(stateSelectJointOrMesh, decisionMouseRightButton, this, MouseButtons.Left);

            ActionDecisionTree actionOnMouseClick = new ActionDecisionTree(decisionMouseLeftButton);

            state.addAction(actionOnMouseClick);

            return state;
        }

        void createStateMachine()
        {
            m_selectItemStateMachine = new StateMachine();

            ConditionEditorState conditionEditorStateEditMesh = new ConditionEditorState(this, EditorState.EditMesh);
            ConditionEditorState conditionEditorStateEditLights = new ConditionEditorState(this, EditorState.EditLights);
            ConditionEditorState conditionEditorStateEditNavMesh = new ConditionEditorState(this, EditorState.EditNavMesh);
            ConditionEditorState conditionEditorStateEditWaypoints = new ConditionEditorState(this, EditorState.EditWaypoints);
            ConditionEditorState conditionEditorStateEditSpawns = new ConditionEditorState(this, EditorState.EditSpawns);
            ConditionEditorState conditionEditorStateEditJoints = new ConditionEditorState(this, EditorState.EditJoints);

            var stateEditNavMesh = createStateEditNavMesh();
            var stateEditMesh = createStateEditMesh();
            var stateEditLights = createStateEditLights();
            var stateEditWaypoints = createStateEditWaypoints();
            var stateEditSpawns = createStateEditSpawns();
            var stateEditJoints = createStateEditJoints();

            Transition transitionEditMesh = new Transition(conditionEditorStateEditMesh, stateEditMesh, "transitionEditMesh");
            Transition transitionEditNavMesh = new Transition(conditionEditorStateEditNavMesh, stateEditNavMesh, "transitionEditNavMesh");
            Transition transitionEditLights = new Transition(conditionEditorStateEditLights, stateEditLights, "transitionEditLights");
            Transition transitionEditWaypoints = new Transition(conditionEditorStateEditWaypoints, stateEditWaypoints, "transitionEditWaypoints");
            Transition transitionEditSpawns = new Transition(conditionEditorStateEditSpawns, stateEditSpawns, "transitionEditSpawns");
            Transition transitionEditJoints = new Transition(conditionEditorStateEditJoints, stateEditJoints, "transitionEditJoints");

            stateEditNavMesh.addTransition(transitionEditMesh);
            stateEditNavMesh.addTransition(transitionEditLights);
            stateEditNavMesh.addTransition(transitionEditWaypoints);
            stateEditNavMesh.addTransition(transitionEditSpawns);
            stateEditNavMesh.addTransition(transitionEditJoints);

            stateEditMesh.addTransition(transitionEditNavMesh);
            stateEditMesh.addTransition(transitionEditLights);
            stateEditMesh.addTransition(transitionEditWaypoints);
            stateEditMesh.addTransition(transitionEditSpawns);
            stateEditMesh.addTransition(transitionEditJoints);

            stateEditLights.addTransition(transitionEditNavMesh);
            stateEditLights.addTransition(transitionEditMesh);
            stateEditLights.addTransition(transitionEditWaypoints);
            stateEditLights.addTransition(transitionEditSpawns);
            stateEditLights.addTransition(transitionEditJoints);

            stateEditWaypoints.addTransition(transitionEditMesh);
            stateEditWaypoints.addTransition(transitionEditNavMesh);
            stateEditWaypoints.addTransition(transitionEditLights);
            stateEditWaypoints.addTransition(transitionEditSpawns);
            stateEditWaypoints.addTransition(transitionEditJoints);

            stateEditSpawns.addTransition(transitionEditMesh);
            stateEditSpawns.addTransition(transitionEditNavMesh);
            stateEditSpawns.addTransition(transitionEditLights);
            stateEditSpawns.addTransition(transitionEditWaypoints);
            stateEditSpawns.addTransition(transitionEditJoints);

            stateEditJoints.addTransition(transitionEditNavMesh);
            stateEditJoints.addTransition(transitionEditMesh);
            stateEditJoints.addTransition(transitionEditLights);
            stateEditJoints.addTransition(transitionEditWaypoints);
            stateEditJoints.addTransition(transitionEditSpawns);

            State emptyState = new State();
            emptyState.addTransition(transitionEditMesh);
            emptyState.addTransition(transitionEditNavMesh);
            emptyState.addTransition(transitionEditLights);
            emptyState.addTransition(transitionEditWaypoints);
            emptyState.addTransition(transitionEditSpawns);
            emptyState.addTransition(transitionEditJoints);

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

        void showSpawnGui()
        {
            toolStripButtonCreateSpawn.Visible = true;
        }

        void hideSpawnGui()
        {
            toolStripButtonCreateSpawn.Visible = false;
        }

        void showJointGui()
        {
            toolStripButtonCreateJoint.Visible = true;
            removeJointToolStripMenuItem.Visible = true;
            addJointToolStripMenuItem.Visible = true;
        }

        void hideJointGui()
        {
            toolStripButtonCreateJoint.Visible = false;
            removeJointToolStripMenuItem.Visible = false;
            addJointToolStripMenuItem.Visible = false;
        }

        void getLightData()
        {
            Float3 position;
            position.x = position.y = position.z = 0;
            NativeMethods.getSelectLightPosition(ref position);
            var falloff = NativeMethods.getSelectLightFalloff();
            var lumen = NativeMethods.getSelectLightLumen();

            setNumericUpDownLightPosition(position);
            numericUpDownLightFalloff.Value = (decimal)falloff;
            numericUpDownLightLumen.Value = (decimal)lumen;

            groupBoxLight.Show();
        }

        public void selectLight()
        {
            if (NativeMethods.selectLight(m_mouseX, m_mouseY))
            {
                getLightData();
            }
        }

        public void deselectLight()
        {
            NativeMethods.deselectLight();
            groupBoxLight.Hide();
        }

        void selectSpawn()
        {
            uint id = 0;
            if (NativeMethods.selectSpawn(m_mouseX, m_mouseY, ref id))
            {
                Float3 position;
                position.x = position.y = position.z = 0;
                NativeMethods.getSpawnPosition(id, ref position);
                var spawnType = NativeMethods.getSpawnType(id);

                numericUpDownSpawnPosX.Value = (decimal)position.x;
                numericUpDownSpawnPosY.Value = (decimal)position.y;
                numericUpDownSpawnPosZ.Value = (decimal)position.z;

                numericUpDownSpawnType.Value = (decimal)spawnType;

                m_selectedSpawn = id;

                groupBoxSpawn.Show();
            }
        }

        void deselectSpawn()
        {
            groupBoxSpawn.Hide();
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
            m_sortedMeshes.Clear();
            m_meshNode.Nodes.Clear();

            m_meshInstanceDataControl.clearMeshEntries();

            var meshCount = NativeMethods.getMeshCount();
            for (uint i = 0; i < meshCount; ++i)
            {
                var meshName = NativeMethods.getMeshName(i);
                var meshSid = NativeMethods.getMeshSid(i);

                insertMesh(meshSid, meshName);
            }
        }

        void insertMesh(ulong sid, string name)
        {
            var meshEntry = new EditorNodeEntry(sid, s_typeMesh, name);
            EditorEntry entry = new EditorEntry(name, sid);

            m_meshNode.Nodes.Add(meshEntry);
            m_sortedMeshes.Add(meshEntry.sid, entry);

            m_meshInstanceDataControl.addMeshEntry(entry);
        }

        void addMesh(ulong sid, string name)
        {
            EditorEntry entry;
            if (!m_sortedMeshes.TryGetValue(sid, out entry))
            {
                insertMesh(sid, name);
            }
        }

        void addSceneMeshInstances()
        {
            m_sortedMeshInstanceNodes.Clear();
            m_meshInstanceNode.Nodes.Clear();

            m_jointDataControl.clearMeshInstances();

            var meshInstanceCount = NativeMethods.getMeshInstanceCount();
            for (uint i = 0; i < meshInstanceCount; ++i)
            {
                var sid = NativeMethods.getMeshInstanceSid(i);
                var meshInstanceName = NativeMethods.getMeshInstanceNameIndex(i);

                insertMeshInstance(sid, meshInstanceName);
            }
        }

        void addSceneAnimations()
        {
            m_sortedAnimations.Clear();
            m_animationsNode.Nodes.Clear();
            m_meshInstanceDataControl.clearAnimationEntries();

            var count = NativeMethods.getAnimationCount();
            for (uint i = 0; i < count; ++i)
            {
                var sid = NativeMethods.getAnimationSidIndex(i);
                var name = NativeMethods.getAnimationNameIndex(i);

                insertAnimation(sid, name);
            }
        }

        void insertMeshInstance(ulong sid, string name)
        {
            var nodeEntry = new EditorNodeEntry(sid, s_typeMeshInstance, name);

            m_sortedMeshInstanceNodes.Add(nodeEntry.sid, nodeEntry);
            m_meshInstanceNode.Nodes.Add(nodeEntry);

            EditorEntry entry = new EditorEntry(name, sid);
            m_jointDataControl.addMeshInstance(entry);
        }

        public void addMeshInstance(ulong sid)
        {
            var meshInstanceName = NativeMethods.getMeshInstanceName(sid);

            insertMeshInstance(sid, meshInstanceName);
        }

        public void removeMeshInstance(ulong sid)
        {
            EditorNodeEntry entry;
            if (m_sortedMeshInstanceNodes.TryGetValue(sid, out entry))
            {
                m_sortedMeshInstanceNodes.Remove(sid);
                m_meshInstanceNode.Nodes.Remove(entry);
            }
        }

        void addSceneMaterials()
        {
            m_meshInstanceDataControl.clearMaterialEntries();
            m_sortedMaterials.Clear();
            m_materialNode.Nodes.Clear();

            var count = NativeMethods.getMaterialCount();
            for (uint i = 0; i < count; ++i)
            {
                var name = NativeMethods.getMaterialNameIndex(i);
                var sid = NativeMethods.getMaterialSid(i);

                insertMaterial(sid, name);
            }
        }

        void insertMaterial(ulong sid, string name)
        {
            var nodeEntry = new EditorNodeEntry(sid, s_typeMaterial, name);
            var entry = new EditorEntry(name, sid);

            m_materialNode.Nodes.Add(nodeEntry);
            m_sortedMaterials.Add(nodeEntry.sid, entry);

            m_meshInstanceDataControl.addMaterialEntry(entry);
        }

        void addMaterial(ulong sid, string name)
        {
            EditorEntry entry;
            if (!m_sortedMaterials.TryGetValue(sid, out entry))
            {
                insertMaterial(sid, name);
            }
        }

        void addAnimation(ulong sid, string name)
        {
            EditorEntry entry;
            if (!m_sortedAnimations.TryGetValue(sid, out entry))
            {
                insertAnimation(sid, name);
            }
        }

        void insertAnimation(ulong sid, string name)
        {
            var nodeEntry = new EditorNodeEntry(sid, s_typeAnimation, name);
            var entry = new EditorEntry(name, sid);

            m_animationsNode.Nodes.Add(nodeEntry);
            m_sortedAnimations.Add(nodeEntry.sid, entry);

            m_meshInstanceDataControl.addAnimationEntry(entry);
        }

        void addSceneJoints()
        {
            m_jointsNode.Nodes.Clear();

            var count = NativeMethods.getJointCount();
            for (uint i = 0; i < count; ++i)
            {
                insertJoint(i);
            }
        }

        public void insertJoint(uint index)
        {
            var name = "Joint" + index;
           // var sid = NativeMethods.getSid(name);
            var nodeEntry = new EditorNodeEntry(index, s_typeJoint, name);
            m_jointsNode.Nodes.Add(nodeEntry);
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
                    addMaterial(sid, str);
                    // m_materialNode.Nodes.Add(new EditorNodeEntry(sid, s_typeMaterial, str));
                }
                else if (type == s_typeScene)
                {
                    addSceneMeshes();
                    addSceneMaterials();
                    addSceneMeshInstances();
                    addSceneAnimations();
                    addSceneJoints();
                }
                else if (type == s_typeAnimation)
                {
                    addAnimation(sid, str);
                }
                else
                {
                    Console.Write("loaded unknown file type\n");
                }
            }
            else
            {
                Console.Write("could not find file {0}\n", sid);
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

            importFile(filename, ext);
        }

        public void importFile(string filename, string extension)
        {
            UInt64 sid = NativeMethods.getSid(filename);

            if (extension == ".mesh")
            {
                m_requestedFiles.Add(sid, filename);

                NativeMethods.loadFile(filename, s_typeMesh, EditorForm.loadFileCallback);
            }
            else if (extension == ".material")
            {
                m_requestedFiles.Add(sid, filename);

                NativeMethods.loadFile(filename, s_typeMaterial, EditorForm.loadFileCallback);
            }
            else if (extension == ".fbx")
            {
                m_requestedFiles.Add(sid, filename);

                NativeMethods.loadFile(filename, s_typeFbx, EditorForm.loadFileCallback);
            }
            else if (extension == ".animation")
            {
                m_requestedFiles.Add(sid, filename);

                NativeMethods.loadFile(filename, s_typeAnimation, EditorForm.loadFileCallback);
            }
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

            m_meshInstanceDataControl.setTranslation(translation);
            m_meshInstanceDataControl.setRotation(rotation);
        }

        void updateGuiSelectedMeshInstance(ulong sid)
        {
            getMeshInstanceTransform(sid);

            m_meshInstanceDataControl.setInstanceName(NativeMethods.getSelectedMeshInstanceName());

            var materialSid = NativeMethods.getMeshInstanceMaterialSid(sid);
            var meshSid = NativeMethods.getMeshInstanceMeshSid(sid);
            var animSid = NativeMethods.getMeshInstanceAnimation(sid);
            var rigidBodyType = NativeMethods.getMeshInstanceRigidBodyType(sid);

            m_meshInstanceDataControl.setRigidBodyType(rigidBodyType);

            EditorEntry entry;
            if (m_sortedMeshes.TryGetValue(meshSid, out entry))
            {
                m_meshInstanceDataControl.setMesh(entry);
            }
            else
            {
                Console.WriteLine("Error getting instance mesh name: {0}", meshSid);
            }

            if (m_sortedMaterials.TryGetValue(materialSid, out entry))
            {
                m_meshInstanceDataControl.setMaterial(entry);
            }
            else
            {
                Console.WriteLine("Error getting instance material name: {0}", materialSid);
            }

            if (m_sortedAnimations.TryGetValue(animSid, out entry))
            {
                m_meshInstanceDataControl.setAnimation(entry);
            }
            else
            {
                m_meshInstanceDataControl.setAnimation(null);
                //Console.WriteLine("Error getting instance animation name: {0}", animSid);
            }
        }

        public void setMeshInstanceRigidBodyType(uint type)
        {
            var sid = NativeMethods.getSelectedMeshInstanceSid();
            NativeMethods.setMeshInstanceRigidBodyType(sid, type);
        }

        void showMeshGui()
        {
            toolStripButtonCreateMeshInstance.Visible = true;
        }

        void hideMeshGui()
        {
            toolStripButtonCreateMeshInstance.Visible = false;
        }

        public void setMeshInstancePosition(Float3 translation)
        {
            var sid = NativeMethods.getSelectedMeshInstanceSid();

            if (m_selectedMeshInstanceSid == sid)
            {
                ActionSetMeshInstancePosition actionSetMeshInstancePosition = new ActionSetMeshInstancePosition(sid, translation);
                runAction(actionSetMeshInstancePosition);
            }
        }

        public void setMeshInstanceRotation(Float3 rotation)
        {
            var sid = NativeMethods.getSelectedMeshInstanceSid();

            if (m_selectedMeshInstanceSid == sid)
            {
                ActionSetMeshInstanceRotation actionSetMeshInstanceRotation = new ActionSetMeshInstanceRotation(sid, rotation);
                runAction(actionSetMeshInstanceRotation);
            }
        }

        void onGetMeshPhysxType(ulong sid, string name)
        {
            var type = NativeMethods.getMeshPhysxType(sid);

            m_meshInfoControl.setPhysxType(type);

            m_meshInfoControl.setName(name);
            m_meshInfoControl.Show();
        }

        public void setMeshPhysxType(uint type)
        {
            EditorNodeEntry entry = (EditorNodeEntry)treeView_entities.SelectedNode;
            if (entry.type == s_typeMesh)
            {
                var currentType = NativeMethods.getMeshPhysxType(entry.sid);

                if (currentType != type)
                {
                    NativeMethods.setMeshPhysxType(entry.sid, type);
                }
            }
        }

        private void treeView_entities_AfterSelect(object sender, TreeViewEventArgs e)
        {
            m_meshInfoControl.Hide();
            try
            {
                EditorNodeEntry entry = (EditorNodeEntry)treeView_entities.SelectedNode;

                if (entry.type == s_typeMeshInstance)
                {
                    uint index = (uint)entry.Index;

                    var sid = entry.sid;
                    //NativeMethods.getMeshInstanceSidRaytrace(index);
                    NativeMethods.setSelectedMeshInstance(sid);

                    //var newSelectedSid = NativeMethods.getSelectedMeshInstanceSid();
                    // if (newSelectedSid != 0)
                    // {
                    updateGuiSelectedMeshInstance(sid);

                    m_selectedMeshInstanceSid = sid;

                    m_meshInstanceDataControl.Show();
                    //}
                }
                else if (entry.type == s_typeMesh)
                {
                    onGetMeshPhysxType(entry.sid, entry.Text);
                }
                else if(entry.type == s_typeJoint)
                {
                    var index = entry.sid;
                    setSelectedJoint((uint)index);
                    //m_jointDataControl.Show();
                }
            }
            catch
            {
                m_meshInstanceDataControl.Hide();
            }
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

        void setSelectedMeshInstanceNode(ulong instanceSid)
        {
            //var instanceSid = NativeMethods.getSelectedMeshInstanceSid();

            EditorNodeEntry entry;
            if (m_sortedMeshInstanceNodes.TryGetValue(instanceSid, out entry))
            {
                treeView_entities.SelectedNode = entry;
            }
        }

        public bool selectMesh(int mouseX, int mouseY)
        {
            // Float3 translation;
            // translation.x = translation.y = translation.z = 0.0f;
            var sid = NativeMethods.getMeshInstanceSidRaytrace(mouseX, mouseY);
            bool result = (sid == 0) ? false : true;
            selectMesh(sid);

            return result;
        }

        public bool selectMesh(int mouseX, int mouseY, out ulong selectedSid)
        {
            selectedSid = NativeMethods.getSelectedMeshInstanceSid();

            return selectMesh(mouseX, mouseY);
        }

        public void selectMesh(ulong sid)
        {
            if (sid != 0)
            {
                setSelectedMeshInstanceNode(sid);
            }
        }

        public ulong deselectMesh()
        {
            var sid = NativeMethods.deselectMeshInstance();

            treeView_entities.SelectedNode = null;
            m_meshInstanceDataControl.Hide();

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
                m_actionListHead.setNext(actionList);
                m_actionListHead = actionList;

                var node = clonedAction.toNode();
                treeViewActionList.Nodes.Add(node);
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
            if (!m_keyDownAlt)
            {
                m_mouseX = e.X;
                m_mouseY = e.Y;
                m_lastClickedMouseButton = e.Button;
                m_isMouseDown = true;
            }
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
                if (m_editorState == EditorState.EditNavMesh)
                {
                    Float3 position;
                    position.x = position.y = position.z = 0;
                    NativeMethods.getSelectNavMeshVertexPosition(ref position);
                    NativeMethods.removeNavMeshVertex(ref position);
                }
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
            //openFileDialog_importAsset.ShowDialog();
            m_fileBrowser.ShowDialog();
        }

        private void openFileDialog1_loadScene_FileOk(object sender, CancelEventArgs e)
        {
            string filename = openFileDialog1_loadScene.SafeFileName;
            string ext = System.IO.Path.GetExtension(filename);

            UInt64 sid = NativeMethods.getSid(filename);

            if (ext == ".scene")
            {
                string value;
                if (!m_requestedFiles.TryGetValue(sid, out value))
                {
                    m_requestedFiles.Add(sid, filename);
                }

                NativeMethods.loadFile(filename, s_typeScene, EditorForm.loadFileCallback);
                m_currentSceneFileName = filename;

                this.Text = "vxEditor: " + m_currentSceneFileName;
            }
        }

        private void panel_render_Paint(object sender, PaintEventArgs e)
        {
        }

        private void saveFileDialog_scene_FileOk(object sender, CancelEventArgs e)
        {

            string filenameWithFullPath = saveFileDialog_scene.FileName;
            string filename_with_ext = System.IO.Path.GetFileName(filenameWithFullPath);
            string ext = System.IO.Path.GetExtension(filenameWithFullPath);
            if (ext == ".scene")
            {
                NativeMethods.saveScene(filename_with_ext);
                m_currentSceneFileName = filename_with_ext;
                this.Text = "vxEditor: " + m_currentSceneFileName;
            }
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
            else if (selectedString == s_textEditWaypoints)
            {
                m_editorState = EditorState.EditWaypoints;
            }
            else if (selectedString == s_textEditJoints)
            {
                m_editorState = EditorState.EditJoints;
            }
        }

        public MouseButtons getLastClickedMouseButton()
        {
            return m_lastClickedMouseButton;
        }

        private void comboBox_selectEditorMode_Click(object sender, EventArgs e)
        {
            var item = comboBox_selectEditorMode.SelectedItem;
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
            getLightData();
        }

        public void setMeshInstanceMaterial(ulong materialSid)
        {
            var instanceSid = NativeMethods.getSelectedMeshInstanceSid();

            if (materialSid != 0 && instanceSid != 0 && m_selectedMeshInstanceSid == instanceSid)
            {
                var oldSid = NativeMethods.getMeshInstanceMaterialSid(instanceSid);

                if (oldSid != materialSid)
                {
                    var action = new ActionSetMeshInstanceMaterial(instanceSid, oldSid, materialSid);
                    runAction(action);
                }
            }
        }

        public void setMeshInstanceAnimation(ulong animSid)
        {
            var instanceSid = NativeMethods.getSelectedMeshInstanceSid();

            if (animSid != 0 && instanceSid != 0 && m_selectedMeshInstanceSid == instanceSid)
            {
                var oldSid = NativeMethods.getMeshInstanceAnimation(instanceSid);

                if (oldSid != animSid)
                {
                    var action = new ActionSetMeshInstanceAnimation(instanceSid, oldSid, animSid);
                    runAction(action);
                }
            }
        }

        public void onRenameMeshInstance(ulong oldSid, ulong newSid, string newName)
        {
            EditorNodeEntry nodeEntry;
            if (m_sortedMeshInstanceNodes.TryGetValue(oldSid, out nodeEntry))
            {
                m_sortedMeshInstanceNodes.Remove(oldSid);
                nodeEntry.sid = newSid;
                nodeEntry.Text = newName;
                m_sortedMeshInstanceNodes.Add(newSid, nodeEntry);
            }
        }

        private void toolStripButtonCreateMeshInstance_Click(object sender, EventArgs e)
        {
            var action = new ActionCreateMeshInstance(this);
            runAction(action);
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

        public void setMeshInstanceMesh(ulong meshInstanceSid, ulong newMeshSid)
        {
            if (m_selectedMeshInstanceSid == meshInstanceSid)
            {
                var oldMeshSid = NativeMethods.getMeshInstanceMeshSid(meshInstanceSid);

                if (oldMeshSid != newMeshSid)
                {
                    var action = new ActionSetMeshInstanceMesh(meshInstanceSid, oldMeshSid, newMeshSid);
                    runAction(action);
                }
            }
        }

        private void treeViewActionList_AfterSelect(object sender, TreeViewEventArgs e)
        {

        }

        private void Form1_MouseMove(object sender, MouseEventArgs e)
        {
            updateStateMachine();
        }

        private void numericUpDownLightFalloff_ValueChanged(object sender, EventArgs e)
        {
            float value = (float)numericUpDownLightFalloff.Value;
            NativeMethods.setSelectLightFalloff(value);
        }

        private void numericUpDownLightLumen_ValueChanged(object sender, EventArgs e)
        {
            float value = (float)numericUpDownLightLumen.Value;
            NativeMethods.setSelectLightLumen(value);
        }

        private void numericUpDownSpawnType_ValueChanged(object sender, EventArgs e)
        {
            if (m_editorState == EditorState.EditSpawns)
            {
                var value = numericUpDownSpawnType.Value;
                NativeMethods.setSpawnType(m_selectedSpawn, (uint)value);
            }
        }

        void setSpawnPosition()
        {
            if (m_editorState == EditorState.EditSpawns)
            {
                Float3 position;
                position.x = (float)numericUpDownSpawnPosX.Value;
                position.y = (float)numericUpDownSpawnPosY.Value;
                position.z = (float)numericUpDownSpawnPosZ.Value;
                NativeMethods.setSpawnPosition(m_selectedSpawn, ref position);
            }
        }

        private void numericUpDownSpawnPosX_ValueChanged(object sender, EventArgs e)
        {
            setSpawnPosition();
        }

        private void numericUpDownSpawnPosY_ValueChanged(object sender, EventArgs e)
        {
            setSpawnPosition();
        }

        private void numericUpDownSpawnPosZ_ValueChanged(object sender, EventArgs e)
        {
            setSpawnPosition();
        }

        private void toolStripButtonCreateSpawn_Click(object sender, EventArgs e)
        {
            NativeMethods.addSpawn();
        }

        void createJoint()
        {
            var selectedSid = NativeMethods.getSelectedMeshInstanceSid();
            NativeMethods.addJoint(selectedSid);

            addSceneJoints();
        }

        private void toolStripButtonCreateJoint_Click(object sender, EventArgs e)
        {
            createJoint();
        }

        public void setSelectedJoint(uint index)
        {
            m_jointDataControl.setSelectedJoint(index);
            m_jointDataControl.Show();
        }

        public void deselectJoint()
        {
            m_jointDataControl.Hide();
        }

        private void removeJointToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_jointDataControl.removeJoint();
        }

        private void addJointToolStripMenuItem_Click(object sender, EventArgs e)
        {
            createJoint();
        }
    }
}
