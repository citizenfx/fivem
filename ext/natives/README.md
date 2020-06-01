# Native codegen
A suite of Lua tools to transform native function declarations into actual code.

## Invoking
* `make` on Windows (use MSYS2 Make).
* Manual command invocation on Linux (see Linux build script)

## Manually invoking
```
lua53 codegen.lua <stashfile> <outtype> [subset]
```

* **stashfile**: An input file.
* **outtype**: See the section below.
* **subset**: Usually `server` or not specified. 

## Input format
The input format is defined most clearly in `native-doc-tooling`'s `views/lua.twig`, and should not be manually written
by users.

### Example
```lua
native "GET_PLAYER_PED"
	hash "0x43A66C31C68491C0"
	jhash (0x6E31E993)
	arguments {
		Player "playerId",
	}
	ns "PLAYER"
	returns	"Ped"
	doc [[!
<summary>
		returns the players ped used in many functions
</summary>
	]]
```

## Output types
* `cs`: C# `API` class for Natives*.cs.
* `dts`: .d.ts file.
* `enum`: C# `Hash` enum content.
* `js`: .js file.
* `json`: NativeDB `natives.json`-style file
* `lua`: Singular monolithic .lua wrappers.
* `markdown`: `native-doc-tooling`-style Markdown files.
* `native_lua`: C++ Lua wrappers. Currently unused.
* `rpc`: Runtime file for RPC natives.
* `slua`: Split lazy-loaded .lua files to save Lua state memory. Usually packed as .zip.

## RPC natives
`rpc_spec_natives.lua` contains a number of natives to expose to server builds as well as client RPC definitions for use
with OneSync.

## Types
See `codegen_types.lua`.