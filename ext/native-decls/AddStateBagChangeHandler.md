---
ns: CFX
apiset: shared
---
## ADD_STATE_BAG_CHANGE_HANDLER

```c
int ADD_STATE_BAG_CHANGE_HANDLER(char* keyFilter, char* bagFilter, func handler);
```

Adds a handler for changes to a state bag.

The function called expects to match the following signature:

```ts
function StateBagChangeHandler(bagName: string, key: string, value: any, reserved: number, replicated: boolean);
```

* **bagName**: The internal bag ID for the state bag which changed. This is usually `player:Source`, `entity:NetID`
  or `localEntity:Handle`.
* **key**: The changed key.
* **value**: The new value stored at key. The old value is still stored in the state bag at the time this callback executes.
* **reserved**: Currently unused.
* **replicated**: Whether the set is meant to be replicated.

At this time, the change handler can't opt to reject changes.

## Parameters
* **keyFilter**: The key to check for, or null.
* **bagFilter**: The bag ID to check for, or null.
* **handler**: The handler function.

## Return value
A cookie to remove the change handler.
