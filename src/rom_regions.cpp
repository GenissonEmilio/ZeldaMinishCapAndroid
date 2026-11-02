#include "rom_regions.h"
#include "runtime_arm.h"
#include "symbol_lookup.h"

struct DispatchEntry {
    std::uint32_t addr;
    std::uint8_t thumb, resume;
    void (*fn)(void);
};
extern "C" const DispatchEntry kDispatchTable[], eu_kDispatchTable[];
extern "C" const unsigned kDispatchTableLen, eu_kDispatchTableLen;

namespace {
struct Corpus {
    const GbaSymbol* symbols = nullptr;
    unsigned symbol_count = 0;
    const GbaDataSymbol* data = nullptr;
    unsigned data_count = 0;
    const RuntimeNativeRamTemplate* templates = nullptr;
    std::uint32_t template_count = 0;
    const RuntimeNativeRamEntry* entries = nullptr;
    std::uint32_t entry_count = 0;
};
// Constant-initialized: generated static constructors may run before this TU.
Corpus corpora[2];
bool europe = false;
}

// Capture each corpus's generated registrations without installing the other
// region's symbol names or executable RAM templates into the active runtime.
#define REGISTRATIONS(region, index) \
extern "C" void minish_##region##_symbols(const GbaSymbol* t, unsigned n) { \
    corpora[index].symbols=t; corpora[index].symbol_count=n; \
} \
extern "C" void minish_##region##_data(const GbaDataSymbol* t, unsigned n) { \
    corpora[index].data=t; corpora[index].data_count=n; \
} \
extern "C" void minish_##region##_ram(const RuntimeNativeRamTemplate* t, \
    std::uint32_t n, const RuntimeNativeRamEntry* e, std::uint32_t m) { \
    corpora[index].templates=t; corpora[index].template_count=n; \
    corpora[index].entries=e; corpora[index].entry_count=m; \
}
REGISTRATIONS(usa, 0)
REGISTRATIONS(eu, 1)
#undef REGISTRATIONS

namespace minish {
void select_europe(bool value) {
    europe=value;
    const auto& c=corpora[europe ? 1 : 0];
    gba_symbol_register_cart(c.symbols,c.symbol_count);
    gba_data_symbol_register_cart(c.data,c.data_count);
    runtime_register_native_ram_templates(c.templates,c.template_count,c.entries,c.entry_count);
}
bool is_europe() { return europe; }
const DispatchEntry* cart_dispatch_table() { return europe ? eu_kDispatchTable : kDispatchTable; }
unsigned cart_dispatch_count() { return europe ? eu_kDispatchTableLen : kDispatchTableLen; }
}
