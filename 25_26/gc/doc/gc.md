# Garbage collection

Intended functioning:

- allocs are wrapped to track allocations
- upon GC run, each active variable is checked
- all allocations not in reach from stack or nested pointers (NO GLOBALS!) are freed (mark and sweep)
- nested pointers tracked by checking alloc spans, if ptr falls inside span, it's a track
- GC is run when? Every N allocs?
