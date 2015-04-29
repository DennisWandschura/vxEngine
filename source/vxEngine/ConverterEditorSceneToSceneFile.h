#pragma once

class SceneFile;
class EditorScene;

class ConverterEditorSceneToSceneFile
{
public:
	static void convert(const EditorScene &scene, SceneFile* sceneFile);
};