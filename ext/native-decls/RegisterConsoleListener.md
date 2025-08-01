---
ns: CFX
apiset: server
---
## REGISTER_CONSOLE_LISTENER

```c
void REGISTER_CONSOLE_LISTENER(func listener, char* channelFilter, char* messageFilter);
```

Registers a listener for console output messages.
You can optionally provide filters to only receive messages from specific channels or containing certain text patterns.

## Parameters
* **listener**: A function of `(channel: string, message: string) => void`. The message might contain `\n`.
* **channelFilter**: A regular expression string to filter messages by channel. If `null`, `undefined`, or `""`, all channels are accepted.
* **messageFilter**: A regular expression string to filter messages by content. If `null`, `undefined`, or `""`, all messages are accepted.

## Examples

```lua
RegisterConsoleListener(function(channel, message)
    local cleaned = message:gsub("SCRIPT ERROR:%s*", ""):gsub("ERROR:%s*", "")
    print(("Error detected in '%s': %s"):format(channel, cleaned))
end,
-- Channel filter: accept all channels
nil,
-- Message filter: match any message containing "error" (case-insensitive)
"(SCRIPT ERROR:|ERROR:)")
```
