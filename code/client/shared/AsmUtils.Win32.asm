.code

Win32TrapAndJump64 proc
	pushfq
	or qword ptr [rsp], 100h
	popfq

	jmp qword ptr gs:[38h]
Win32TrapAndJump64 endp

end
