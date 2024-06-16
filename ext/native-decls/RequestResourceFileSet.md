---
ns: CFX
apiset: client
---
## REQUEST_RESOURCE_FILE_SET

```c
BOOL REQUEST_RESOURCE_FILE_SET(char* setName);
```

Requests a resource file set with the specified name to be downloaded and mounted on top of the current resource.

Resource file sets are specified in `fxmanifest.lua` with the following syntax:

```lua
file_set 'addon_ui' {
    'ui/addon/index.html',
    'ui/addon/**.js',
}
```

This command will trigger a script error if the request failed.

## Parameters
* **setName**: The name of the file set as specified in `fxmanifest.lua`.

## Return value
`TRUE` if the set is mounted, `FALSE` if the request is still pending.

## Examples
```lua
-- fxmanifest.lua
file_set 'dummies' {
    'dummy/**.txt',
    'potato.txt',
}

-- main script
local function PrintTest()
    local tests = { 'potato.txt', 'dummy/1.txt', 'dummy/b/2.txt' }

    for _, v in ipairs(tests) do
        local data = LoadResourceFile(GetCurrentResourceName(), v)
        print(v, data)
    end
end

RegisterCommand('fileset', function()
    PrintTest()

    while not RequestResourceFileSet('dummies') do
        Wait(100)
    end

    PrintTest()
end)
```