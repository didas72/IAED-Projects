#TODO: Move in_cgc logic here and bypass saving of registers as well

.section .bss

.global _cgc_callee_registers
.global _cgc_in_gc
.global _cgc_real_malloc
.global _cgc_real_calloc
.global _cgc_real_free
.global _cgc_real_realloc
.global _cgc_real_reallocarray

.hidden _cgc_callee_registers
.hidden _cgc_in_gc
.hidden _cgc_real_malloc
.hidden _cgc_real_calloc
.hidden _cgc_real_free
.hidden _cgc_real_realloc
.hidden _cgc_real_reallocarray

# Callee saved registers for use during marking
_cgc_callee_registers:
#     RBX, R12, R13, R14, R15
.quad 0,   0,   0,   0,   0

# Avoid controlling mallocs within CGC
_cgc_in_gc:
.quad 0

# Real function addresses (filled by C code)
_cgc_real_malloc: .quad 0
_cgc_real_calloc: .quad 0
_cgc_real_free:   .quad 0
_cgc_real_realloc:   .quad 0
_cgc_real_reallocarray:   .quad 0

.section .text

.global malloc
.global free

malloc:
	mov _cgc_in_gc(%rip), %rax
	cmp $0, %rax
	je _malloc_tracked
	mov _cgc_real_malloc(%rip), %rax
	jmp *%rax

_malloc_tracked:
	movq $1, _cgc_in_gc(%rip)
	lea _cgc_callee_registers(%rip), %r10
	mov %rbx,   (%r10)
	mov %r12,  8(%r10)
	mov %r13, 16(%r10)
	mov %r14, 24(%r10)
	mov %r15, 32(%r10)

	# Argument stays in rdi
	jmp cgc_malloc

calloc:
	mov _cgc_in_gc(%rip), %rax
	cmp $0, %rax
	je _calloc_tracked
	mov _cgc_real_calloc(%rip), %rax
	jmp *%rax

_calloc_tracked:
	movq $1, _cgc_in_gc(%rip)
	lea _cgc_callee_registers(%rip), %r10
	mov %rbx,   (%r10)
	mov %r12,  8(%r10)
	mov %r13, 16(%r10)
	mov %r14, 24(%r10)
	mov %r15, 32(%r10)

	# Arguments stay in rdi, rsi
	jmp cgc_calloc

free:
	mov _cgc_in_gc(%rip), %rax
	cmp $0, %rax
	je _free_tracked
	mov _cgc_real_free(%rip), %rax
	jmp *%rax

_free_tracked:
	movq $1, _cgc_in_gc(%rip)
	lea _cgc_callee_registers(%rip), %r10
	mov %rbx,   (%r10)
	mov %r12,  8(%r10)
	mov %r13, 16(%r10)
	mov %r14, 24(%r10)
	mov %r15, 32(%r10)

	# Argument stays in rdi
	jmp cgc_free

realloc:
	mov _cgc_in_gc(%rip), %rax
	cmp $0, %rax
	je _realloc_tracked
	mov _cgc_real_realloc(%rip), %rax
	jmp *%rax

_realloc_tracked:
	movq $1, _cgc_in_gc(%rip)
	lea _cgc_callee_registers(%rip), %r10
	mov %rbx,   (%r10)
	mov %r12,  8(%r10)
	mov %r13, 16(%r10)
	mov %r14, 24(%r10)
	mov %r15, 32(%r10)

	# Argument stays in rdi, rsi
	jmp cgc_realloc

reallocarray:
	mov _cgc_in_gc(%rip), %rax
	cmp $0, %rax
	je _reallocarray_tracked
	mov _cgc_real_reallocarray(%rip), %rax
	jmp *%rax

_reallocarray_tracked:
	movq $1, _cgc_in_gc(%rip)
	lea _cgc_callee_registers(%rip), %r10
	mov %rbx,   (%r10)
	mov %r12,  8(%r10)
	mov %r13, 16(%r10)
	mov %r14, 24(%r10)
	mov %r15, 32(%r10)

	# Argument stays in rdi, rsi
	jmp cgc_reallocarray
