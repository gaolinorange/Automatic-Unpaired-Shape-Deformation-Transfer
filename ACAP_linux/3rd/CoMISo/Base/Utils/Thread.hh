// (C) Copyright 2017 by Autodesk, Inc.

#ifndef BASE_THREAD_HH_INCLUDED
#define BASE_THREAD_HH_INCLUDED

// System specific thread support

#if defined(__apple_build_version__)
// work around the lack of thread_local support in XCode 7
#if !__has_feature(cxx_thread_local)
#define BASE_THREAD_LOCAL_BROKEN 
#endif//!__has_feature(cxx_thread_local)
#endif//defined(__apple_build_version__)

// TODO: move these macros to a more suitable location
#define GCC_VERSION(MAJOR, MINOR, PATCH) \
  (MAJOR * 10000 +  MINOR * 100 + PATCH)
#define GCC_VERSION_CURRENT\
  GCC_VERSION(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)

#if defined(__GNUC__) && GCC_VERSION_CURRENT < GCC_VERSION(4, 8, 3)
// there is a thread_local linker bug prior to gcc-4.8.3
// see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55800
#define BASE_THREAD_LOCAL_BROKEN 
#endif//__GNUC__

// disable completely this and use static non-thread storage instead due to
// bugs in thread_local variable initialization in worker threads
#if 1 //def BASE_THREAD_LOCAL_BROKEN // if broken do not use thread_local
#define BASE_THREAD_LOCAL
#else// BASE_THREAD_LOCAL_BROKEN
#define BASE_THREAD_LOCAL thread_local
#endif// BASE_THREAD_LOCAL_BROKEN

#endif//BASE_THREAD_HH_INCLUDED
