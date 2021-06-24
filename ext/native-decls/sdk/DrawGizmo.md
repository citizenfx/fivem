---
ns: CFX
apiset: client
game: gta5
---
## DRAW_GIZMO

```c
BOOL DRAW_GIZMO(long matrixPtr, char* id);
```

Draws a gizmo. This function supports SDK infrastructure and is not intended to be used directly from your code.

This should be used from JavaScript or another language supporting mutable buffers like ArrayBuffer.

Matrix layout is as follows:

- Element \[0], \[1] and \[2] should represent the right vector.
- Element \[4], \[5] and \[6] should represent the forward vector.
- Element \[8], \[9] and \[10] should represent the up vector.
- Element \[12], \[13] and \[14] should represent X, Y and Z translation coordinates.
- All other elements should be [0, 0, 0, 1].

## Parameters
* **matrixPtr**: A mutable pointer to a 64-byte buffer of floating-point values, representing an XMFLOAT4X4 in layout.
* **id**: A unique identifier of what the gizmo is affecting.

## Return value
Whether or not the matrix was modified.