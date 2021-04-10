---
ns: CFX
apiset: shared
---
## REGISTER_COMMAND

```c
void REGISTER_COMMAND(char* commandName, func handler, BOOL restricted);
```

Registered commands can be executed by entering them in the client console (this works for client side and server side registered commands). Or by entering them in the server console/through an RCON client (only works for server side registered commands). Or if you use a supported chat resource, like the default one provided in the cfx-server-data repository, then you can enter the command in chat by prefixing it with a `/`.

Commands registered using this function can also be executed by resources, using the [`ExecuteCommand` native](#_0x561C060B).

The restricted bool is not used on the client side. Permissions can only be checked on the server side, so if you want to limit your command with an ace permission automatically, make it a server command (by registering it in a server script).

**Example result**:

![](https://i.imgur.com/TaCnG09.png)


## Required Arguments
* **commandName**: The command you want to register.
* **handler**: A handler function that gets called whenever the command is executed.

## Optional Arguments
* **restricted**: If this is a server command and you set this to true, then players will need the command.yourCommandName ace permission to execute this command.

## Handler function parameters
* **playerSource**: The player font which executed the command.
* **args**: returns the string the player entered after the command.
* **rawCommand**: returns the string with the name of the executed command.

## Examples

```lua
-- (server side script)
-- Registers a command named 'ping'.
RegisterCommand("ping", function(playerSource, args, rawCommand)
    -- If the playerSource is > 0, then that means it must be a player.
    if (playerSource > 0) then
    
        -- result (using the default GTA:O chat theme) https://i.imgur.com/TaCnG09.png
        TriggerClientEvent("chat:addMessage", -1, {
            args = {
                GetPlayerName(playerSource),
                "PONG!"
            },
            color = { 5, 255, 255 }
        })
    
    -- If it's not a player, then it must be RCON, a resource, or the server console directly.
    else
        print("This command was executed by the server console, RCON client, or a resource.")
    end
end, false --[[this command is not restricted, everyone can use this.]])
```

```cs
RegisterCommand("ping", new Action<int, List<object>, string>((playerSource, args, rawCommand) =>
{
    if (playerSource > 0) // it's a player.
    {
        // Create a message object.
        dynamic messageObject = new ExpandoObject();
        // Set the message object args (message author, message content)
        messageObject.args = new string[] { GetPlayerName(playerSource.ToString()), "PONG!" };
        // Set the message color (r, g, b)
        messageObject.color = new int[] { 5, 255, 255 };

        // Trigger the client event with the message object on all clients.
        TriggerClientEvent("chat:addMessage", messageObject);
    }
    // It's not a player, so it's the server console, a RCON client, or a resource.
    else
    {
        Debug.WriteLine("This command was executed by the server console, RCON client, or a resource.");
    }
}), false /*This command is also not restricted, anyone can use it.*/ );
```
