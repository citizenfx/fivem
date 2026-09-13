---
ns: CFX
apiset: client
game: rdr3
---
## REQUEST_SCREENSHOT_UPLOAD

```c
void REQUEST_SCREENSHOT_UPLOAD(char* url, char* field, char* fileName, int quality, int maxDimension, char* extraFieldsJson, func callback);
```

Captures the game window and uploads the resulting JPEG to an HTTP endpoint as `multipart/form-data`, then hands the outcome to `callback`.

Capture, encode and upload all run on a background thread; the callback is invoked on the script thread once the request completes. Several uploads may be in flight at once, and the callback is dropped if the requesting resource stops before it completes.

Only `http` and `https` URLs are accepted. `field` and `fileName` are sanitized before being written into the multipart headers, and the response body is truncated at 64 KiB. Capture is only performed while the game window owns the foreground - see [`IS_SCREENSHOT_AVAILABLE`](#_0xCC71C452); when it is not, the callback still runs, with a status of 0.

## Parameters
* **url**: Endpoint to POST the image to.
* **field**: Form field name for the file part. Pass an empty string for the default of `file`. Discord webhooks expect `files[0]`.
* **fileName**: File name reported in the `Content-Disposition` header. Pass an empty string for the default of `screenshot.jpg`.
* **quality**: JPEG quality between 1 and 100. Pass 0 to use the default of 90.
* **maxDimension**: Longest edge in pixels the uploaded image may have. Pass 0 to keep the native resolution. Values are clamped to 16..16384.
* **extraFieldsJson**: Optional JSON object whose keys are emitted as additional non-file form fields, for example Discord's `payload_json`. Pass an empty string or `null` for none. Malformed JSON makes the call do nothing at all, callback included.
* **callback**: Invoked with the HTTP status code and the response body. The status is 0 when the request never reached the server, in which case the body describes the failure. It is not invoked at all when the native is called without a callback or from outside a resource.

## Examples
```lua
RegisterCommand('report', function()
    local payload = json.encode({ content = 'screenshot from ' .. GetPlayerName(PlayerId()) })

    RequestScreenshotUpload('https://discord.com/api/webhooks/id/token', 'files[0]', 'report.jpg', 90, 1920, json.encode({ payload_json = payload }), function(status, body)
        if status >= 200 and status < 300 then
            print('uploaded')
        else
            print(('upload failed (%d): %s'):format(status, body))
        end
    end)
end)
```
