#ifdef  BE_LIBRARY_EXPORTS 
#define BE_API __declspec(dllexport)
#elif BE_LIBRARY_IMPORT
#define BE_API __declspec(dllimport)
#else
#define BE_API 
#endif