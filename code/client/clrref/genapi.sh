#!/bin/sh
set -e
root=$1
out=$2
lib=$(realpath $3)

mkdir -p $out

# set up working directory
rm -rf /tmp/dotnet || true
mkdir -p /tmp/dotnet
cd /tmp/dotnet

dotnet new console
dotnet nuget add source 'https://pkgs.dev.azure.com/dnceng/public/_packaging/dotnet-eng/nuget/v3/index.json' || true
dotnet add package Microsoft.DotNet.GenAPI -v 6.0.0-beta.21063.5

dotnet ~/.nuget/packages/microsoft.dotnet.genapi/6.0.0-beta.21063.5/tools/netcoreapp2.1/Microsoft.DotNet.GenAPI.dll \
	"$lib" --lib-path "/usr/lib/mono/4.7.1-api/" --out "$out/CitizenFX.Core.cs" \
	--exclude-api-list "$root/client/clrref/exclude_list.txt" \
	--exclude-attributes-list "$root/client/clrref/exclude_attributes_list.txt"

# clean up
cd ~
rm -rf /tmp/dotnet
rm -rf ~/.dotnet/

exit 0
