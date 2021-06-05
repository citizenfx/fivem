files_project(_ROOTPATH .. '/components/citizen-server-impl/') {
	'include/state/**.h',
	'src/state/**.h',
	'src/state/**.cpp',
}

vpaths {
	["src/state/*"] = _ROOTPATH .. '/components/citizen-server-impl/src/state/**',
	["include/state/*"] = _ROOTPATH .. '/components/citizen-server-impl/include/state/**',
}

defines { 'COMPILING_STATE' }