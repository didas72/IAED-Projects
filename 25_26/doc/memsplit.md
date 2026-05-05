# Memsplit

Tree containing the allocation (if any) that encompasses any given address.

Each level splits memory in 2^BPL segments, which eventually maps the entire address space.

In code, each level is defined by a block, which holds 2^BPL segment entries (pointers to further blocks). Each entry can be NULL if the space is not allocated. The last level of memsplit does not point to further entries. Instead, the second to last level holds the actual allocation bases that encompass the addresses, or NULL if not allocated.

LATER:
As an optimization, if two or more consecutive pointers to the lower levels are equal, they are assumed to NOT point to further entries, and instead encode the allocation base that encompasses the entire segment.
