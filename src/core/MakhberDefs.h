
#if defined(_WIN32) && defined(MAKHBER_DLL)
#    ifdef MAKHBER_DLL_EXPORT
#        define MAKHBER_EXPORT __declspec(dllexport)
#    else
#        define MAKHBER_EXPORT __declspec(dllimport)
#    endif
#else
#    define MAKHBER_EXPORT
#endif

#ifdef _MSC_VER
#    define NOMINMAX
#endif

#ifndef TS_PATH
#    define TS_PATH "/translations"
#endif
