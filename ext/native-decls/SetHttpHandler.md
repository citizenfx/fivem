---
ns: CFX
apiset: server
---
## SET_HTTP_HANDLER

```c
void SET_HTTP_HANDLER(func handler);
```

Sets the handler for HTTP requests made to the executing resource.

Example request URL: `http://localhost:30120/http-test/ping` - this request will be sent to the `http-test` resource with the `/ping` path.

The handler function assumes the following signature:

```ts
function HttpHandler(
  request: {
    address: string;
    headers: Record<string, string>;
    method: string;
    path: string;
    setDataHandler(handler: (data: string) => void): void;
    setDataHandler(handler: (data: ArrayBuffer) => void, binary: 'binary'): void;
    setCancelHandler(handler: () => void): void;
  },
  response: {
    writeHead(code: number, headers?: Record<string, string | string[]>): void;
    write(data: string): void;
    send(data?: string): void;
  }
): void;
```

- **request**: The request object.
  - **address**: The IP address of the request sender.
  - **path**: The path to where the request was sent.
  - **headers**: The headers sent with the request.
  - **method**: The request method.
  - **setDataHandler**: Sets the handler for when a data body is passed with the request. Additionally you can pass the `'binary'` argument to receive a `BufferArray` in JavaScript or `System.Byte[]` in C# (has no effect in Lua).
  - **setCancelHandler**: Sets the handler for when the request is cancelled.
- **response**: An object to control the response.
  - **writeHead**: Sets the status code & headers of the response. Can be only called once and won't work if called after running other response functions.
  - **write**: Writes to the response body without sending it. Can be called multiple times.
  - **send**: Writes to the response body and then sends it along with the status code & headers, finishing the request.

## Examples
```lua
SetHttpHandler(function(request, response)
  if request.method == 'GET' and request.path == '/ping' then -- if a GET request was sent to the `/ping` path
      response.writeHead(200, { ['Content-Type'] = 'text/plain' }) -- set the response status code to `200 OK` and the body content type to plain text
      response.send('pong') -- respond to the request with `pong`
  else -- otherwise
      response.writeHead(404) -- set the response status code to `404 Not Found`
      response.send() -- respond to the request with no data
  end
end)
```

## Parameters
* **handler**: The handler function.

