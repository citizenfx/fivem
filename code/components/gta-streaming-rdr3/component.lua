return function()
	configuration {}
	filter {}

	includedirs {
		'components/gta-streaming-five/include/',
	}

	files {
		'components/gta-streaming-five/include/Streaming.h',
		'components/gta-streaming-five/src/LoadStreamingFile.cpp',
		'components/gta-streaming-five/src/StreamingFreeTests.cpp',
	}
end