---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX_BY_NAME

```c
int GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX_BY_NAME(char* name);
```

Gets the plate index of a number plate style by its `TextureSetName`, for use with [SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX](#_0x9088EB5A43FFB0A1).

This works for both the game's own plate styles and add-on plate styles from `VEHICLE_PLATES_FILE` data files, so scripts don't have to hard-code plate indices.

### Add-on plate styles

Resources can add plate styles with a `VEHICLE_PLATES_FILE` data file, in the same format as the `VehiclePlates` node in `carcols.ymt`, with these additions:

* `TextureDictionary` (required): the streamed texture dictionary (`.ytd`) containing the plate textures.
* `Index` (optional): the plate index to use. When it is left out, the first free index after the game's plate styles is used.

`NormalMapName` and the font settings are optional, and default to the game's default plate style.

Plate indices are synchronized over the network as numbers, so every client needs to assign the same index to the same plate style. Indices that are assigned automatically depend on the order in which plate files are loaded, which can differ for players who join after a plate resource was restarted. Set an `Index` (for example, `100` and up) when plate resources may be restarted while players are connected.

```lua
-- fxmanifest.lua
data_file 'VEHICLE_PLATES_FILE' 'plates.meta'
files { 'plates.meta' }
```

```xml
<!-- plates.meta, with plates.ytd in the resource's stream folder -->
<VehiclePlates>
  <Textures>
    <Item>
      <TextureSetName>Texas Classic</TextureSetName>
      <TextureDictionary>plates</TextureDictionary>
      <DiffuseMapName>texas_plate</DiffuseMapName>
      <NormalMapName>texas_plate_n</NormalMapName>
      <FontExtents x="0.080000" y="0.355000" z="0.920000" w="0.816000" />
      <MaxLettersOnPlate x="7.000000" y="1.000000" />
      <FontColor value="0xFF8A1C1C" />
      <FontOutlineColor value="0xFFFFFFFF" />
      <IsFontOutlineEnabled value="false" />
      <FontOutlineMinMaxDepth x="0.475000" y="0.500000" />
      <Index value="100" />
    </Item>
  </Textures>
</VehiclePlates>
```

## Parameters
* **name**: The `TextureSetName` of the plate style.

## Return value
The plate index, or `-1` if no plate style with this name exists.

## Examples
```lua
local index = GetVehicleNumberPlateTextIndexByName('Texas Classic')

if index ~= -1 then
    SetVehicleNumberPlateTextIndex(GetVehiclePedIsIn(PlayerPedId(), false), index)
end
```
