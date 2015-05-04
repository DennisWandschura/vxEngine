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

    static class NativeMethods
    {
        const string m_libPath = "d:/Users/dw/Documents/Visual Studio 2013/Projects/vxEngine/lib/";
        const string m_dllName = "vxEngine_d.dll";

        public delegate void LoadFileCallback(UInt64 sid, UInt32 type);

        // bool initialize(intptr_t hwndPanel, intptr_t hwndTmp, unsigned int panelSizeX, unsigned int panelSizeY);
        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool initializeEditor(IntPtr hwndPanel, IntPtr hwndTmp, uint panelSizeX, uint panelSizeY, uint typeMesh, uint typeMaterial, uint typeScene);

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
        public unsafe static extern bool multiSelectNavMeshVertex(int x, int y);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void deselectNavMeshVertex();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool addNavMeshVertex(int x, int y);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void deleteSelectedNavMeshVertex();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool createNavMeshTriangleFromSelectedVertices();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void getSelectNavMeshVertexPosition(ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void setSelectNavMeshVertexPosition(ref Float3 position);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern bool selectMesh(int x, int y);

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void deselectMesh();

        [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void updateSelectedMeshInstanceTransform(ref Float3 translation);

         [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void showNavmesh(bool b);

         [DllImport(m_libPath + m_dllName, CallingConvention = CallingConvention.Cdecl)]
         public unsafe static extern void showInfluenceMap(bool b);
    }
}
