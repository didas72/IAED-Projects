#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>

#include <libunwind.h>

static int fd = -1;

#define N_REGS 16
static uint64_t registers[N_REGS];



typedef enum {
	TRACE_TYPE_NONE = 0,
	TRACE_TYPE_BEGIN = 1,
	TRACE_TYPE_REGS = 2,
	TRACE_TYPE_STRACE = 3,
} header_type_e;

#define TRACE_SIZE_ANY ~(uint64_t)0



void cgcl_init()
{
	fd = open("cgcl.bin", O_CREAT | O_TRUNC | O_RDWR);
}



__attribute__((naked))
void cgcl_trace()
{
	__asm__ volatile(
		"mov %rax, registers+0(%rip)\n"
		"mov %rcx, registers+8(%rip)\n"
		"mov %rdx, registers+16(%rip)\n"
		"mov %rbx, registers+24(%rip)\n"
		"mov %rsi, registers+32(%rip)\n"
		"mov %rdi, registers+40(%rip)\n"
		"mov %rsp, registers+48(%rip)\n"
		"mov %rbp, registers+56(%rip)\n"
		"mov %r8,  registers+64(%rip)\n"
		"mov %r9,  registers+72(%rip)\n"
		"mov %r10, registers+80(%rip)\n"
		"mov %r11, registers+88(%rip)\n"
		"mov %r12, registers+96(%rip)\n"
		"mov %r13, registers+104(%rip)\n"
		"mov %r14, registers+112(%rip)\n"
		"mov %r15, registers+120(%rip)\n"

		"jmp cgcl_trace_inner\n"
	);
}

static inline void dump_header(header_type_e type, uint64_t len)
{
	uint64_t hdr[2] = { (uint64_t)type, len };
	write(fd, &hdr, sizeof(hdr));
}

static inline void cgcl_regdump()
{
	dump_header(TRACE_TYPE_REGS, sizeof(registers));
	write(fd, registers, sizeof(registers));
}

static inline void cgcl_backtrace(unw_cursor_t *cursor, unw_context_t *context)
{
	dump_header(TRACE_TYPE_STRACE, TRACE_SIZE_ANY);
	while (unw_step(cursor) > 0)
	{
		unw_word_t ip;
		unw_get_reg(cursor, UNW_REG_IP, &ip);
		//TODO: Read 'nromal' registers with unw_get_reg when returning 0 (not UNW_EBADREG)
		//TODO: Save regs
		//TODO: Save function name? (read next)
		//TODO: Besides rip, might also be fun to get start_ip to resolve symbols instead of using function name
	}
}

static void cgcl_trace_inner()
{
	unw_cursor_t cursor;
	unw_context_t context;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	if (fd == -1) cgcl_init();

	dump_header(TRACE_TYPE_BEGIN, 0);
	cgcl_regdump();
	cgcl_backtrace(&cursor, &context);
}
