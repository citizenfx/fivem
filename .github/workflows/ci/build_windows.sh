pwsh ./fxd.ps1 get-chrome
./prebuild.cmd

pwsh ./fxd.ps1 gen -game $PROGRAM

MSBUILD=$(./code/tools/ci/vswhere.exe -latest -prerelease -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | head -n 1 | tr -d '\r')

if [[ -z "$MSBUILD" ]]; then
	echo "::error::Could not find MSBuild"
	exit 1
fi

cd code/build/$PROGRAM/$([[ $PROGRAM = server ]] && echo windows || echo '')

"$MSBUILD" CitizenMP.sln -t:build -restore -p:RestorePackagesConfig=true -p:preferredtoolarchitecture=x64 -p:configuration=release -maxcpucount:4 -v:q -fl1 "-flp1:logfile=errors.log;errorsonly"
MSBUILD_ERROR=$?

if [[ $MSBUILD_ERROR -eq 0 ]]; then
	echo Successfully build $PROGRAM
else
	RED=$'\e[31m'
	
	echo "::error::Failed to build $PROGRAM, MSBuild returned with error code: $MSBUILD_ERROR"
	while IFS= read -r LINE; do echo "$RED$LINE"; done < errors.log
	
	exit $MSBUILD_ERROR
fi