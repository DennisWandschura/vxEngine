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
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace LevelEditor
{
    public struct Float3
    {
        public float x, y, z;

        public Float3(float x, float y, float z) { this.x = x; this.y = y; this.z = z; }

        public void zero()
        {
            this.x = 0;
            this.y = 0;
            this.z = 0;
        }
    };

    public struct uint3
    {
        public uint x, y, z;

        public uint3(uint x, uint y, uint z) { this.x = x; this.y = y; this.z = z; }

        public void zero()
        {
            this.x = 0;
            this.y = 0;
            this.z = 0;
        }
    }

    static class NativeMethods
    {
        const string m_libPath = "../../../lib/vs2013/";
        const string m_dllName = "vxEngine_vs12_d.dll";

        public delegate void LoadFileCallback(ulong sid, uint type);

        // bool initialize(intptr_t hwndPanel, intptr_t hwndTmp, unsigned int panelSizeX, unsigned int panelSizeY);
        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool initializeEditor(IntPtr hwndPanel, IntPtr hwndTmp, uint panelSizeX, uint panelSizeY, uint typeMesh, uint typeMaterial, uint typeScene, uint typeFbx, uint typeAnim);

        // void shutdown();
        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void shutdownEditor();

        //void render();
        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void frame();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void loadFile(string filename, uint type, LoadFileCallback f);

        // UINT64 getSid(const char *str);
        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern UInt64 getSid(string filename);

        // void saveScene(const char* name);
        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void saveScene(string filename);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void moveCamera(float dirX, float dirY, float dirZ);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void rotateCamera(float dirX, float dirY, float dirZ);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool selectNavMeshVertex(int x, int y);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool selectNavMeshVertexPosition(ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool selectNavMeshVertexIndex(uint i);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool multiSelectNavMeshVertex(int x, int y);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern uint deselectNavMeshVertex();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool addNavMeshVertex(int x, int y, ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void removeNavMeshVertex(ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool createNavMeshTriangleFromSelectedVertices(ref uint3 selected);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void createNavMeshTriangleFromIndices(ref uint3 indices);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void removeNavMeshTriangle();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool getSelectNavMeshVertexPosition(ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setSelectNavMeshVertexPosition(ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern uint getSelectedNavMeshCount();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern uint getMeshInstanceCount();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        public unsafe static extern string getMeshInstanceNameIndex(uint i);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        public unsafe static extern string getMeshInstanceName(ulong sid);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern ulong getMeshInstanceSid(uint i);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern ulong getSelectedMeshInstanceSid();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern ulong getMeshInstanceMeshSid(ulong instanceSid);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setMeshInstanceMeshSid(ulong instanceSid, ulong meshSid);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern ulong getMeshInstanceAnimation(ulong instanceSid);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setMeshInstanceAnimation(ulong instanceSid, ulong animSid);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool selectMeshInstance(int x, int y);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool selectMeshInstanceIndex(uint i);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool selectMeshInstanceSid(ulong sid);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern ulong deselectMeshInstance();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        public unsafe static extern string getSelectedMeshInstanceName();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setMeshInstancePosition(ulong sid, ref Float3 translation);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void getMeshInstancePosition(ulong sid, ref Float3 translation);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setMeshInstanceRotation(ulong sid, ref Float3 rotationDeg);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void getMeshInstanceRotation(ulong sid, ref Float3 rotationDeg);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern ulong getMeshInstanceMaterialSid(ulong instanceSid);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setMeshInstanceMaterial(ulong instanceSid, ulong materialSid);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool setMeshInstanceName(ulong instanceSid, string name);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern ulong createMeshInstance();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void removeMeshInstance(ulong sid);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void showNavmesh(bool b);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void showInfluenceMap(bool b);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void createLight();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool selectLight(int x, int y);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void deselectLight();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void getSelectLightPosition(ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setSelectLightPosition(ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern float getSelectLightFalloff();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setSelectLightFalloff(float falloff);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern float getSelectLightLumen();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setSelectLightLumen(float lumen);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void addSpawn();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool selectSpawn(int mouseX, int mouseY, ref uint id);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void getSpawnPosition(uint id, ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern uint getSpawnType(uint id);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setSpawnPosition(uint id, ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setSpawnType(uint id, uint type);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern uint getMeshCount();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        public unsafe static extern string getMeshName(uint i);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern ulong getMeshSid(uint i);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern uint getMaterialCount();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        public unsafe static extern string getMaterialNameIndex(uint i);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        public unsafe static extern string getMaterialName(ulong i);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern ulong getMaterialSid(uint i);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool addWaypoint(int x, int y, ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void addWaypointPosition(ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void removeWaypoint(ref Float3 position);
    }
}
