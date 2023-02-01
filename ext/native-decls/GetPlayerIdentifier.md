---
ns: CFX
apiset: server
---
## GET_PLAYER_IDENTIFIER

```c
char* GET_PLAYER_IDENTIFIER(char* playerSrc, int identifier);
```


## Parameters
* **playerSrc**: 
* **identifier**: 

## Return value
A string value containing one of 8 different identifiers.

This value can be one of the following identifiers,
- ip (ipv4 address)
- xbl (xbox live account id)
- live (microsoft live account id)
- fivem (fivem account id)
- steam (steam account hexadecimal)
- license (current rockstar license)
- license2 (previously held rockstar license)
