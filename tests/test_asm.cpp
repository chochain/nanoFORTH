///
/// Unit Test - NanoForth Assembler
/// 
///> g++ -std=c++14 -Wall -L/home/gnii/devel/catch2/ -lcatch ../src/nanoforth_asm.cpp ../src/nanoforth_core.cpp test_asm.cpp && a.out
///
#define  CATCH_CONFIG_MAIN
#include "../../catch2/catch.hpp"
#include "../src/nanoforth_asm.h"

typedef struct {
    const char *str;
    U16 rst;
} vec;

typedef struct {
    U16  prev;
    char name[3];
    U8   op;
} hdr;

TEST_CASE("U8 query(U8 *tkn, U16 *adr)")
{
    // note: address in Big-Endian 
    const char dic[] = \
        "\xff" "\xff" "ABC" "\x00"  \
        "\x00" "\x00" "DE " "\x00"  \
        "\x00" "\x06" "GHI" "\x00"  \
        "\x00" "\x0c" "J  " "\x00";
    // for (U16 i=0; i<24; i++) printf("%02x ", (U8)dic[i]);
    // positive cases
    vec tkn_1[] = {
        { "ABC", 0  },
        { "DE ", 6  },
        { "GHI", 12 },
        { "J ",  18 }
    };
    // negative cases
    vec tkn_0[] = {
        { "XXX", 0  },
        { "de ", 0  },
        { "j ",  0  }
    };
    N4Asm n4((U8*)dic);
    U8 *last0 = (U8*)&dic[18];
    U8 *here0 = (U8*)&dic[24];
    
    SECTION("found") {
        for (U16 i=0; i<sizeof(tkn_1)/sizeof(vec); i++) {
            n4.last = last0;
            n4.here = here0;
            SECTION(tkn_1[i].str) {
                U16 adr;
                U8  n = n4.query((U8*)tkn_1[i].str, &adr);
                REQUIRE(1==n);
                REQUIRE(tkn_1[i].rst==adr);
                REQUIRE(last0==n4.last);
            }
        }
    }
    SECTION("not found") {
        n4.last = last0;
        n4.here = here0;
        for (U16 i=0; i<sizeof(tkn_0)/sizeof(vec); i++) {
            n4.last = last0;
            n4.here = here0;
            SECTION(tkn_0[i].str) {
                U16 adr;
                U8  n = n4.query((U8*)tkn_0[i].str, &adr);
                REQUIRE(tkn_0[i].rst==n);
                REQUIRE(last0==n4.last);
            }
        }
    }
}
