#ifndef SC_LANG_STATE_H
#define SC_LANG_STATE_H

#include "SC_Lang_Slot.h"
#include "SC_Lang_Object.h"
#include "SC_Lang_API_Types.h"
#include "SC_Lang_Symbol.h"

#include <VMGlobals_bare.h>

struct VMGlobals;
struct PyrGC;

namespace SC {
namespace Lang {

class PrimitiveBase;
class Sys;

class State {
	friend class PrimitiveBase;
	friend class Sys;
public:
	inline Slot receiver() { return Slot( _vm->sp - _stackc + 1 ); }

	Slot stackAt( int i ) {
		if( i >= 0 && i < _stackc - 1 )
			return Slot( _vm->sp - _stackc + 2 + i );
		else
			return Slot();
	}

	inline Slot operator[] ( int i ) { return stackAt(i); }

	const Slot result() const {
		return Slot( &_vm->result );
	}

	State & operator<< ( nil_t ) { ++_stackc; SetNil( ++_vm->sp ); return *this; }
	State & operator<< ( int val ) { ++_stackc; SetInt( ++_vm->sp, val ); return *this; }
	State & operator<< ( float val ) { ++_stackc; SetFloat( ++_vm->sp, val ); return *this; }
	State & operator<< ( double val ) { ++_stackc; SetFloat( ++_vm->sp, val ); return *this; }
	State & operator<< ( bool val ) { ++_stackc; SetBool( ++_vm->sp, val ); return *this; }
	State & operator<< ( const Symbol & val ) { ++_stackc; SetSymbol( ++_vm->sp, val.x ); return *this; }
	State & operator<< ( const ObjectBase & val ) { ++_stackc; SetObject( ++_vm->sp, val.mHdr ); return *this; }
	State & operator<< ( void * val ) { ++_stackc; SetPtr( ++_vm->sp, val ); }

	inline Slot operator * () { return Slot( _vm->sp ); }
	inline State & operator ++ () { ++_stackc; SetNil( ++_vm->sp ); return *this; }

	inline void setCanCallOS( bool b ) { _vm->canCallOS = b; }

	inline PyrGC* gc() { return _vm->gc; }

private:
	State( VMGlobals *vm, int stackSize = 0 ) : _vm(vm), _stackc(stackSize) {}
	VMGlobals *_vm;
	int _stackc;
};

}
}

#endif
