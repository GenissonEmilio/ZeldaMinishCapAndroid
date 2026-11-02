#include "rom_regions.h"
#include "runtime_arm.h"
#include "symbol_lookup.h"
#include <cassert>
#include <cstring>
#include <initializer_list>

struct DispatchEntry { uint32_t addr; uint8_t thumb, resume; void (*fn)(); };
int executed=0;
void usa() { executed=1; }
void eu() { executed=2; }
extern "C" const DispatchEntry kDispatchTable[]={{0x08000100,1,0,usa}};
extern "C" const DispatchEntry eu_kDispatchTable[]={{0x08000100,1,0,eu}};
extern "C" const unsigned kDispatchTableLen=1,eu_kDispatchTableLen=1;
const RuntimeNativeRamTemplate* active_templates=nullptr;
const RuntimeNativeRamEntry* active_entries=nullptr;
extern "C" void runtime_register_native_ram_templates(const RuntimeNativeRamTemplate* t,uint32_t n,
    const RuntimeNativeRamEntry* e,uint32_t m) {
    active_templates=t; active_entries=e; assert(n==1 && m==1);
}
extern "C" void minish_usa_symbols(const GbaSymbol*,unsigned);
extern "C" void minish_eu_symbols(const GbaSymbol*,unsigned);
extern "C" void minish_usa_data(const GbaDataSymbol*,unsigned);
extern "C" void minish_eu_data(const GbaDataSymbol*,unsigned);
extern "C" void minish_usa_ram(const RuntimeNativeRamTemplate*,uint32_t,const RuntimeNativeRamEntry*,uint32_t);
extern "C" void minish_eu_ram(const RuntimeNativeRamTemplate*,uint32_t,const RuntimeNativeRamEntry*,uint32_t);
int main() {
    const GbaSymbol us_symbols[]={{0x08000100,"USA"}},eu_symbols[]={{0x08000100,"EU"}};
    const GbaDataSymbol us_data[]={{0x02000000,4,"USA data"}},eu_data[]={{0x02000000,4,"EU data"}};
    const RuntimeNativeRamTemplate us_ram[1]{},eu_ram[1]{};
    const RuntimeNativeRamEntry us_entries[1]{},eu_entries[1]{};
    minish_usa_symbols(us_symbols,1);minish_usa_data(us_data,1);minish_usa_ram(us_ram,1,us_entries,1);
    minish_eu_symbols(eu_symbols,1);minish_eu_data(eu_data,1);minish_eu_ram(eu_ram,1,eu_entries,1);
    // Generated initializers must not install another region's executable RAM.
    assert(active_templates==nullptr);
    for(bool europe:{false,true,false}) {
        minish::select_europe(europe);
        assert(minish::is_europe()==europe && minish::cart_dispatch_count()==1);
        minish::cart_dispatch_table()[0].fn();
        assert(executed==(europe?2:1));
        assert(active_templates==(europe?eu_ram:us_ram));
        assert(active_entries==(europe?eu_entries:us_entries));
        uint32_t offset=0;
        assert(std::strcmp(gba_symbol_lookup(0x08000100,&offset),europe?"EU":"USA")==0);
        assert(std::strcmp(gba_data_symbol_lookup(0x02000000,&offset),europe?"EU data":"USA data")==0);
    }
}
