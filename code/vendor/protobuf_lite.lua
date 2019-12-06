return {
        include = function()
                includedirs { "../code/vendor/protobuf/vsprojects/", "../vendor/protobuf/src/", "../vendor/protobuf/vsprojects/" }
        end,

	run = function()
		targetname "protobuf_lite"
		language "C++"
		kind "StaticLib"

		if os.istarget('windows') then
			configuration "windows"
			buildoptions "/MP /wd4244 /wd4267 /wd4018 /wd4355 /wd4800 /wd4251 /wd4996 /wd4146 /wd4305"
		elseif os.istarget('linux') then
			defines { 'HAVE_PTHREAD' }
		end
		
		-- note: not actually the 'lite' file set anymore
		files
		{
			-- lite
			"../vendor/protobuf/src/google/protobuf/any_lite.cc",
			"../vendor/protobuf/src/google/protobuf/arena.cc",
			"../vendor/protobuf/src/google/protobuf/extension_set.cc",
			"../vendor/protobuf/src/google/protobuf/generated_enum_util.cc",
			"../vendor/protobuf/src/google/protobuf/generated_message_table_driven_lite.cc",
			"../vendor/protobuf/src/google/protobuf/generated_message_util.cc",
			"../vendor/protobuf/src/google/protobuf/implicit_weak_message.cc",
			"../vendor/protobuf/src/google/protobuf/io/coded_stream.cc",
			"../vendor/protobuf/src/google/protobuf/io/io_win32.cc",
			"../vendor/protobuf/src/google/protobuf/io/strtod.cc",
			"../vendor/protobuf/src/google/protobuf/io/zero_copy_stream.cc",
			"../vendor/protobuf/src/google/protobuf/io/zero_copy_stream_impl.cc",
			"../vendor/protobuf/src/google/protobuf/io/zero_copy_stream_impl_lite.cc",
			"../vendor/protobuf/src/google/protobuf/message_lite.cc",
			"../vendor/protobuf/src/google/protobuf/parse_context.cc",
			"../vendor/protobuf/src/google/protobuf/repeated_field.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/bytestream.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/common.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/int128.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/status.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/statusor.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/stringpiece.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/stringprintf.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/structurally_valid.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/strutil.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/time.cc",
			"../vendor/protobuf/src/google/protobuf/wire_format_lite.cc",

			-- nonlite
			"../vendor/protobuf/src/google/protobuf/any.cc",
			"../vendor/protobuf/src/google/protobuf/any.pb.cc",
			"../vendor/protobuf/src/google/protobuf/api.pb.cc",
			"../vendor/protobuf/src/google/protobuf/compiler/importer.cc",
			"../vendor/protobuf/src/google/protobuf/compiler/parser.cc",
			"../vendor/protobuf/src/google/protobuf/descriptor.cc",
			"../vendor/protobuf/src/google/protobuf/descriptor.pb.cc",
			"../vendor/protobuf/src/google/protobuf/descriptor_database.cc",
			"../vendor/protobuf/src/google/protobuf/duration.pb.cc",
			"../vendor/protobuf/src/google/protobuf/dynamic_message.cc",
			"../vendor/protobuf/src/google/protobuf/empty.pb.cc",
			"../vendor/protobuf/src/google/protobuf/extension_set_heavy.cc",
			"../vendor/protobuf/src/google/protobuf/field_mask.pb.cc",
			"../vendor/protobuf/src/google/protobuf/generated_message_reflection.cc",
			"../vendor/protobuf/src/google/protobuf/generated_message_table_driven.cc",
			"../vendor/protobuf/src/google/protobuf/io/gzip_stream.cc",
			"../vendor/protobuf/src/google/protobuf/io/printer.cc",
			"../vendor/protobuf/src/google/protobuf/io/tokenizer.cc",
			"../vendor/protobuf/src/google/protobuf/map_field.cc",
			"../vendor/protobuf/src/google/protobuf/message.cc",
			"../vendor/protobuf/src/google/protobuf/reflection_ops.cc",
			"../vendor/protobuf/src/google/protobuf/service.cc",
			"../vendor/protobuf/src/google/protobuf/source_context.pb.cc",
			"../vendor/protobuf/src/google/protobuf/struct.pb.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/mathlimits.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/substitute.cc",
			"../vendor/protobuf/src/google/protobuf/text_format.cc",
			"../vendor/protobuf/src/google/protobuf/timestamp.pb.cc",
			"../vendor/protobuf/src/google/protobuf/type.pb.cc",
			"../vendor/protobuf/src/google/protobuf/unknown_field_set.cc",
			"../vendor/protobuf/src/google/protobuf/util/delimited_message_util.cc",
			"../vendor/protobuf/src/google/protobuf/util/field_comparator.cc",
			"../vendor/protobuf/src/google/protobuf/util/field_mask_util.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/datapiece.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/default_value_objectwriter.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/error_listener.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/field_mask_utility.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/json_escaping.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/json_objectwriter.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/json_stream_parser.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/object_writer.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/proto_writer.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/protostream_objectsource.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/protostream_objectwriter.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/type_info.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/type_info_test_helper.cc",
			"../vendor/protobuf/src/google/protobuf/util/internal/utility.cc",
			"../vendor/protobuf/src/google/protobuf/util/json_util.cc",
			"../vendor/protobuf/src/google/protobuf/util/message_differencer.cc",
			"../vendor/protobuf/src/google/protobuf/util/time_util.cc",
			"../vendor/protobuf/src/google/protobuf/util/type_resolver_util.cc",
			"../vendor/protobuf/src/google/protobuf/wire_format.cc",
			"../vendor/protobuf/src/google/protobuf/wrappers.pb.cc",
		}

	end
}
