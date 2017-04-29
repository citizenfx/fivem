return {
	include = function()
		includedirs { "../vendor/leveldb/include/" }

		links { 'shlwapi' }
	end,

	run = function()
		targetname "leveldb"
		language "C++"
		kind "StaticLib"

		defines { 'LEVELDB_PLATFORM_WINDOWS' }

		includedirs {
			"../vendor/leveldb/",
			"../vendor/leveldb/port/win/"
		}
		
		files_project '../vendor/leveldb/'
		{
			'db/builder.cc',
			'db/c.cc',
			'db/dbformat.cc',
			'db/db_bench.cc',
			'db/db_impl.cc',
			'db/db_iter.cc',
			'db/filename.cc',
			'db/log_reader.cc',
			'db/log_writer.cc',
			'db/memtable.cc',
			'db/repair.cc',
			'db/table_cache.cc',
			'db/version_edit.cc',
			'db/version_set.cc',
			'db/write_batch.cc',
			'helpers/memenv/memenv.cc',
			'port/port_win.cc',
			'port/port_win_sse.cc',
			'table/block.cc',
			'table/block_builder.cc',
			'table/filter_block.cc',
			'table/format.cc',
			'table/iterator.cc',
			'table/merger.cc',
			'table/table.cc',
			'table/table_builder.cc',
			'table/two_level_iterator.cc',
			'util/arena.cc',
			'util/bloom.cc',
			'util/cache.cc',
			'util/coding.cc',
			'util/comparator.cc',
			'util/env.cc',
			'util/env_win.cc',
			'util/filter_policy.cc',
			'util/hash.cc',
			'util/histogram.cc',
			'util/logging.cc',
			'util/options.cc',
			'util/status.cc',
			'util/testharness.cc',
			'util/testutil.cc',
			'util/crc32c.cc'
		}
	end
}
