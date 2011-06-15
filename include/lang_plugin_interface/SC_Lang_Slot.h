#ifndef SC_LANG_SLOT_IMPL_H
#define SC_LANG_SLOT_IMPL_H

#include "SC_Lang_Slot_Decl.h"
#include "SC_Lang_Object.h" // SC::Lang::Class
#include "SC_Lang_Symbol.h"

#include <PyrKernel_bare.h>

#include <cassert>

namespace SC {
namespace Lang {

inline bool Slot::isValid() const { return x != 0; }

inline bool Slot:: isNil() const { assert(isValid()); return IsNil(x); }
inline bool Slot:: isTrue() const { assert(isValid()); return IsTrue(x); }
inline bool Slot:: isFalse() const { assert(isValid()); return IsFalse(x); }
inline bool Slot:: isInt() const { assert(isValid()); return IsInt(x); }
inline bool Slot:: isFloat() const { assert(isValid()); return IsFloat(x); }
inline bool Slot:: isChar() const { assert(isValid()); return IsChar(x); }
inline bool Slot:: isSymbol() const { assert(isValid()); return IsSym(x); }
inline bool Slot:: isObject() const { assert(isValid()); return IsObj(x); }
inline bool Slot:: isPointer() const { assert(isValid()); return IsPtr(x); }

inline bool Slot::is( SlotType t ) const
{
	assert(isValid());
	return ( t == SlotFloat ? IsFloat(x) : GetTag(x) == t );
}

inline int Slot::type() const
{
	assert(isValid());
	return ( IsFloat(x) ? SlotFloat : GetTag(x) );
}

inline bool Slot::isKindOf( const Class &c ) const {
	assert( isValid() && c.isValid() );
	return isKindOfSlot( x, c.ptr() );
}

inline bool Slot::isKindOf( const Symbol &sym ) const {
	assert( isValid() && sym.isValid() );
	return isKindOfSlot( x, sym.x->u.classobj );
}

inline int Slot::asInt() const
{
	assert(isValid());
	return slotRawInt(x);
}
inline float Slot::asFloat() const
{
	assert(isValid());
	return slotRawFloat(x);
}
inline Symbol Slot::asSymbol() const
{
	assert(isValid());
	return Symbol( slotRawSymbol(x) );
}
inline Class Slot::asClass() const
{
	assert(isValid());
	return Class( slotRawClass(x) );
}
inline Object Slot::asObject() const
{
	assert(isValid());
	return Object( slotRawObject(x) );
}
inline String Slot::asString() const
{
	assert(isValid());
	return String( slotRawString(x) );
}
inline SymbolArray Slot::asSymbolArray() const
{
	assert(isValid());
	return SymbolArray( slotRawSymbolArray(x) );
}
inline FloatArray Slot::asFloatArray() const
{
	assert(isValid());
	return FloatArray( reinterpret_cast<PyrFloatArray*>(slotRawObject(x)) );
}
inline DoubleArray Slot::asDoubleArray() const
{
	assert(isValid());
	return DoubleArray( slotRawDoubleArray(x) );
}
inline Int32Array Slot::asInt32Array() const
{
	assert(isValid());
	return Int32Array( reinterpret_cast<PyrInt32Array*>(slotRawObject(x)) );
}
inline Int16Array Slot::asInt16Array() const
{
	assert(isValid());
	return Int16Array( reinterpret_cast<PyrInt16Array*>(slotRawObject(x)) );
}
inline Int8Array Slot::asInt8Array() const
{
	assert(isValid());
	return Int8Array( slotRawInt8Array(x) );
}
inline void *Slot::asRawPointer() const
{
	assert(isValid());
	return slotRawPtr(x);
}

inline void Slot::setOk( int err, bool *ok )  const
{
	if( ok ) *ok = ( err == errNone );
}

inline int Slot::toInt ( bool *ok ) const
{
	assert(isValid());
	int i = 0;
	setOk( slotIntVal( x, &i ), ok );
	return i;
}

inline float Slot::toFloat ( bool *ok ) const
{
	assert(isValid());
	float f = 0.f;
	setOk( slotFloatVal( x, &f ), ok );
	return f;
}

inline double Slot::toDouble ( bool *ok ) const
{
	assert(isValid());
	double d = 0.0;
	setOk( slotDoubleVal( x, &d ), ok );
	return d;
}

inline bool Slot::toBool ( bool *ok ) const
{
	assert(isValid());
	bool b = IsTrue(x);
	if( ok ) *ok = b || IsFalse(x);
	return b;
}

inline Symbol Slot::toSymbol () const
{
	assert(isValid());
	PyrSymbol *psym = 0;
	slotSymbolVal( x, &psym );
	return Symbol(psym);
}
/*
TODO: How to effectively check correct type?
inline Class Slot::toClass () const
{
	assert(isValid());
	if( IsObj(x) )
		return Class( slotRawClass(x) );
	else
		return Class();
}
*/
inline Object Slot::toObject () const
{
	assert(isValid());
	if( IsObj(x) && slotRawObject(x)->obj_format <= obj_slot )
		return Object( slotRawObject(x) );
	else
		return Object();
}
inline String Slot::toString() const
{
	if( IsObj(x) && slotRawObject(x)->obj_format == obj_char )
		return String( slotRawString(x) );
	else
		return String();
}
inline SymbolArray Slot::toSymbolArray() const
{
	assert(isValid());
	if( IsObj(x) && slotRawObject(x)->obj_format == obj_symbol )
		return SymbolArray( slotRawSymbolArray(x) );
	else
		return SymbolArray();
}
inline FloatArray Slot::toFloatArray() const
{
	assert(isValid());
	if( IsObj(x) && slotRawObject(x)->obj_format == obj_float )
		return FloatArray( reinterpret_cast<PyrFloatArray*>(slotRawObject(x)) );
	else
		return FloatArray();
}
inline DoubleArray Slot::toDoubleArray() const
{
	assert(isValid());
	if( IsObj(x) && slotRawObject(x)->obj_format == obj_double )
		return DoubleArray( slotRawDoubleArray(x) );
	else
		return DoubleArray();
}
inline Int32Array Slot::toInt32Array() const
{
	assert(isValid());
	if( IsObj(x) && slotRawObject(x)->obj_format == obj_int32 )
		return Int32Array( reinterpret_cast<PyrInt32Array*>(slotRawObject(x)) );
	else
		return Int32Array();
}
inline Int16Array Slot::toInt16Array() const
{
	assert(isValid());
	if( IsObj(x) && slotRawObject(x)->obj_format == obj_int16 )
		return Int16Array( reinterpret_cast<PyrInt16Array*>(slotRawObject(x)) );
	else
		return Int16Array();
}
inline Int8Array Slot::toInt8Array() const
{
	assert(isValid());
	if( IsObj(x) && slotRawObject(x)->obj_format == obj_int8 )
		return Int8Array( slotRawInt8Array(x) );
	else
		return Int8Array();
}
inline void * Slot::toRawPointer() const
{
	assert(isValid());
	if( IsPtr(x) ) return slotRawPtr(x);
	else return 0;
}


inline bool Slot::operator== ( bool b ) const
{
	assert(isValid());
	return ( b ? IsTrue(x) : IsFalse(x) );
}

inline void Slot::setNil() { SetNil (x); }
inline void Slot::setInt( int val ) { SetInt (x, val); }
inline void Slot::setFloat( float val ) { SetFloat (x, val); }
inline void Slot::setDouble( double val ) { SetFloat (x, val); }
inline void Slot::setChar( char val ) { SetChar (x, val); }
inline void Slot::setBool( bool val ) { SetBool (x, val); }
inline void Slot::setSymbol( const Symbol & val ) { SetSymbol (x, val.x); }
inline void Slot::setObject( const ObjectBase & val ) { SetObject (x, val.mHdr); }
inline void Slot::setRawPointer( void * val ) { SetPtr (x, val); }

inline Slot & Slot::operator= ( int val ) { setInt (val); return *this; }
inline Slot & Slot::operator= ( float val ) { setFloat (val); return *this; }
inline Slot & Slot::operator= ( double val ) { setDouble (val); return *this; }
inline Slot & Slot::operator= ( char val ) { setChar (val); return *this; }
inline Slot & Slot::operator= ( bool val ) { setBool (val); return *this; }
inline Slot & Slot::operator= ( const Symbol & val ) { setSymbol (val); return *this; }
inline Slot & Slot::operator= ( const ObjectBase & val ) { setObject (val); return *this; }
inline Slot & Slot::operator= ( void * val ) { setRawPointer (val); return *this; }

inline void Slot::copy( const Slot & other ) {
	assert( isValid() && other.isValid() );
	slotCopy( x, other.x );
}

} // namespace SC
} // namespace Lang

#endif
