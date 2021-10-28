@IF EXIST "%~dp0\node.exe" (
  "%~dp0\node.exe"  "%~dp0\build.js" %*
) ELSE (
  @SETLOCAL
  @SET PATHEXT=%PATHEXT:;.JS;=;%
  node  "%~dp0\build.js" %*
)