@echo off
set ROOT=%CD%
cd %~dp0

copy /y %ROOT%\caches\fivereborn\citizen\scripting\v8\natives_universal.d.ts natives_universal.d.ts
copy /y %ROOT%\caches\fivereborn\citizen\scripting\v8\index.d.ts index.d.ts

echo //registry.npmjs.org/:_authToken=%NPM_TOKEN% > .npmrc

call npm config set git-tag-version false

call npm version "2.0.%CI_PIPELINE_ID%-1"
call npm publish