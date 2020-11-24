---
ns: CFX
apiset: shared
---
## GET_RESOURCE_METADATA

```c
char* GET_RESOURCE_METADATA(char* resourceName, char* metadataKey, int index);
```

Gets the metadata value at a specified key/index from a resource's manifest.
See also: [Resource manifest](https://docs.fivem.net/docs/scripting-reference/resource-manifest/resource-manifest/)

## Parameters
* **resourceName**: The resource name.
* **metadataKey**: The key in the resource manifest.
* **index**: The value index, in a range from [0..GET_NUM_RESOURCE_METDATA-1].

## Return value
