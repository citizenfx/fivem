if os.istarget('windows') and not _OPTIONS['with-asan'] then
	flags { "LinkTimeOptimization" }
end