---
ns: CFX
apiset: server
---
## GET_PLAYER_IDENTIFIER

```c
char* GET_PLAYER_IDENTIFIER(char* playerSrc, int identiferIndex);
```

To get the number of identifiers, use [GET_NUM_PLAYER_IDENTIFIERS](#_0xFF7F66AB)

To get a specific type of identifier, use [GET_PLAYER_IDENTIFIER_BY_TYPE](#_0xA61C8FC6)

## Parameters
* **playerSrc**: 
* **identiferIndex**: 

## Return value
Returns the identifier at the specific index, if out of bounds returns `null`
