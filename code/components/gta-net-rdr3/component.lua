includedirs {
	'../gta-net-five/include/',
}

return function()
	add_dependencies { 'citizen:resources:client', 'vendor:concurrentqueue' }

	filter {}

	files {
		'components/gta-net-five/include/CloneManager.h',
		'components/gta-net-five/include/FramePacketHandler.h',
		'components/gta-net/five/include/ObjectIdPacketHandler.h',
		'components/gta-net-five/src/ClientRPC.cpp',
		'components/gta-net-five/src/CloneDebug.cpp',
		'components/gta-net-five/src/CloneExperiments.cpp',
		'components/gta-net-five/src/CloneExperiments_ArrayHandler.cpp',
		'components/gta-net-five/src/CloneManager.cpp',
		'components/gta-net-five/src/CloneObjectManager.cpp',
		'components/gta-net-five/src/ObjectIdManager.cpp',
		'components/gta-net-five/src/rlNetBuffer.cpp',
		'components/gta-net-five/src/netGameEvent.cpp',
		'components/gta-net-five/src/netPlayerManager.cpp',
		'components/gta-net-five/src/netTimeSync.cpp',
		'components/gta-net-five/src/MumbleVoice.cpp',
	}
end
