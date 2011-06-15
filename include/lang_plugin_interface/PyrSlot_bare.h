#ifndef SC_PYR_SLOT_BARE_H
#define SC_PYR_SLOT_BARE_H

#if ( __SIZEOF_POINTER__ == 4 ) || defined(__i386__) || defined(__ppc__) || defined(__arm__) || defined(_WIN32)
#include "PyrSlot32.h"
#elif ( __SIZEOF_POINTER__ == 8 ) || defined(__x86_64__) || defined(_WIN64)
#include "PyrSlot64.h"
#else
#error "no PyrSlot imlementation for this platform"
#endif

#endif
