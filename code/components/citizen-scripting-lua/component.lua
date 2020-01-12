if os.istarget('windows') and not _OPTIONS['with-asan'] then
	if _OPTIONS["game"] ~= "ny" then
		flags { "LinkTimeOptimization" }
	end

	buildoptions '/Zc:threadSafeInit- /EHa /fp:fast'
end