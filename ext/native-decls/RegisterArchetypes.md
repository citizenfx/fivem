---
ns: CFX
apiset: client
game: gta5
---
## REGISTER_ARCHETYPES

```c
void REGISTER_ARCHETYPES(func factory);
```

**Experimental**: This native may be altered or removed in future versions of CitizenFX without warning.

Registers a set of archetypes with the game engine. These should match `CBaseArchetypeDef` class information from the game.

## Parameters
* **factory**: A function returning a list of archetypes.

## Examples

```lua
RegisterArchetypes(function()
	return {
		{
			flags = 32,
			bbMin = vector3(-39.99570000, -8.00155600, -2.56818800),
			bbMax = vector3(40.00439000, 7.99858000, 1.44575100),
			bsCentre = vector3(0.00434110, -0.00148870, -0.56121830),
			bsRadius = 40.84160000,
			name = 'my_asset',
			textureDictionary = 'my_asset',
			physicsDictionary = 'my_asset',
			assetName = 'my_asset',
			assetType = 'ASSET_TYPE_DRAWABLE',
			lodDist = 450.45,
			specialAttribute = 0
		}
	}
end)
```