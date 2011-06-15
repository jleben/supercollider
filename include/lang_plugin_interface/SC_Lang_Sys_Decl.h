#ifndef SC_LANG_SYS_DECL_H
#define SC_LANG_SYS_DECL_H

#include "PyrIntfTable.h"
#include "PyrSlot_bare.h"
#include "SC_Lang_Object_Decl.h"

#include <pthread.h>
#include <cstring>

namespace SC {
namespace Lang {

class Symbol;
class Slot;
class ObjectBase;
class Class;
class Object;
class String;
class Sys;
class PrimitiveBase;
class State;


class Sys
{
public:

	Sys ( PyrInterfaceTable *table ) : intf( table ) {}

	/// primitives
	template <int primitiveFn( State & )>
	void definePrimitive( const char *name, int argc, int vargc ) const;

	/// global data

	Symbol findSymbol ( const char * ) const;
	Symbol getSymbol ( const char * ) const;
	Symbol getMetaSymbol ( const char *name ) const;
	Class getClass ( const Symbol & ) const;
	State globalState() const;

	/// interpreter, lexer

	void lock() const;
	bool tryLock() const;
	void unlock() const;

	void interpretCmdLine (const char *textbuf, int textlen, char *methodname) const;
	void runInterpreter (const State & state, const Symbol & selector, int argc) const;

	/// GC
	template <int finalizeFn( State &, Object & )>
	void installFinalizer ( State &, Object &, int slotIndex ) const;

	Object newObject ( State &, size_t nbytes, int flags, int format, bool collect ) const;
	String newString ( State &, const char *s, int flags, bool collect ) const;
	String newStringN ( State &, int size, int flags, bool collect ) const;
	Object newArray ( State &, int size, int flags, bool collect ) const;
	SymbolArray newSymbolArray ( State &, int size, int flags, bool collect ) const;
	Int8Array newInt8Array ( State &, int size, int flags, bool collect ) const;
	Int32Array newInt32Array ( State &, int size, int flags, bool collect ) const;
	DoubleArray newDoubleArray ( State &, int size, int flags, bool collect ) const;

	void GCWrite ( State &, ObjectBase & parent, Slot & slot ) const;

	/// miscellaneous

	int getBuffer (uint32 index, SndBuf *buf, bool& didChange) const;

	/// main plugins

	void tick () const;
	void readCmdLine () const;
	bool shouldBeRunning () const;

private:
	template <int primitiveFn( State & )>
	static int primitiveHandler( VMGlobals *g, int i );

	template <int finalizeFn( State &, Object & )>
	static int finalizeHandler( VMGlobals*, PyrObject* );

	PyrInterfaceTable *intf;
};

}
}

#endif
