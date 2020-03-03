#ifdef _WIN32
    #ifdef BE_LIBRARY_EXPORTS
        #define BE_API __declspec(dllexport)
    #elif BE_LIBRARY_IMPORT
        #define BE_API __declspec(dllimport)
    #else
        #define BE_API 
    #endif
#elif defined(__linux__)

    #ifdef BE_LIBRARY_EXPORTS
        #define BE_API __attribute__((__visibility__("default")))
    #elif BE_LIBRARY_IMPORT
        #define BE_API
    #else
        #define BE_API 
    #endif

#endif