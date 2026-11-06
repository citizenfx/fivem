---
ns: CFX
apiset: server
---
## GET_PASSWORD_HASH_ASYNC

```c
void GET_PASSWORD_HASH_ASYNC(char* password, func callback);
```

Asynchronous version of [GET_PASSWORD_HASH](#_0x23473EA4): the password is hashed on a worker thread instead of blocking the server thread, and the callback is then called on the server thread.

## Parameters
* **password**: The password to hash.
* **callback**: A function of `(hash: string) => void`. The hash is an empty string if hashing failed.
