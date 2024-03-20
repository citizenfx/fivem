---
ns: CFX
apiset: server
---

## GET_PLAYER_PROOFS

```c
void GET_PLAYER_PROOFS(Player player, int* bulletProof, int* fireProof, int* explosionProof, int* collisionProof, int* meleeProof, int* drownProof, int* steamProof);
```

Gets the player proofs for certain types of damage.

## Return value

Returns 1 if true, otherwise 0 if false.

## Parameters

-   **entity**: The player entity to get the proofs of.
-   **bulletProof**:
-   **fireProof**:
-   **explosionProof**:
-   **collisionProof**:
-   **meleeProof**:
-   **drownProof**:
-   **steamProof**:
