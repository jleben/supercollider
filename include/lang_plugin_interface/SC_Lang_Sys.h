#ifndef SC_LANG_SYS_IMPL_H
#define SC_LANG_SYS_IMPL_H

#include "SC_Lang_Sys_Decl.h"
#include "SC_Lang_Object.h"
#include "SC_Lang_State.h"
#include "SC_Lang_Symbol.h"

namespace SC {
namespace Lang {

template <int primitiveFn( State & )>
inline void Sys::definePrimitive( const char *name, int argc, int vargc ) const
{
	(*intf->mDefinePrimitive)(name, &primitiveHandler<primitiveFn>, argc+1, vargc);
}

template <int primitiveFn( State & )>
inline int Sys::primitiveHandler( VMGlobals *g, int i )
{
	State s( g,i );
	return (*primitiveFn)( s );
}

template <int finalizeFn( State &, Object & )>
inline void Sys::installFinalizer ( State &s, Object &o, int i ) const
{
	(*intf->mInstallFinalizer)( s._vm, o.ptr(), i, &finalizeHandler<finalizeFn> );
}

template <int finalizeFn( State &, Object & )>
inline int Sys::finalizeHandler( VMGlobals *g, PyrObject *o )
{
	State state(g);
	Object obj(o);
	return (*finalizeFn)( state, obj );
}

inline Symbol Sys::getSymbol ( const char *name ) const {
	return Symbol( (*intf->mGetSymbol)(name) );
}

inline Symbol Sys::getMetaSymbol ( const char *name ) const {
	char str[256];
	strcpy(str, "Meta_");
	strncat(str, name, 250); // (buf size) - (strlen("Meta_")) - (1 for '\0')
	return Symbol( (*intf->mGetSymbol)(str) );
}

inline Symbol Sys::findSymbol ( const char *name ) const {
	return Symbol( (*intf->mFindSymbol)(name) );
}
inline Class Sys::getClass ( const Symbol &sym ) const {
	return Class( sym.x->u.classobj );
}

inline State Sys::globalState() const { return State( intf->mMainVMGlobals, 0 ); }

/// interpreter, lexer

inline void Sys::lock() const { (*intf->mLock)(); }
inline bool Sys::tryLock() const { return (*intf->mTryLock)(); }
inline void Sys::unlock() const { (*intf->mUnlock)(); }

inline void Sys::interpretCmdLine (const char *textbuf, int textlen, char *methodname) const {
	(*intf->mInterpretCmdLine)(textbuf, textlen, methodname);
}
inline void Sys::runInterpreter ( const State & state, const Symbol & selector, int argc) const {
	(*intf->mRunInterpreter)(state._vm, selector.x, argc);
}

/// GC

inline Object Sys::newObject ( State &state, size_t nbytes, int flags, int format, bool collect ) const
{
	PyrObject *obj = (*intf->mNewPyrObject)( state._vm->gc, nbytes, flags, format, collect );
	return Object(obj);
}

inline String Sys::newString ( State &state, const char *s, int flags, bool collect ) const
{
	PyrString *str = (*intf->mNewPyrString)( state._vm->gc, s, flags, collect );
	return String(str);
}

inline String Sys::newStringN ( State &state, int size, int flags, bool collect ) const
{
	PyrString *str = (*intf->mNewPyrStringN)( state._vm->gc, size, flags, collect );
	return String(str);
}

inline Object Sys::newArray ( State &state, int size, int flags, bool collect ) const
{
	PyrObject *array = (*intf->mNewPyrArray)( state._vm->gc, size, flags, collect);
	return Object(array);
}

inline SymbolArray Sys::newSymbolArray ( State &state, int size, int flags, bool collect ) const
{
	PyrSymbolArray *a = (*intf->mNewPyrSymbolArray)( state._vm->gc, size, flags, collect );
	return SymbolArray(a);
}

inline Int8Array Sys::newInt8Array ( State &state, int size, int flags, bool collect ) const
{
	PyrInt8Array *a = (*intf->mNewPyrInt8Array)( state._vm->gc, size, flags, collect );
	return Int8Array(a);
}

inline Int32Array Sys::newInt32Array ( State &state, int size, int flags, bool collect ) const
{
	PyrInt32Array *a = (*intf->mNewPyrInt32Array)( state._vm->gc, size, flags, collect );
	return Int32Array(a);
}

inline DoubleArray Sys::newDoubleArray ( State &state, int size, int flags, bool collect ) const
{
	PyrDoubleArray *a = (*intf->mNewPyrDoubleArray)( state._vm->gc, size, flags, collect );
	return DoubleArray(a);
}

inline void Sys::GCWrite ( State &state, ObjectBase &parent, Slot & slot ) const
{
	(*intf->mGCWrite)( state._vm->gc, parent.mHdr, slot.x );
}

/// miscellaneous

inline int Sys::getBuffer (uint32 index, SndBuf *buf, bool& didChange) const {
	return (*intf->mGetBuffer)(index, buf, didChange);
}

/// main plugins

inline void Sys::tick () const { (*intf->mTick)(); }
inline void Sys::readCmdLine () const { (*intf->mReadCmdLine)(); }
inline bool Sys::shouldBeRunning () const { return (*intf->mShouldBeRunning)(); }

} // namespace SC
} // namespace Lang

#endif // SC_LANG_API_SYSTEM_H
