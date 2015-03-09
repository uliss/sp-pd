
// define attributes for explicit export of symbols:


#if defined MSW										// when compiling for Windows	
#define EXPORT __declspec(dllexport)
#elif __GNUC__ >= 4										// else, when compiling with GCC 4.0 or higher
#define EXPORT __attribute__((visibility("default")))	
#endif
#ifndef EXPORT 
#define EXPORT											// empty definition for other cases
#endif

// declaration of exported function:

extern "C" EXPORT void soundtouch_tilde_setup(void);