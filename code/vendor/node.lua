return {
	include = function()
		if _OPTIONS['game'] == 'server' then
			includedirs { '../vendor/node/src/' }
		end
	end,

	run = function()
		if _OPTIONS['game'] ~= 'server' then
			language 'C'
			kind 'StaticLib'
			
			files { 'vendor/node_dummy.c' }
			
			return
		end
	
		language "C++"
		kind "SharedLib"

		links { 'v8', 'v8_libplatform' }
		includedirs { '../vendor/node/src/' }

		includedirs { '../vendor/node/deps/cares/include/' }
		includedirs { '../vendor/node/deps/http_parser/' }

		add_dependencies 'vendor:nghttp2'
		add_dependencies 'vendor:libuv'
		add_dependencies 'vendor:zlib'
		add_dependencies 'vendor:openssl_crypto'
		add_dependencies 'vendor:openssl_ssl'

		defines { 'NODE_WANT_INTERNALS=1', 'NODE_ARCH="x64"', 'NODE_USE_V8_PLATFORM', 'HAVE_OPENSSL', 'OPENSSL_NO_ENGINE', 'NODE_OPENSSL_SYSTEM_CERT_PATH=""' }
		
		if os.istarget('windows') then
			defines { 'NODE_PLATFORM="win32"' }
			
			includedirs { 'deplibs/include/include/' }
			
			links { 'ws2_32', 'dbghelp' }
			
			buildoptions { '/MP' }
		elseif os.istarget('linux') then
			defines { 'NODE_PLATFORM="linux"', '__POSIX__' }
		end

		if os.istarget('windows') then
			defines { 'CARES_PULL_WS2TCPIP_H=1', 'WIN32' }
			
			links { 'iphlpapi' }

			files_project '../vendor/node/deps/cares/' {
				'include/ares.h',
				'include/ares_rules.h',
				'include/ares_version.h',
				'include/nameser.h',
				'src/ares_cancel.c',
				'src/ares__close_sockets.c',
				'src/ares_create_query.c',
				'src/ares_data.c',
				'src/ares_data.h',
				'src/ares_destroy.c',
				'src/ares_dns.h',
				'src/ares_expand_name.c',
				'src/ares_expand_string.c',
				'src/ares_fds.c',
				'src/ares_free_hostent.c',
				'src/ares_free_string.c',
				'src/ares_getenv.h',
				'src/ares_gethostbyaddr.c',
				'src/ares_gethostbyname.c',
				'src/ares__get_hostent.c',
				'src/ares_getnameinfo.c',
				'src/ares_getopt.c',
				'src/ares_getopt.h',
				'src/ares_getsock.c',
				'src/ares_init.c',
				'src/ares_ipv6.h',
				'src/ares_library_init.c',
				'src/ares_library_init.h',
				'src/ares_llist.c',
				'src/ares_llist.h',
				'src/ares_mkquery.c',
				'src/ares_nowarn.c',
				'src/ares_nowarn.h',
				'src/ares_options.c',
				'src/ares_parse_aaaa_reply.c',
				'src/ares_parse_a_reply.c',
				'src/ares_parse_mx_reply.c',
				'src/ares_parse_naptr_reply.c',
				'src/ares_parse_ns_reply.c',
				'src/ares_parse_ptr_reply.c',
				'src/ares_parse_soa_reply.c',
				'src/ares_parse_srv_reply.c',
				'src/ares_parse_txt_reply.c',
				'src/ares_platform.h',
				'src/ares_private.h',
				'src/ares_process.c',
				'src/ares_query.c',
				'src/ares__read_line.c',
				'src/ares_search.c',
				'src/ares_send.c',
				'src/ares_setup.h',
				'src/ares_strcasecmp.c',
				'src/ares_strcasecmp.h',
				'src/ares_strdup.c',
				'src/ares_strdup.h',
				'src/ares_strerror.c',
				'src/ares_timeout.c',
				'src/ares__timeval.c',
				'src/ares_version.c',
				'src/ares_writev.c',
				'src/ares_writev.h',
				'src/bitncmp.c',
				'src/bitncmp.h',
				'src/inet_net_pton.c',
				'src/inet_ntop.c',
				'src/ares_inet_net_pton.h',
				'src/setup_once.h',
			}

			includedirs { '../vendor/node/deps/cares/config/win32/', '../vendor/node/deps/cares/src/' }

			files_project '../vendor/node/deps/cares/' {
				'src/config-win32.h',
				'src/windows_port.c',
				'src/ares_getenv.c',
				'src/ares_iphlpapi.h',
				'src/ares_platform.c'
			}
		elseif os.istarget('linux') then
			links { 'cares' }
		end

		files_project '../vendor/node/deps/http_parser/' {
			'http_parser.c'
		}

		prebuildcommands {
			('python %s %s'):format(
				path.getabsolute('vendor/node_js2c.py'),
				path.getabsolute('../vendor/node/')
			)
		}

		if os.istarget('windows') then
			files_project '../vendor/node/' {
				'src/backtrace_win32.cc'
			}
		else
			files_project '../vendor/node/' {
				'src/backtrace_posix.cc'
			}
		end

		files_project '../vendor/node/' {
			'src/node_javascript.cc',
			'src/async_wrap.cc',
	        'src/cares_wrap.cc',
	        'src/connection_wrap.cc',
	        'src/connect_wrap.cc',
	        'src/env.cc',
	        'src/fs_event_wrap.cc',
	        'src/handle_wrap.cc',
	        'src/js_stream.cc',
	        'src/module_wrap.cc',
	        'src/node.cc',
	        'src/node_api.cc',
	        'src/node_api.h',
	        'src/node_api_types.h',
	        'src/node_buffer.cc',
	        'src/node_config.cc',
	        'src/node_constants.cc',
	        'src/node_contextify.cc',
	        'src/node_crypto.cc',
	        'src/node_crypto_bio.cc',
	        'src/node_crypto_clienthello.cc',
	        'src/node_debug_options.cc',
	        'src/node_file.cc',
	        'src/node_http2.cc',
	        'src/node_http_parser.cc',
	        'src/node_main.cc',
	        'src/node_os.cc',
	        'src/node_platform.cc',
	        'src/node_perf.cc',
	        'src/node_serdes.cc',
	        'src/node_trace_events.cc',
	        'src/node_url.cc',
	        'src/node_util.cc',
	        'src/node_v8.cc',
	        'src/node_stat_watcher.cc',
	        'src/node_watchdog.cc',
	        'src/node_zlib.cc',
	        'src/node_i18n.cc',
	        'src/pipe_wrap.cc',
	        'src/process_wrap.cc',
	        'src/signal_wrap.cc',
	        'src/spawn_sync.cc',
	        'src/string_bytes.cc',
	        'src/string_search.cc',
	        'src/stream_base.cc',
	        'src/stream_wrap.cc',
	        'src/tcp_wrap.cc',
	        'src/timer_wrap.cc',
	        'src/tls_wrap.cc',
	        'src/tracing/agent.cc',
	        'src/tracing/node_trace_buffer.cc',
	        'src/tracing/node_trace_writer.cc',
	        'src/tracing/trace_event.cc',
	        'src/tty_wrap.cc',
	        'src/udp_wrap.cc',
	        'src/util.cc',
	        'src/uv.cc',
	        -- headers to make for a more pleasant IDE experience
	        'src/aliased_buffer.h',
	        'src/async-wrap.h',
	        'src/async-wrap-inl.h',
	        'src/base-object.h',
	        'src/base-object-inl.h',
	        'src/connection_wrap.h',
	        'src/connect_wrap.h',
	        'src/env.h',
	        'src/env-inl.h',
	        'src/handle_wrap.h',
	        'src/js_stream.h',
	        'src/module_wrap.h',
	        'src/node.h',
	        'src/node_http2_core.h',
	        'src/node_http2_core-inl.h',
	        'src/node_buffer.h',
	        'src/node_constants.h',
	        'src/node_debug_options.h',
	        'src/node_http2.h',
	        'src/node_http2_state.h',
	        'src/node_internals.h',
	        'src/node_javascript.h',
	        'src/node_mutex.h',
	        'src/node_platform.h',
	        'src/node_perf.h',
	        'src/node_perf_common.h',
	        'src/node_root_certs.h',
	        'src/node_version.h',
	        'src/node_watchdog.h',
	        'src/node_wrap.h',
	        'src/node_revert.h',
	        'src/node_i18n.h',
	        'src/pipe_wrap.h',
	        'src/tty_wrap.h',
	        'src/tcp_wrap.h',
	        'src/udp_wrap.h',
	        'src/req-wrap.h',
	        'src/req-wrap-inl.h',
	        'src/string_bytes.h',
	        'src/stream_base.h',
	        'src/stream_base-inl.h',
	        'src/stream_wrap.h',
	        'src/tracing/agent.h',
	        'src/tracing/node_trace_buffer.h',
	        'src/tracing/node_trace_writer.h',
	        'src/tracing/trace_event.h',
	        'src/util.h',
	        'src/util-inl.h',
	        'deps/http_parser/http_parser.h',
	        'deps/v8/include/v8.h',
	        'deps/v8/include/v8-debug.h',
		}
	end
}
