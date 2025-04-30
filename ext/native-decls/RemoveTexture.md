---
ns: CFX
apiset: client
game: rdr3
---
## REMOVE_TEXTURE

```c
void REMOVE_TEXTURE(int textureId);
```

Removes the specified texture and remove it from the ped.
Unlike `0x6BEFAA907B076859` which only marks the texture as "can be reused" (and keeps it until will be reused), this function deletes it right away. Can fix some sync issues. `DOES_TEXTURE_EXIST` can be use to wait until fully unloaded by game
```lua
RemoveTexture(textureId)
while DoesTextureExist(textureId) do 
    Wait(0)
end
```

## Parameters
* **textureId**: texture id created by `0xC5E7204F322E49EB`.