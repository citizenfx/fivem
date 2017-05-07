mkdir -Force natives_stash

pushd nativemoon
make -j4
popd

$hash = (Get-FileHash -Algorithm SHA256 nativemoon\gta.lua).Hash.Substring(0, 8)

copy -force nativemoon\gta.lua "natives_stash\gta_$hash.lua"