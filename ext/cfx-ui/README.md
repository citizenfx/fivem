# cfx-ui

`cfx-ui` is an Angular application containing the main menu for the Cfx.re game clients, as well as the web server lists
on `servers.fivem.net` and `servers.redm.gg`.

## Building
See [ui-build](../ui-build/) for building a full version of `cfx-ui`.

## Developing (game)
```
yarn ng serve -c game --host %COMPUTERNAME% --disable-host-check
```

Then run the game with `+set ui_url http://COMPUTERNAME:4200/`, where `COMPUTERNAME` is your local computer name.
Due to NUI policy, `localhost` is not supported.
