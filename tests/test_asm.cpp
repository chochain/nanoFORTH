///
/// Unit Test - NanoForth Assembler
/// 
///> g++ -std=c++14 -Wall -L/home/gnii/devel/catch2/ -lcatch test_asm.cpp
///
#define  CATCH_CONFIG_MAIN
#include "../../catch2/catch.hpp"
#include "../src/nanoforth_asm.h"

typedef struct {
    const char *str;
    U16 rst;
} vec;

TEST_CASE("U8 query(U8 *tkn, U16 *adr)")
{
    const char dic[] =
        "\xff" "\xff" "ABC" "\xd0" \
        "\x00" "\x00" "DE " "\xd0" \
        "\x06" "\x00" "GHI" "\xd0" \
        "\x0c" "\x00" "J  " "\xd0";
    vec tkn[] = {
        { "ABC", 0  },
        { "DE ", 6  },
        { "GHI", 12 },
        { "J ",  18 }
    };
    N4Asm n4((U8*)dic);
    U8 *last0 = (U8*)&dic[18];
    U8 *here0 = (U8*)&dic[24];
    
    SECTION("found") {
        for (U16 i=0; i<sizeof(tkn)/sizeof(vec); i++) {
            n4.last = last0;
            n4.here = here0;
            SECTION(tkn[i].str) {
                U16 adr;
                U8  n = n4.query((U8*)tkn[i].str, &adr);
                REQUIRE(1==n);
                REQUIRE(tkn[i].rst==adr);
                REQUIRE(last0==n4.last);
            }
        }
    }
    SECTION("not found") {
        n4.last = last0;
        n4.here = here0;
        U16  adr;
        U8  n = n4.query((U8*)"XXX", &adr);
        REQUIRE(0==n);
        REQUIRE(last0==n4.last);
    }
}
