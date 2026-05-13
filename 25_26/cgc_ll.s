#TODO: Move in_cgc logic here and bypass saving of registers as well

.global malloc
.global free

malloc:
	push %rbx
	push %r12
	push %r13
	push %r14
	push %r15

	# Argument can stay in rdi
	# Invoke C malloc
	call cgc_malloc

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %rbx
	ret

free:
	push %rbx
	push %r12
	push %r13
	push %r14
	push %r15

	# Argument can stay in rdi
	# Invoke C free
	call cgc_free

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %rbx
	ret

#TODO: Other allocs and free
