---
ns: CFX
apiset: client
game: rdr3
---
## SAVE_SCREENSHOT_TO_FILE

```c
BOOL SAVE_SCREENSHOT_TO_FILE(char* fileName, int quality);
```

Captures the game window and writes it to a JPEG file in the client's screenshot directory (`data/screenshots/` inside the RedM install).

`fileName` is always resolved relative to that directory. Absolute paths, drive letters, UNC prefixes and `..` segments are rejected, so a resource can not write to arbitrary locations on the client's disk. The name must end in `.jpg` or `.jpeg`; subdirectories are created as needed.

This call is synchronous and blocks the script thread for the whole capture and encode.

## Parameters
* **fileName**: Path relative to the screenshot directory, for example `report.jpg` or `reports/2024-01-01.jpg`.
* **quality**: JPEG quality between 1 and 100. Pass 0 to use the default of 90.

## Return value
True when the file was written, false when the path was rejected, capture was unavailable, or writing failed.

## Examples
```lua
if SaveScreenshotToFile('reports/latest.jpg', 90) then
    print('saved to data/screenshots/reports/latest.jpg')
end
```
