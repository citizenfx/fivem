---
ns: CFX
apiset: shared
---
## GET_RESOURCE_STATE

```c
char* GET_RESOURCE_STATE(char* resourceName);
```

**Valid resource states**:
* *missing*: whenever the resource doesn't exist
* *uninitalized*: whenever the resource hasn't been initialized (like at server start)
* *starting*: whenever the resource is starting and loading dependencies
* *started*: whenever the resource is started
* *stopped*: whenever the resource was stopped, or hasn't been started
* *unknown*: whenever the resource state isn't known

## Parameters
* **resourceName**: The name of the resource.

## Return value
Returns the current state of the specified resource.
