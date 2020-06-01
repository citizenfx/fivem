# `event-doc-gen`

Generates event documentation using TypeDoc from embedded TypeScript definitions in project source code.

Still a work in progress.

## Documentation format

### Example
```cpp
								/*NETEV playerEnteredScope SERVER
								/#*
								 * A server-side event that is triggered when a player enters another player's scope.
								 *
								 * @param data - Data containing the players entering each other's scope.
								 #/
								declare function playerEnteredScope(data: {
									/#*
									 * The player that is entering the scope.
									 #/
									player: string,

									/#*
									 * The player for which the scope is being entered.
									 #/
									for: string
								}): void;
								*/
```

### Explanation

* **Start marker**: `/* NETEV <eventname> [subset]`
* Followed by a TypeScript specification with TSDoc annotations.
* `/*` and `*/` in TypeScript are replaced with `/#` and `#/` to not conflict with the C++ comment marker.

## Building
```
node index.js ../../code/
```

This'll generate an `out/` directory containing client and server event documentation.