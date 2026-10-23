pwsh ./fxd.ps1 get-chrome
./prebuild.cmd

pwsh ./fxd.ps1 gen -game $PROGRAM

# Resolve MSBuild through the vendored vswhere, the same way code/tools/fxd/gen.ps1 resolves
# the Visual Studio version, so this no longer breaks when the runner image ships a new VS.
MSBUILD=$(./code/tools/ci/vswhere.exe -prerelease -latest -products '*' -requires Microsoft.Component.MSBuild Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'MSBuild\**\Bin\MSBuild.exe' | tr -d '\r' | head -n 1)

if [[ -z $MSBUILD ]]; then
	echo "::error::Failed to build $PROGRAM, could not locate MSBuild.exe through vswhere."
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