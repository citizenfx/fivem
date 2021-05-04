#pragma once

#ifndef IS_FXSERVER
enum class WorldEditorMode
{
	TRANSLATE,
	ROTATE,
	SCALE,
};

struct WorldEditorControls
{
	bool gizmoLocal;
	bool gizmoSelect;

	WorldEditorMode gizmoMode;

	float mouseX;
	float mouseY;
};
#endif
