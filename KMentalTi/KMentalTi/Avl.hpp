#ifndef AVL_HPP
#define AVL_HPP

#include <ntifs.h>
#include "Global.hpp"

RTL_GENERIC_COMPARE_RESULTS AvlCompare(RTL_AVL_TABLE* Table, PVOID First, PVOID Second);
VOID AvlFree(RTL_AVL_TABLE* Table, PVOID Buffer);
PVOID AvlAlloc(RTL_AVL_TABLE* Table, CLONG Bytes);
VOID AvlInsert(ULONG Key, ULONG Value);
bool AvlDelete(ULONG Key);
void AvlNuke();

#endif // !AVL_HPP

