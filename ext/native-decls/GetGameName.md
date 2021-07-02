---
ns: CFX
apiset: shared
---
## GET_GAME_NAME

```c
char* GET_GAME_NAME();
```

Returns the current game being executed.

Possible values:

| Return value | Meaning                        |
| ------------ | ------------------------------ |
| `fxserver`   | Server-side code ('Duplicity') |
| `fivem`      | FiveM for GTA V                |
| `libertym`   | LibertyM for GTA IV            |
| `redm`       | RedM for Red Dead Redemption 2 |

## Return value
The game the script environment is running in.