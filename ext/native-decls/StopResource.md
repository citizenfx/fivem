---
ns: CFX
apiset: server
---

# STOP_RESOURCE

```c
BOOL STOP_RESOURCE(char* resourceName);
```

Stops the specified resource.

## Parameters

- **resourceName**: The name of the resource to stop.

## Return Value

A boolean indicating whether the resource was successfully stopped (`true`) or if it failed to stop (`false`, such as when the resource is already stopped or the name is invalid).

## Usage Limitations and Best Practices

When architecting complex server-side systems or modular scripts, it is a common anti-pattern to attempt to invoke `STOP_RESOURCE` on the resource currently executing the native (e.g., calling `StopResource(GetCurrentResourceName())`).

Because this native executes synchronously, the script attempts to destroy its own execution context while still processing the instruction stack. This typically results in:

- **Deadlocks / Freezing:** The server thread can lock up because the script halts abruptly without releasing execution control back to the resource event manager.
- **Unresolved Promises:** Any asynchronous cleanup tasks (like saving data before the resource unloads using the `onResourceStop` handler) will never resolve because the thread terminates mid-execution.

## Workarounds

If a script needs to be stopped programmatically from within (for instance, due to a failed initialization check or a missing dependency), do not call `STOP_RESOURCE` directly. Instead:

### 1. External Watchdog (Recommended)

Emit a server event (e.g., `TriggerEvent('core:requestStop', GetCurrentResourceName())`) and have a separate core manager resource handle the `StopResource()` invocation safely from outside the script's execution thread.

### 2. Event Loop Deferral (TS/JS)

In TypeScript/JavaScript environments, you can defer the termination to the next tick:

```ts
setImmediate(() => {
  StopResource(GetCurrentResourceName());
});
```

This allows the current synchronous stack to finish before the resource is stopped. However, the external watchdog approach remains the safest and most robust practice.

---

## Sources

- GitHub CitizenFX Issues — #1339, #1762
