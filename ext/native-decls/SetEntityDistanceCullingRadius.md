---
ns: CFX
apiset: server
---
## SET_ENTITY_DISTANCE_CULLING_RADIUS

```c
void SET_ENTITY_DISTANCE_CULLING_RADIUS(Entity entity, float radius);
```

It overrides the default distance culling radius of an entity. Set to `0.0` to reset.
If you want to interact with an entity outside of your players' scopes set the radius to a huge number.

**WARNING**: Culling natives are deprecated and have known, [unfixable issues](https://forum.cfx.re/t/issue-with-culling-radius-and-server-side-entities/4900677/4)

## Parameters
* **entity**: The entity handle to override the distance culling radius.
* **radius**: The new distance culling radius.
