---
ns: CFX
apiset: client
---
## SEND_DUI_KEY_DOWN

```c
void SEND_DUI_KEY_DOWN(long duiObject, char* keyPressed);
```

Injects a "Key Down" event into the DUI browser

## Parameters
* **duiObject**: The DUI browser handle.
* **keyPressed**: the key to inject into the browser

> Currently Supports Ascii characters

## Example

```lua
CreateThread(function()
    local dui = CreateDui("https://forum.cfx.re", 1920, 1080) -- Create A Dui Object
    while true do
        Wait(0) -- Wait Every Tick.
        --[[
            Drawing Dui Logic
        ]]
        if IsControlJustPressed(0, 38) then -- E key Pressed
            SendDuiKeyDown(dui, "e") -- Tell the Dui object the key was pressed
        end
         if IsControlJustReleased(0, 38) then -- E key Released
            SendDuiKeyUp(dui, "e") -- Tell the Dui object the key was released
        end
    end
end)
```
