---
ns: CFX
apiset: shared
---
## EXECUTE_COMMAND

```c
void EXECUTE_COMMAND(char* commandString);
```

Depending on your use case you may need to use `add_acl resource.<your_resource_name> command.<command_name> allow` to use this native in your resource.

## Parameters
* **commandString**: The entire command to execute

## Examples
```lua
RegisterCommand("leave_vehicle", function(source, args)
    local shouldLeaveInstantly = args[1] == "now"
    local ped = PlayerPedId()
    local vehicle = GetVehiclePedIsIn(ped)

    -- we're not in a vehicle we shouldn't do anything.
    if vehicle == 0 then return end

    if not shouldLeaveInstantly then
        TaskLeaveVehicle(ped, vehicle, 0)
    else
        TaskLeaveVehicle(ped, vehicle, 0)
    end
end, false)

ExecuteCommand("leave_vehicle now")
```
