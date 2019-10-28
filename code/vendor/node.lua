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

		links { 'v8', 'v8_libplatform', 'v8_libbase' }
		includedirs { '../vendor/node/src/' }

		includedirs { '../vendor/node/deps/cares/include/' }
		includedirs { '../vendor/node/deps/histogram/src/' }
		includedirs { '../vendor/node/deps/llhttp/include/' }
		includedirs { '../vendor/node/deps/brotli/c/include/' }
		includedirs { '../vendor/node/deps/http_parser/' }

		add_dependencies 'vendor:nghttp2'
		add_dependencies 'vendor:libuv'
		add_dependencies 'vendor:zlib'
		add_dependencies 'vendor:openssl_crypto'
		add_dependencies 'vendor:openssl_ssl'

		defines { 'NODE_WANT_INTERNALS=1', 'NODE_ARCH="x64"', 'NODE_NO_BROWSER_GLOBALS', 'NODE_USE_V8_PLATFORM', 'HAVE_OPENSSL', 'OPENSSL_NO_ENGINE', 'NODE_OPENSSL_SYSTEM_CERT_PATH=""' }
		
		if os.istarget('windows') then
			defines { 'NODE_PLATFORM="win32"', 'NOMINMAX' }
			
			includedirs { 'deplibs/include/include/' }
			
			links { 'ws2_32', 'dbghelp', 'psapi' }
			
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
				'src/ares_strsplit.c',
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
		
		files_project '../vendor/node/deps/llhttp/' {
			'src/llhttp.c', 'src/api.c', 'src/http.c'
		}

		files_project '../vendor/node/deps/http_parser/' {
			'http_parser.c'
		}
		
		files_project '../vendor/node/deps/histogram/' {
			'src/hdr_histogram.c'
		}
		
		files_project '../vendor/node/deps/brotli/' {
			-- Common
			'c/common/dictionary.c',
			'c/common/transform.c',

			-- Decoder
			'c/dec/bit_reader.c',
			'c/dec/decode.c',
			'c/dec/huffman.c',
			'c/dec/state.c',

			-- Encoder
			'c/enc/backward_references.c',
			'c/enc/backward_references_hq.c',
			'c/enc/bit_cost.c',
			'c/enc/block_splitter.c',
			'c/enc/brotli_bit_stream.c',
			'c/enc/cluster.c',
			'c/enc/compress_fragment.c',
			'c/enc/compress_fragment_two_pass.c',
			'c/enc/dictionary_hash.c',
			'c/enc/encode.c',
			'c/enc/encoder_dict.c',
			'c/enc/entropy_encode.c',
			'c/enc/histogram.c',
			'c/enc/literal_cost.c',
			'c/enc/memory.c',
			'c/enc/metablock.c',
			'c/enc/static_dict.c',
			'c/enc/utf8_util.c'
		}

		files_project 'vendor/' {
			'node_js2c.py'
		}

		filter 'files:vendor/node_js2c.py'
			buildcommands {
				('python %s %s'):format(
					path.getabsolute('vendor/node_js2c.py'),
					path.getabsolute('../vendor/node/')
				)
			}
			
			local nodeInputs = table.join(
				 os.matchfiles('../vendor/node/lib/**.js'),
				 os.matchfiles('../vendor/node/deps/v8/tools/*.js'),
				 os.matchfiles('../vendor/node/src/*_macros.py')
			)
			
			buildinputs(nodeInputs)
			buildoutputs { '../vendor/node/src/node_javascript.cc' }

		filter {}

		if os.istarget('windows') then
			files_project '../vendor/node/' {
				'src/node_javascript.cc', -- with msc, commands that output C/C++ source files are not fed into the build process yet
			}
		else
			files_project '../vendor/node/' {
				--'src/backtrace_posix.cc'
			}
		end

		files_project '../vendor/node/' {
			'src/api/async_resource.cc',
			'src/api/callback.cc',
			'src/api/encoding.cc',
			'src/api/environment.cc',
			'src/api/exceptions.cc',
			'src/api/hooks.cc',
			'src/api/utils.cc',

			'src/async_wrap.cc',
			'src/cares_wrap.cc',
			'src/connect_wrap.cc',
			'src/connection_wrap.cc',
			'src/debug_utils.cc',
			'src/env.cc',
			'src/fs_event_wrap.cc',
			'src/handle_wrap.cc',
			'src/heap_utils.cc',
			'src/js_native_api.h',
			'src/js_native_api_types.h',
			'src/js_native_api_v8.cc',
			'src/js_native_api_v8.h',
			'src/js_native_api_v8_internals.h',
			'src/js_stream.cc',
			'src/module_wrap.cc',
			'src/node.cc',
			'src/node_api.cc',
			'src/node_binding.cc',
			'src/node_buffer.cc',
			'src/node_config.cc',
			'src/node_constants.cc',
			'src/node_contextify.cc',
			'src/node_credentials.cc',
			'src/node_dir.cc',
			'src/node_domain.cc',
			'src/node_env_var.cc',
			'src/node_errors.cc',
			'src/node_file.cc',
			'src/node_http_parser_llhttp.cc',
			'src/node_http_parser_traditional.cc',
			'src/node_http2.cc',
			'src/node_i18n.cc',
			'src/node_main_instance.cc',
			'src/node_messaging.cc',
			'src/node_metadata.cc',
			'src/node_native_module.cc',
			'src/node_native_module_env.cc',
			'src/node_options.cc',
			'src/node_os.cc',
			'src/node_perf.cc',
			'src/node_platform.cc',
			'src/node_postmortem_metadata.cc',
			'src/node_process_events.cc',
			'src/node_process_methods.cc',
			'src/node_process_object.cc',
			'src/node_serdes.cc',
			'src/node_stat_watcher.cc',
			'src/node_symbols.cc',
			'src/node_task_queue.cc',
			'src/node_trace_events.cc',
			'src/node_types.cc',
			'src/node_url.cc',
			'src/node_util.cc',
			'src/node_v8.cc',
			'src/node_watchdog.cc',
			'src/node_worker.cc',
			'src/node_zlib.cc',
			'src/pipe_wrap.cc',
			'src/process_wrap.cc',
			'src/sharedarraybuffer_metadata.cc',
			'src/signal_wrap.cc',
			'src/spawn_sync.cc',
			'src/stream_base.cc',
			'src/stream_pipe.cc',
			'src/stream_wrap.cc',
			'src/string_bytes.cc',
			'src/string_decoder.cc',
			'src/tcp_wrap.cc',
			'src/timers.cc',
			'src/tracing/agent.cc',
			'src/tracing/node_trace_buffer.cc',
			'src/tracing/node_trace_writer.cc',
			'src/tracing/trace_event.cc',
			'src/tracing/traced_value.cc',
			'src/tty_wrap.cc',
			'src/udp_wrap.cc',
			'src/util.cc',
			'src/uv.cc',
			-- headers to make for a more pleasant IDE experience
			'src/aliased_buffer.h',
			'src/async_wrap.h',
			'src/async_wrap-inl.h',
			'src/base_object.h',
			'src/base_object-inl.h',
			'src/base64.h',
			'src/connect_wrap.h',
			'src/connection_wrap.h',
			'src/debug_utils.h',
			'src/env.h',
			'src/env-inl.h',
			'src/handle_wrap.h',
			'src/histogram.h',
			'src/histogram-inl.h',
			'src/http_parser_adaptor.h',
			'src/js_stream.h',
			'src/memory_tracker.h',
			'src/memory_tracker-inl.h',
			'src/module_wrap.h',
			'src/node.h',
			'src/node_api.h',
			'src/node_api_types.h',
			'src/node_binding.h',
			'src/node_buffer.h',
			'src/node_constants.h',
			'src/node_context_data.h',
			'src/node_contextify.h',
			'src/node_dir.h',
			'src/node_errors.h',
			'src/node_file.h',
			'src/node_http_parser_impl.h',
			'src/node_http2.h',
			'src/node_http2_state.h',
			'src/node_i18n.h',
			'src/node_internals.h',
			'src/node_main_instance.h',
			'src/node_messaging.h',
			'src/node_metadata.h',
			'src/node_mutex.h',
			'src/node_native_module.h',
			'src/node_native_module_env.h',
			'src/node_object_wrap.h',
			'src/node_options.h',
			'src/node_options-inl.h',
			'src/node_perf.h',
			'src/node_perf_common.h',
			'src/node_platform.h',
			'src/node_process.h',
			'src/node_revert.h',
			'src/node_root_certs.h',
			'src/node_stat_watcher.h',
			'src/node_union_bytes.h',
			'src/node_url.h',
			'src/node_version.h',
			'src/node_v8_platform-inl.h',
			'src/node_watchdog.h',
			'src/node_worker.h',
			'src/pipe_wrap.h',
			'src/req_wrap.h',
			'src/req_wrap-inl.h',
			'src/sharedarraybuffer_metadata.h',
			'src/spawn_sync.h',
			'src/stream_base.h',
			'src/stream_base-inl.h',
			'src/stream_pipe.h',
			'src/stream_wrap.h',
			'src/string_bytes.h',
			'src/string_decoder.h',
			'src/string_decoder-inl.h',
			'src/string_search.h',
			'src/tcp_wrap.h',
			'src/tracing/agent.h',
			'src/tracing/node_trace_buffer.h',
			'src/tracing/node_trace_writer.h',
			'src/tracing/trace_event.h',
			'src/tracing/trace_event_common.h',
			'src/tracing/traced_value.h',
			'src/tty_wrap.h',
			'src/udp_wrap.h',
			'src/util.h',
			'src/util-inl.h',
			-- Dependency headers
			'deps/http_parser/http_parser.h',
			'deps/v8/include/v8.h',

			'src/node_snapshot_stub.cc',
			'src/node_code_cache_stub.cc',
			
			-- use_openssl
			'src/node_crypto.cc',
			'src/node_crypto_bio.cc',
			'src/node_crypto_clienthello.cc',
			'src/node_crypto.h',
			'src/node_crypto_bio.h',
			'src/node_crypto_clienthello.h',
			'src/node_crypto_clienthello-inl.h',
			'src/node_crypto_groups.h',
			'src/tls_wrap.cc',
			'src/tls_wrap.h',
		}
	end
}
