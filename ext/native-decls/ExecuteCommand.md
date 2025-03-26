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
* **commandString**: 

## Examples

```lua
Citizen.CreateThread(function()
  -- stop the server after 1 minute
  Citizen.Wait(60000)
  ExecuteCommand("quit Shortlived")
end)
```
