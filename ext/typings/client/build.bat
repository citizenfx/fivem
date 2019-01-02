@echo off
set ROOT=%CD%
cd %~dp0

%ROOT%\code\tools\ci\xz.exe -cd %ROOT%/caches/diff/fivereborn/citizen/scripting/v8/natives_universal.d.ts.xz > natives_universal.d.ts
copy /y %ROOT%\caches\diff\fivereborn\citizen\scripting\v8\index.d.ts index.d.ts

echo //registry.npmjs.org/:_authToken=%NPM_TOKEN% > .npmrc

call npm config set git-tag-version false

call npm version "1.0.%CI_PIPELINE_ID%-1"
call npm publish