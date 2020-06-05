---
ns: CFX
apiset: client
game: gta5
---
## SET_MINIMAP_COMPONENT_POSITION

```c
void SET_MINIMAP_COMPONENT_POSITION(char* name, char* alignX, char* alignY, float posX, float posY, float sizeX, float sizeY);
```

Overrides the minimap component data (from `common:/data/ui/frontend.xml`) for a specified component.

## Parameters
* **name**: The name of the minimap component to override.
* **alignX**: Equivalent to the `alignX` field in `frontend.xml`.
* **alignY**: Equivalent to the `alignY` field in `frontend.xml`.
* **posX**: Equivalent to the `posX` field in `frontend.xml`.
* **posY**: Equivalent to the `posY` field in `frontend.xml`.
* **sizeX**: Equivalent to the `sizeX` field in `frontend.xml`.
* **sizeY**: Equivalent to the `sizeY` field in `frontend.xml`.
