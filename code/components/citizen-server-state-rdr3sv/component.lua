return function()
	filter {}
	configuration {}
	defines { 'STATE_RDR3' }

	dofile('components/citizen-server-state/init.lua')
end