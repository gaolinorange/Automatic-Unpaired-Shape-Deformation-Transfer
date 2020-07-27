/*===========================================================================*\
 *                                                                           *
 *                               IGM                                         *
 *      Copyright (C) 2017 by Computer Graphics Group, RWTH Aachen           *
 *                           www.rwth-graphics.de                            *
 *                                                                           *
\*===========================================================================*/

// Since noexcept is not supported by all compiler we define the NOEXCEPT macro here
// which does nothing if noexcept is not supported and passes its argument to noexcept
// if it is supported.

#if defined(_MSC_VER) && (_MSC_VER <= 1800)
// noexcept not supported
#define NOEXCEPT(ON)
#else
// noexcept supported
#define NOEXCEPT(ON) noexcept(ON)
#endif
