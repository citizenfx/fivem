---
ns: CFX
apiset: server
---
## REGISTER_RESOURCE_BUILD_TASK_FACTORY

```c
void REGISTER_RESOURCE_BUILD_TASK_FACTORY(char* factoryId, func factoryFn);
```

Registers a build task factory for resources.
The function should return an object (msgpack map) with the following fields:
```
{
// returns whether the specific resource should be built
shouldBuild = func(resourceName: string): bool,

// asynchronously start building the specific resource.
// call cb when completed
build = func(resourceName: string, cb: func(success: bool, status: string): void): void
}
```

## Parameters
* **factoryId**: The identifier for the build task.
* **factoryFn**: The factory function.

