
#ifndef BASEDLLEXPORT
	#if defined(WIN32) || defined(_WIN32)
    #ifdef BASEDLL
      #ifdef USEBASE
        #define BASEDLLEXPORT __declspec(dllimport)
        #define BASEDLLEXPORTONLY
			#else
        #define BASEDLLEXPORT __declspec(dllexport)
        #define BASEDLLEXPORTONLY __declspec(dllexport)
			#endif
    #else
      #define BASEDLLEXPORT
      #define BASEDLLEXPORTONLY
		#endif
	#else // non-Windows symbols export:
    #ifdef BASEDLL
      // only export symbols if BASEDLL is defined (same behavior as Windows)
      #define BASEDLLEXPORT __attribute__((visibility("default")))
      #define BASEDLLEXPORTONLY BASEDLLEXPORT
    #else // BASEDLL not defined
      // this allows building Base as static library without exposing its symbols
      #define BASEDLLEXPORT
      #define BASEDLLEXPORTONLY
    #endif // BASEDLL
	#endif
#endif

#undef min
#undef max

#if defined(_MSC_VER) 

// Some warnings are disabled permanently since the cannot be controlled on 
// section basis.

// disable "needs-to-have-dll-interface-to-be-used-by-clients-of-class" warning
#pragma warning (disable: 4251)

#endif

// configure some defines based on the platform
#if (_MSC_VER >= 1700 || __cplusplus > 199711L || defined(__GXX_EXPERIMENTAL_CXX0X__))
#define STD_ARRAY_AVAILABLE
#endif


