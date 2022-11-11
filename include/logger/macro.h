#pragma once

#define SRV_STRINGIZE(text) SRV_STRINGIZE_I(text)
#define SRV_STRINGIZE_I(...) #__VA_ARGS__

// Workaround for varying preprocessing behavior between MSVC and gcc
#define SRV_EXPAND_MACRO(x) x

// suppress warning "conditional expression is constant" in the while(0) for
// visual c++ http://cnicholson.net/2009/03/stupid-c-tricks-dowhile0-and-c4127/
#define SRV_MULTILINE_MACRO_BEGIN \
    do                            \
    {
#ifdef _MSC_VER
#define SRV_MULTILINE_MACRO_END                               \
    __pragma(warning(push)) __pragma(warning(disable : 4127)) \
    }                                                         \
    while (0)                                                 \
    __pragma(warning(pop))
#else
#define SRV_MULTILINE_MACRO_END \
    }                           \
    while (0)
#endif

// Math
#define SRV_C_MAX(a, b)         \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })

#define SRV_C_MIN(a, b)         \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
