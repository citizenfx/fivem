return {
	include = function()
		includedirs { '../vendor/node/src/' }
	end,

	run = function()
		language "C++"
		kind "SharedLib"

		includedirs { '../vendor/node/src/' }
		includedirs { '../vendor/node/gen/' }
		includedirs { '../vendor/node/gen/src/' }
		includedirs { '../vendor/node/gen/src/node/inspector/protocol/' }

		includedirs { '../vendor/node/deps/cares/include/' }
		includedirs { '../vendor/node/deps/histogram/src/' }
		includedirs { '../vendor/node/deps/llhttp/include/' }
		includedirs { '../vendor/node/deps/brotli/c/include/' }
		includedirs { '../vendor/node/deps/uvwasi/include/' }
		includedirs { '../vendor/node/deps/googletest/include/' }
		includedirs { '../vendor/node/deps/icu-small/source/common/' }

		add_dependencies 'vendor:nghttp2'
		add_dependencies 'vendor:libuv'
		add_dependencies 'vendor:zlib'
		add_dependencies 'vendor:openssl_crypto'
		add_dependencies 'vendor:openssl_ssl'
		add_dependencies 'vendor:v8-93'

		defines { 'NODE_WANT_INTERNALS=1', 'NODE_ARCH="x64"', 'NODE_NO_BROWSER_GLOBALS', 'NODE_USE_V8_PLATFORM', 'HAVE_OPENSSL', 'OPENSSL_NO_ENGINE', 'NODE_OPENSSL_SYSTEM_CERT_PATH=""', 'HAVE_INSPECTOR' }
		
		if os.istarget('windows') then
			defines { 'NODE_PLATFORM="win32"', 'NOMINMAX', '_UNICODE=1' }
			
			links { 'ws2_32', 'dbghelp', 'psapi' }
			
			buildoptions { '/wd4251', '/wd4275', '/wd4067', '/wd4267', '/wd4996', '/MP', '/utf-8' }
		elseif os.istarget('linux') then
			defines { 'NODE_PLATFORM="linux"', '__POSIX__' }
		end
		
		defines { 'U_STATIC_IMPLEMENTATION', 'U_COMMON_IMPLEMENTATION' }
		
		files_project '../vendor/node/deps/icu-small/' {
			'source/common/*.cpp',
			'source/stubdata/*.cpp',
		}

		if os.istarget('windows') then
			defines { 'CARES_PULL_WS2TCPIP_H=1', '_WINSOCK_DEPRECATED_NO_WARNINGS', 'WIN32' }
			
			includedirs { '../vendor/node/deps/cares/config/win32/' }

			links { 'iphlpapi' }

			files_project '../vendor/node/deps/cares/' {
						'include/ares.h',
						'include/ares_dns.h',
						'include/ares_rules.h',
						'include/ares_version.h',
						'src/lib/ares_android.c',
						'src/lib/ares_cancel.c',
						'src/lib/ares__close_sockets.c',
						'src/lib/ares_create_query.c',
						'src/lib/ares_data.c',
						'src/lib/ares_data.h',
						'src/lib/ares_destroy.c',
						'src/lib/ares_expand_name.c',
						'src/lib/ares_expand_string.c',
						'src/lib/ares_fds.c',
						'src/lib/ares_free_hostent.c',
						'src/lib/ares_free_string.c',
						'src/lib/ares_freeaddrinfo.c',
						'src/lib/ares_getenv.h',
						'src/lib/ares_getaddrinfo.c',
						'src/lib/ares_gethostbyaddr.c',
						'src/lib/ares_gethostbyname.c',
						'src/lib/ares__get_hostent.c',
						'src/lib/ares_getnameinfo.c',
						'src/lib/ares_getsock.c',
						'src/lib/ares_init.c',
						'src/lib/ares_ipv6.h',
						'src/lib/ares_library_init.c',
						'src/lib/ares_library_init.h',
						'src/lib/ares_llist.c',
						'src/lib/ares_llist.h',
						'src/lib/ares_mkquery.c',
						'src/lib/ares_nameser.h',
						'src/lib/ares_nowarn.c',
						'src/lib/ares_nowarn.h',
						'src/lib/ares_options.c',
						'src/lib/ares__parse_into_addrinfo.c',
						'src/lib/ares_parse_aaaa_reply.c',
						'src/lib/ares_parse_a_reply.c',
						'src/lib/ares_parse_caa_reply.c',
						'src/lib/ares_parse_mx_reply.c',
						'src/lib/ares_parse_naptr_reply.c',
						'src/lib/ares_parse_ns_reply.c',
						'src/lib/ares_parse_ptr_reply.c',
						'src/lib/ares_parse_soa_reply.c',
						'src/lib/ares_parse_srv_reply.c',
						'src/lib/ares_parse_txt_reply.c',
						'src/lib/ares_platform.h',
						'src/lib/ares_private.h',
						'src/lib/ares_process.c',
						'src/lib/ares_query.c',
						'src/lib/ares__read_line.c',
						'src/lib/ares__readaddrinfo.c',
						'src/lib/ares_search.c',
						'src/lib/ares_send.c',
						'src/lib/ares_setup.h',
						'src/lib/ares__sortaddrinfo.c',
						'src/lib/ares_strcasecmp.c',
						'src/lib/ares_strcasecmp.h',
						'src/lib/ares_strdup.c',
						'src/lib/ares_strdup.h',
						'src/lib/ares_strerror.c',
						'src/lib/ares_strsplit.c',
						'src/lib/ares_timeout.c',
						'src/lib/ares__timeval.c',
						'src/lib/ares_version.c',
						'src/lib/ares_writev.c',
						'src/lib/ares_writev.h',
						'src/lib/bitncmp.c',
						'src/lib/bitncmp.h',
						'src/lib/inet_net_pton.c',
						'src/lib/inet_ntop.c',
						'src/lib/ares_inet_net_pton.h',
						'src/lib/setup_once.h',
						'src/tools/ares_getopt.c',
						'src/tools/ares_getopt.h',
						'src/lib/config-win32.h',
						'src/lib/windows_port.c',
						'src/lib/ares_getenv.c',
						'src/lib/ares_iphlpapi.h',
						'src/lib/ares_platform.c',
			}

			includedirs { '../vendor/node/deps/cares/config/win32/', '../vendor/node/deps/cares/src/lib/' }

			files_project '../vendor/node/deps/cares/' {
				'src/lib/config-win32.h',
				'src/lib/windows_port.c',
				'src/lib/ares_getenv.c',
				'src/lib/ares_iphlpapi.h',
				'src/lib/ares_platform.c',
			}
		elseif os.istarget('linux') then
			links { 'cares', 'atomic' }
		end
		
		files_project '../vendor/node/deps/uvwasi/' {
			'src/clocks.c',
			'src/fd_table.c',
			'src/path_resolver.c',
			'src/poll_oneoff.c',
			'src/uv_mapping.c',
			'src/uvwasi.c',
			'src/wasi_rights.c',
			'src/wasi_serdes.c',
		}

		files_project '../vendor/node/deps/llhttp/' {
			'src/llhttp.c', 'src/api.c', 'src/http.c'
		}
		
		files_project '../vendor/node/deps/histogram/' {
			'src/hdr_histogram.c'
		}
		
		files_project '../vendor/node/deps/brotli/' {
			--Common
			'c/common/constants.c',
			'c/common/context.c',
			'c/common/dictionary.c',
			'c/common/platform.c',
			'c/common/transform.c',

			--Decoder
			'c/dec/bit_reader.c',
			'c/dec/decode.c',
			'c/dec/huffman.c',
			'c/dec/state.c',

			--Encoder
			'c/enc/backward_references.c',
			'c/enc/backward_references_hq.c',
			'c/enc/bit_cost.c',
			'c/enc/block_splitter.c',
			'c/enc/brotli_bit_stream.c',
			'c/enc/cluster.c',
			'c/enc/command.c',
			'c/enc/compress_fragment.c',
			'c/enc/compress_fragment_two_pass.c',
			'c/enc/dictionary_hash.c',
			'c/enc/encode.c',
			'c/enc/encoder_dict.c',
			'c/enc/entropy_encode.c',
			'c/enc/fast_log.c',
			'c/enc/histogram.c',
			'c/enc/literal_cost.c',
			'c/enc/memory.c',
			'c/enc/metablock.c',
			'c/enc/static_dict.c',
			'c/enc/utf8_util.c'
		}

		files_project 'vendor/' {
			'node_js2c.py',
			'node_inspector.py'
		}

		filter 'files:vendor/node_js2c.py'
			buildcommands {
				('%s %s %s'):format(
					pythonExecutable,
					path.getabsolute('vendor/node_js2c.py'),
					path.getabsolute('../vendor/node/')
				)
			}
			
			local nodeInputs = table.join(
				 os.matchfiles('../vendor/node/lib/**.js'),
				 os.matchfiles('../vendor/node/deps/v8/tools/*.js'),
				 os.matchfiles('../vendor/node/src/*_macros.py'),
				 os.matchfiles('../vendor/node/config.gypi')
			)
			
			buildinputs(nodeInputs)
			buildoutputs { '../vendor/node/src/node_javascript.cc' }

		filter {}
		
		filter 'files:vendor/node_inspector.py'
			buildcommands {
				('%s %s %s'):format(
					pythonExecutable,
					path.getabsolute('vendor/node_inspector.py'),
					path.getabsolute('../vendor/node/')
				)
			}
			
			local nodeInputs = {
				'../vendor/node/tools/inspector_protocol/lib/Allocator_h.template',
				'../vendor/node/tools/inspector_protocol/lib/Array_h.template',
				'../vendor/node/tools/inspector_protocol/lib/base_string_adapter_cc.template',
				'../vendor/node/tools/inspector_protocol/lib/base_string_adapter_h.template',
				'../vendor/node/tools/inspector_protocol/lib/DispatcherBase_cpp.template',
				'../vendor/node/tools/inspector_protocol/lib/DispatcherBase_h.template',
				'../vendor/node/tools/inspector_protocol/lib/encoding_cpp.template',
				'../vendor/node/tools/inspector_protocol/lib/encoding_h.template',
				'../vendor/node/tools/inspector_protocol/lib/ErrorSupport_cpp.template',
				'../vendor/node/tools/inspector_protocol/lib/ErrorSupport_h.template',
				'../vendor/node/tools/inspector_protocol/lib/Forward_h.template',
				'../vendor/node/tools/inspector_protocol/lib/FrontendChannel_h.template',
				'../vendor/node/tools/inspector_protocol/lib/Maybe_h.template',
				'../vendor/node/tools/inspector_protocol/lib/Object_cpp.template',
				'../vendor/node/tools/inspector_protocol/lib/Object_h.template',
				'../vendor/node/tools/inspector_protocol/lib/Parser_cpp.template',
				'../vendor/node/tools/inspector_protocol/lib/Parser_h.template',
				'../vendor/node/tools/inspector_protocol/lib/Protocol_cpp.template',
				'../vendor/node/tools/inspector_protocol/lib/ValueConversions_h.template',
				'../vendor/node/tools/inspector_protocol/lib/Values_cpp.template',
				'../vendor/node/tools/inspector_protocol/lib/Values_h.template',
				'../vendor/node/tools/inspector_protocol/templates/Exported_h.template',
				'../vendor/node/tools/inspector_protocol/templates/Imported_h.template',
				'../vendor/node/tools/inspector_protocol/templates/TypeBuilder_cpp.template',
				'../vendor/node/tools/inspector_protocol/templates/TypeBuilder_h.template',
				'../vendor/node/tools/inspector_protocol/code_generator.py',
			}
			
			buildinputs(nodeInputs)
			buildoutputs {
				'../vendor/node/gen/src/node/inspector/protocol/Forward.h',
				'../vendor/node/gen/src/node/inspector/protocol/Protocol.cpp',
				'../vendor/node/gen/src/node/inspector/protocol/Protocol.h',
				'../vendor/node/gen/src/node/inspector/protocol/NodeWorker.cpp',
				'../vendor/node/gen/src/node/inspector/protocol/NodeWorker.h',
				'../vendor/node/gen/src/node/inspector/protocol/NodeTracing.cpp',
				'../vendor/node/gen/src/node/inspector/protocol/NodeTracing.h',
				'../vendor/node/gen/src/node/inspector/protocol/NodeRuntime.cpp',
				'../vendor/node/gen/src/node/inspector/protocol/NodeRuntime.h',
				'../vendor/node/gen/src/node/inspector/protocol/v8_inspector_protocol_json.h'
			}

		filter {}

		if os.istarget('windows') then
			files_project '../vendor/node/' {
				'src/node_javascript.cc', -- with msc, commands that output C/C++ source files are not fed into the build process yet
				'gen/src/node/inspector/protocol/Forward.h',
				'gen/src/node/inspector/protocol/Protocol.cpp',
				'gen/src/node/inspector/protocol/Protocol.h',
				'gen/src/node/inspector/protocol/NodeWorker.cpp',
				'gen/src/node/inspector/protocol/NodeWorker.h',
				'gen/src/node/inspector/protocol/NodeTracing.cpp',
				'gen/src/node/inspector/protocol/NodeTracing.h',
				'gen/src/node/inspector/protocol/NodeRuntime.cpp',
				'gen/src/node/inspector/protocol/NodeRuntime.h',
				'gen/src/node/inspector/protocol/v8_inspector_protocol_json.h'
			}
		else
			files_project '../vendor/node/' {
				--'src/backtrace_posix.cc'
			}
		end

		files_project '../vendor/node/' {
			'src/api/async_resource.cc',
			'src/api/callback.cc',
			'src/api/embed_helpers.cc',
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
			'src/histogram.cc',
			'src/js_native_api.h',
			'src/js_native_api_types.h',
			'src/js_native_api_v8.cc',
			'src/js_native_api_v8.h',
			'src/js_native_api_v8_internals.h',
			'src/js_stream.cc',
			'src/json_utils.cc',
			'src/js_udp_wrap.cc',
			'src/module_wrap.cc',
			'src/node.cc',
			'src/node_api.cc',
			'src/node_binding.cc',
			'src/node_blob.cc',
			'src/node_buffer.cc',
			'src/node_config.cc',
			'src/node_constants.cc',
			'src/node_contextify.cc',
			'src/node_credentials.cc',
			'src/node_dir.cc',
			'src/node_env_var.cc',
			'src/node_errors.cc',
			'src/node_external_reference.cc',
			'src/node_file.cc',
			'src/node_http_parser.cc',
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
			'src/node_report.cc',
			'src/node_report_module.cc',
			'src/node_report_utils.cc',
			'src/node_serdes.cc',
			'src/node_snapshotable.cc',
			'src/node_sockaddr.cc',
			'src/node_stat_watcher.cc',
			'src/node_symbols.cc',
			'src/node_task_queue.cc',
			'src/node_trace_events.cc',
			'src/node_types.cc',
			'src/node_url.cc',
			'src/node_url_tables.cc',
			'src/node_util.cc',
			'src/node_v8.cc',
			'src/node_wasi.cc',
			'src/node_watchdog.cc',
			'src/node_worker.cc',
			'src/node_zlib.cc',
			'src/pipe_wrap.cc',
			'src/process_wrap.cc',
			'src/signal_wrap.cc',
			'src/spawn_sync.cc',
			'src/stream_base.cc',
			'src/stream_pipe.cc',
			'src/stream_wrap.cc',
			'src/string_bytes.cc',
			'src/string_decoder.cc',
			'src/tcp_wrap.cc',
			'src/timers.cc',
			'src/timer_wrap.cc',
			'src/tracing/agent.cc',
			'src/tracing/node_trace_buffer.cc',
			'src/tracing/node_trace_writer.cc',
			'src/tracing/trace_event.cc',
			'src/tracing/traced_value.cc',
			'src/tty_wrap.cc',
			'src/udp_wrap.cc',
			'src/util.cc',
			'src/uv.cc',
			--headers to make for a more pleasant IDE experience
			'src/aliased_buffer.h',
			'src/aliased_struct.h',
			'src/aliased_struct-inl.h',
			'src/allocated_buffer.h',
			'src/allocated_buffer-inl.h',
			'src/async_wrap.h',
			'src/async_wrap-inl.h',
			'src/base_object.h',
			'src/base_object-inl.h',
			'src/base64.h',
			'src/base64-inl.h',
			'src/callback_queue.h',
			'src/callback_queue-inl.h',
			'src/connect_wrap.h',
			'src/connection_wrap.h',
			'src/debug_utils.h',
			'src/debug_utils-inl.h',
			'src/env.h',
			'src/env-inl.h',
			'src/handle_wrap.h',
			'src/histogram.h',
			'src/histogram-inl.h',
			'src/js_stream.h',
			'src/json_utils.h',
			'src/large_pages/node_large_page.cc',
			'src/large_pages/node_large_page.h',
			'src/memory_tracker.h',
			'src/memory_tracker-inl.h',
			'src/module_wrap.h',
			'src/node.h',
			'src/node_api.h',
			'src/node_api_types.h',
			'src/node_binding.h',
			'src/node_blob.h',
			'src/node_buffer.h',
			'src/node_constants.h',
			'src/node_context_data.h',
			'src/node_contextify.h',
			'src/node_dir.h',
			'src/node_errors.h',
			'src/node_external_reference.h',
			'src/node_file.h',
			'src/node_file-inl.h',
			'src/node_http_common.h',
			'src/node_http_common-inl.h',
			'src/node_http2.h',
			'src/node_http2_state.h',
			'src/node_i18n.h',
			'src/node_internals.h',
			'src/node_main_instance.h',
			'src/node_mem.h',
			'src/node_mem-inl.h',
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
			'src/node_process-inl.h',
			'src/node_report.h',
			'src/node_revert.h',
			'src/node_root_certs.h',
			'src/node_snapshotable.h',
			'src/node_sockaddr.h',
			'src/node_sockaddr-inl.h',
			'src/node_stat_watcher.h',
			'src/node_union_bytes.h',
			'src/node_url.h',
			'src/node_version.h',
			'src/node_v8.h',
			'src/node_v8_platform-inl.h',
			'src/node_wasi.h',
			'src/node_watchdog.h',
			'src/node_worker.h',
			'src/pipe_wrap.h',
			'src/req_wrap.h',
			'src/req_wrap-inl.h',
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
			'src/timer_wrap.h',
			'src/tty_wrap.h',
			'src/udp_wrap.h',
			'src/util.h',
			'src/util-inl.h',
			--Dependency headers
			'../../code/deplibs/include/include/v8.h',

			'deps/uvwasi/src/uvwasi.h',
			'deps/uvwasi/src/uvwasi.c',

			'src/node_snapshot_stub.cc',
			'src/node_code_cache_stub.cc',
			
			-- use_openssl
			'src/crypto/crypto_aes.cc',
			'src/crypto/crypto_bio.cc',
			'src/crypto/crypto_common.cc',
			'src/crypto/crypto_dsa.cc',
			'src/crypto/crypto_hkdf.cc',
			'src/crypto/crypto_pbkdf2.cc',
			'src/crypto/crypto_sig.cc',
			'src/crypto/crypto_timing.cc',
			'src/crypto/crypto_cipher.cc',
			'src/crypto/crypto_context.cc',
			'src/crypto/crypto_ec.cc',
			'src/crypto/crypto_hmac.cc',
			'src/crypto/crypto_random.cc',
			'src/crypto/crypto_rsa.cc',
			'src/crypto/crypto_spkac.cc',
			'src/crypto/crypto_util.cc',
			'src/crypto/crypto_clienthello.cc',
			'src/crypto/crypto_dh.cc',
			'src/crypto/crypto_hash.cc',
			'src/crypto/crypto_keys.cc',
			'src/crypto/crypto_keygen.cc',
			'src/crypto/crypto_scrypt.cc',
			'src/crypto/crypto_tls.cc',
			'src/crypto/crypto_aes.cc',
			'src/crypto/crypto_x509.cc',
			'src/crypto/crypto_bio.h',
			'src/crypto/crypto_clienthello-inl.h',
			'src/crypto/crypto_dh.h',
			'src/crypto/crypto_groups.h',
			'src/crypto/crypto_hmac.h',
			'src/crypto/crypto_rsa.h',
			'src/crypto/crypto_spkac.h',
			'src/crypto/crypto_util.h',
			'src/crypto/crypto_cipher.h',
			'src/crypto/crypto_common.h',
			'src/crypto/crypto_dsa.h',
			'src/crypto/crypto_hash.h',
			'src/crypto/crypto_keys.h',
			'src/crypto/crypto_keygen.h',
			'src/crypto/crypto_scrypt.h',
			'src/crypto/crypto_tls.h',
			'src/crypto/crypto_clienthello.h',
			'src/crypto/crypto_context.h',
			'src/crypto/crypto_ec.h',
			'src/crypto/crypto_hkdf.h',
			'src/crypto/crypto_pbkdf2.h',
			'src/crypto/crypto_sig.h',
			'src/crypto/crypto_random.h',
			'src/crypto/crypto_timing.h',
			'src/crypto/crypto_x509.h',
			'src/node_crypto.cc',
			'src/node_crypto.h',
			
			-- have_inspector
			'src/inspector_agent.cc',
			'src/inspector_io.cc',
			'src/inspector_agent.h',
			'src/inspector_io.h',
			'src/inspector_profiler.h',
			'src/inspector_profiler.cc',
			'src/inspector_js_api.cc',
			'src/inspector_socket.cc',
			'src/inspector_socket.h',
			'src/inspector_socket_server.cc',
			'src/inspector_socket_server.h',
			'src/inspector/main_thread_interface.cc',
			'src/inspector/main_thread_interface.h',
			'src/inspector/node_string.cc',
			'src/inspector/node_string.h',
			'src/inspector/runtime_agent.cc',
			'src/inspector/runtime_agent.h',
			'src/inspector/tracing_agent.cc',
			'src/inspector/tracing_agent.h',
			'src/inspector/worker_agent.cc',
			'src/inspector/worker_agent.h',
			'src/inspector/worker_inspector.cc',
			'src/inspector/worker_inspector.h',
		}
	end
}
