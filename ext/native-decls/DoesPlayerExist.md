---
ns: CFX
apiset: server
---
## DOES_PLAYER_EXIST

```c
BOOL DOES_PLAYER_EXIST(char* playerSrc);
```

Returns whether or not the player exists

## Return value
True if the player exists, false otherwise

## Examples

```lua
local deferralMessages = { "Isn't this just magical!", "We can defer all day!", "You'll get in eventually", "You're totally not going to sit here forever", "The Fruit Tree is a lie" }
AddEventHandler("playerConnecting", function(name, setKickReason, deferrals)
    local source = source
    deferrals.defer()

    Wait(0)


    local messageIndex = 0

    repeat
        Wait(2000)
        if messageIndex >= #deferralMessages then
            deferrals.done()
        else
            messageIndex = messageIndex + 1
        end
        deferrals.update(deferralMessages[messageIndex])
    until not DoesPlayerExist(source)
end)
```
