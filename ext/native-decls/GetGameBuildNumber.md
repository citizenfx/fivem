---
ns: CFX
apiset: shared
---
## GET_GAME_BUILD_NUMBER

```c
int GET_GAME_BUILD_NUMBER();
```

Returns the internal build number of the current game being executed.

Possible values:

* FiveM
  * 1604
  * 2060
  * 2189
  * 2372
  * 2545
  * 2612
  * 2699
  * 2802
  * 2944
  * 3095
  * 3258
  * 3323
  * 3407
  * 3570
* RedM
  * 1311
  * 1355
  * 1436
  * 1491
* LibertyM
  * 43
* FXServer
  * 0

## Return value
The build number, or **0** if no build number is known.
