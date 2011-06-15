#ifndef SC_LANG_PRIMITIVE_H
#define SC_LANG_PRIMITIVE_H

#if 0

#include "SC_Lang_State.h"

struct VMGlobals;
typedef int (*PrimitiveHandler)(struct VMGlobals *g, int numArgsPushed);

namespace SC {
namespace Lang {

class Sys;

typedef int (*PrimitiveFn)( State & );


class PrimitiveBase {
public:
	inline static State state( VMGlobals *vm, int stackSize ) {
		return State( vm, stackSize );
	}
	template <int primitiveFn( State & )>
	static void define( const Sys & sys, const char *name, int argc, int vargc ) {
		(*sys.mDefinePrimitive)(name, &handler<primitiveFn>, argc, vargc);
	}
private:
	template <int primitiveFn( State & )> static int handler( VMGlobals *g, int i ) {}
};


template <int primitiveFn( State & )>
class Primitive : public PrimitiveBase
{
public:
	static void define( const Sys & sys, const char * name, int argc, int vargc ) {
		PrimitiveBase::define( sys, name, &_handle, argc, vargc );
	}
private:
	static int _handle( VMGlobals *g, int i ) {
		State s( state(g,i) );
		return ( (*primitiveFn)(s) );
	}
};

}
}
#endif

#endif
