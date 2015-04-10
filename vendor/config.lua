vendor_component 'tinyxml2'
vendor_component 'breakpad'
vendor_component 'yaml-cpp'
vendor_component 'msgpack-c'
vendor_component 'protobuf_lite'
vendor_component 'zlib'
vendor_component 'opus'
vendor_component 'libuv'
vendor_component 'libssh'
vendor_component 'picohttpparser'
vendor_component 'udis86'
vendor_component 'cpp-uri'

if os.get() == 'windows' then
	vendor_component 'boost_program_options'
	vendor_component 'boost_filesystem'
end