#pragma once
#include <cstdint>

struct DispatchEntry;
namespace minish {
// Select before starting the runtime; never switch a running guest's corpus.
void select_europe(bool europe);
bool is_europe();
const DispatchEntry* cart_dispatch_table();
unsigned cart_dispatch_count();
}
