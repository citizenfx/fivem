---
ns: CFX
apiset: client
---
## REGISTER_FONT_ID

```c
int REGISTER_FONT_ID(char* fontName);
```

Registers a specified font name for use with text draw commands.

## Parameters
* **fontName**: The name of the font in the GFx font library.

## Return value
An index to use with [SET\_TEXT\_FONT](#_0x66E0276CC5F6B9DA) and similar natives.
