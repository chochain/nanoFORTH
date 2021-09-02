///
/// Unit Test - NanoForth Core Helper
/// 
///> g++ -std=c++14 -Wall -L/home/gnii/devel/catch2/ -lcatch ../src/nanoforth_core.cpp test_core.cpp && a.out
///
#define  CATCH_CONFIG_MAIN
#include "../../../catch2/catch.hpp"
#include "../src/nanoforth_core.h"

N4Core n4;

typedef struct {
    const char *str;
    U16   rst;
} vec;
typedef struct {
    const char *gname;
    vec   *t;
    int   n;
} grp;

TEST_CASE("U8 number(U8 *str, S16 *num)")
{
    vec _hex[] = {
        { "$0 ",    0x0    }, { "$1 ",    0x1    }, { "$8 ",    0x8    }, { "$9 ",    0x9 },
        { "$A ",    0xa    }, { "$F ",    0xf    },
        { "$a ",    0xa    }, { "$f ",    0xf    },
        { "$0001 ", 0x0001 }, { "$1000 ", 0x1000 }, { "$1111 ", 0x1111 },
        { "$0008 ", 0x0008 }, { "$8000 ", 0x8000 }, { "$8888 ", 0x8888 },
        { "$0009 ", 0x0009 }, { "$9000 ", 0x9000 }, { "$9999 ", 0x9999 },
        { "$000A ", 0x000a }, { "$A000 ", 0xa000 }, { "$AAAA ", 0xaaaa },
        { "$000F ", 0x000f }, { "$F000 ", 0xf000 }, { "$FFFF ", 0xffff },
        { "$000a ", 0x000a }, { "$a000 ", 0xa000 }, { "$aaaa ", 0xaaaa },
        { "$000f ", 0x000f }, { "$f000 ", 0xf000 }, { "$ffff ", 0xffff }
    };
    vec _dec[] = {
        { "0 ",     0    }, { "1  ",   1    }, { "8 ",    8    }, { "9 ",    9    },
        { "00 ",    0    }, { "01 ",   1    }, { "99 ",   99   },
        { "000 ",   0    }, { "001 ",  1    }, { "999 ",  999  },
        { "0000 ",  0    }, { "0001 ", 1    }, { "9999 ", 9999 },
        { "00000 ", 0    }, { "00001 ",1    }, { "32767 ",32767}
    };
    vec _neg[] = {
        { "-0 ",     0    }, { "-1  ",   (U16)-1    }, { "-8 ",    (U16)-8    }, { "-9 ",    (U16)-9 },
        { "-00 ",    0    }, { "-01 ",   (U16)-1    }, { "-99 ",   (U16)-99   },
        { "-000 ",   0    }, { "-001 ",  (U16)-1    }, { "-999 ",  (U16)-999  },
        { "-0000 ",  0    }, { "-0001 ", (U16)-1    }, { "-9999 ", (U16)-9999 },
        { "-00000 ", 0    }, { "-00001 ",(U16)-1    }, { "-32767 ",(U16)-32767},
        { "-32768 ", (U16)32768 },
        { "-$0 ",    0x0   }, { "-$1 ",    0xffff   }, { "-$9 ",    0xfff7 },
        { "-$A ",    0xfff6}, { "-$F ",    0xfff1   },
        { "-$7fff",  0x8001}, { "-$8001",  0x7fff   }, { "-$FFFF",  0x1    }
    };
    vec _cnr[] = {    // corner cases (fix later)
        { "$-1",  0 }, { "$G", 16 }, { "$12/", 0x12 }, { "123/", 123 }
    };
    grp tlst[] = {
        { "hex", _hex, sizeof(_hex)/sizeof(vec) },
        { "dec", _dec, sizeof(_dec)/sizeof(vec) },
        { "neg", _neg, sizeof(_neg)/sizeof(vec) },
        { "cnr", _cnr, sizeof(_cnr)/sizeof(vec) }
    };
    S16 num;
    for (U16 i=0; i<sizeof(tlst)/sizeof(grp); i++) {
        SECTION(tlst[i].gname) {
            for (U16 j=0; j<tlst[i].n; j++) {
                vec v = tlst[i].t[j];
                SECTION(v.str) {
                    U8 n = n4.number((U8*)v.str, &num);
                    REQUIRE(n==1);
                    REQUIRE(v.rst==(U16)num);
                }
            }
        }
    }
    const char *_err[] = {  // error cases should return 0
        "123:"
    };
    SECTION("err") {
        for (U16 j=0; j<sizeof(_err)/sizeof(const char *); j++) {
            SECTION(_err[j]) {
                U8 n = n4.number((U8*)_err[j], &num);
                REQUIRE(n==0);
            }
        }
    }
}

TEST_CASE("U8 find(U8 *tkn, const char *lst, U16 id)")
{
    const char _long_list[] = "\x31"                            \
    "DRP" "DUP" "SWP" "OVR" "ROT" "+  " "-  " "*  " "/  " "MOD" \
	"NEG" "AND" "OR " "XOR" "NOT" "=  " "<  " ">  " "<= " ">= " \
	"<> " "@  " "!  " "C@ " "C! " "KEY" "EMT" "CR " ".  " ".\" "\
    ">R " "R> " "WRD" "HRE" "CEL" "ALO" "SAV" "LD " "TRC" "CLK" \
    "D+ " "D- " "DNG" "DLY" "PIN" "IN " "OUT" "AIN" "PWM";
    const char _short_list[] = "\x1" "DUP";
    const char _empty_list[] = "\x0" "DUP";
    vec tkn[] = {
        { "DRP", 0  },
        { "DUP", 1  },
        { "PWM", 48 },
    };
    SECTION("found long") {
        for (U16 i=0; i<sizeof(tkn)/sizeof(vec); i++) {
            SECTION(tkn[i].str) {
                U16 id;
                U8 n = n4.find((U8*)tkn[i].str, _long_list, &id);
                REQUIRE(1==n);
                REQUIRE(tkn[i].rst==id);
            }
        }
    }
    SECTION("found short") {
        U16 id;
        U8 n = n4.find((U8*)"DUP", _short_list, &id);
        REQUIRE(1==n);
        REQUIRE(0==id);
    }
    SECTION("not found") {
        U16 id;
        U8 n = n4.find((U8*)"XXX", _long_list, &id);
        REQUIRE(0==n);
    }
    SECTION("empty list") {
        U16 id;
        U8 n = n4.find((U8*)"DUP", _empty_list, &id);
        REQUIRE(0==n);
    }
    SECTION("short token") {
        U16 id;
        U8 n = n4.find((U8*)"X", _short_list, &id);
        REQUIRE(0==n);
    }
}
