---
ns: CFX
apiset: client
---
## CREATE_DUI

```c
long CREATE_DUI(char* url, int width, int height);
```

Creates a DUI browser. This can be used to draw on a runtime texture using CREATE\_RUNTIME\_TEXTURE\_FROM\_DUI\_HANDLE.

## Parameters
* **url**: The initial URL to load in the browser.
* **width**: The width of the backing surface.
* **height**: The height of the backing surface.

## Return value
A DUI object.
