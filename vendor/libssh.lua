return {
	include = function()
		includedirs { '../vendor/libssh/include/', '../vendor/libssh/include/cfx/' }
	end,

	run = function()
		language "C++"
		kind "SharedLib"

		add_dependencies 'vendor:zlib'
		add_dependencies 'vendor:botan'

		files_project '../vendor/libssh/src/'
		{
			'../include/**.h',
			'agent.cpp',
			'auth.cpp',
			'base64.cpp',
			'bignum.cpp',
			'buffer.cpp',
			'callbacks.cpp',
			'channels.cpp',
			'client.cpp',
			'config.cpp',
			'connect.cpp',
			'curve25519.cpp',
			'dh.cpp',
			'ecdh.cpp',
			'error.cpp',
			'getpass.cpp',
			'init.cpp',
			'kex.cpp',
			'known_hosts.cpp',
			'legacy.cpp',
			'log.cpp',
			'match.cpp',
			'messages.cpp',
			'misc.cpp',
			'options.cpp',
			'packet.cpp',
			'packet_cb.cpp',
			'packet_crypt.cpp',
			'pcap.cpp',
			'pki.cpp',
			'pki_container_openssh.cpp',
			'pki_ed25519.cpp',
			'poll.cpp',
			'session.cpp',
			'scp.cpp',
			'socket.cpp',
			'string.cpp',
			'threads.cpp',
			'wrapper.cpp',
			'external/bcrypt_pbkdf.cpp',
			'external/blowfish.cpp',
			'external/ed25519.cpp',
			'external/fe25519.cpp',
			'external/ge25519.cpp',
			'external/sc25519.cpp',
			'external/curve25519_ref.cpp',
			'libbotan.cpp',
			'pki_libbotan.cpp',
			'server.cpp',
			'bind.cpp',
			'gzip.cpp',
		}
	end
}