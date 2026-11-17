---
ns: CFX
apiset: server
---
## VERIFY_PASSWORD_HASH_ASYNC

```c
void VERIFY_PASSWORD_HASH_ASYNC(char* password, char* hash, func callback);
```

Asynchronous version of [VERIFY_PASSWORD_HASH](#_0x2E310ACD): the password is checked on a worker thread instead of blocking the server thread, and the callback is then called on the server thread.

## Parameters
* **password**: The password to check.
* **hash**: The hash to check the password against, as returned by [GET_PASSWORD_HASH_ASYNC](#_0xBFEE7075) or [GET_PASSWORD_HASH](#_0x23473EA4).
* **callback**: A function of `(valid: boolean) => void`. `valid` is also false if the hash is malformed.
