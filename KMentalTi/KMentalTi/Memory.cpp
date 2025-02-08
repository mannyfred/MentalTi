#include "Memory.hpp"

void* __cdecl operator new(size_t size, PoolType pool, ULONG tag) {
    void* p = ExAllocatePool2((POOL_FLAGS)pool, size, tag);
    return p;
}

void* __cdecl operator new[](size_t size, PoolType pool, ULONG tag) {
    void* p = ExAllocatePool2((POOL_FLAGS)pool, size, tag);
    return p;
}

void __cdecl operator delete(void* p, size_t) {
    NT_ASSERT(p);
    ExFreePool(p);
}

void __cdecl operator delete[](void* p, size_t) {
    NT_ASSERT(p);
    ExFreePool(p);
}