#ifndef SC_LANG_SLOT_DECL_H
#define SC_LANG_SLOT_DECL_H

#include "SC_Lang_Object_Decl.h"
#include "PyrSlot_bare.h"

namespace SC {
namespace Lang {

class Symbol;
class Sys;
class State;

enum SlotType {
	SlotNil = tagNil,
	SlotTrue = tagTrue,
	SlotFalse = tagFalse,
	SlotInt = tagInt,
	SlotChar = tagChar,
	SlotSymbol = tagSym,
	SlotObject = tagObj,
	SlotPointer = tagPtr,
	SlotFloat
	// NOTE tagFloat does not exist on 32bit arch.
};

class Slot
{
	friend class Symbol;
	friend class ObjectBase;
	friend class Class;
	friend class Object;
	friend class Sys;
	friend class State;

public:
	Slot() : x(0) {};
	bool isValid() const;

	bool isNil() const;
	bool isTrue() const;
	bool isFalse() const;
	bool isInt() const;
	bool isFloat() const;
	bool isChar() const;
	bool isSymbol() const;
	bool isObject() const;
	bool isPointer() const;

	bool is( SlotType ) const;
	int type() const;

	bool isKindOf( const Class & ) const;
	bool isKindOf( const Symbol & ) const;

	int asInt() const;
	float asFloat() const;
	Symbol asSymbol() const;
	Class asClass() const;
	Object asObject() const;
	String asString() const;
	SymbolArray asSymbolArray() const;
	FloatArray asFloatArray() const;
	DoubleArray asDoubleArray() const;
	Int32Array asInt32Array() const;
	Int16Array asInt16Array() const;
	Int8Array asInt8Array() const;
	void *asRawPointer() const;

	int toInt ( bool *ok = 0 ) const;
	float toFloat ( bool *ok = 0 ) const;
	double toDouble ( bool *ok = 0 ) const;
	bool toBool ( bool *ok = 0 ) const;
	Symbol toSymbol () const;
	//TODO: Class toClass() const;
	Object toObject () const;
	String toString() const;
	SymbolArray toSymbolArray() const;
	FloatArray toFloatArray() const;
	DoubleArray toDoubleArray() const;
	Int32Array toInt32Array() const;
	Int16Array toInt16Array() const;
	Int8Array toInt8Array() const;
	void *toRawPointer() const;

	bool operator== ( bool ) const;

	void setNil();
	void setInt( int );
	void setFloat( float );
	void setDouble( double );
	void setChar( char );
	void setBool( bool );
	void setSymbol( const Symbol & );
	void setObject( const ObjectBase & );
	void setRawPointer( void * );

	Slot & operator= ( int );
	Slot & operator= ( float );
	Slot & operator= ( double );
	Slot & operator= ( char );
	Slot & operator= ( bool );
	Slot & operator= ( const Symbol & );
	Slot & operator= ( const ObjectBase & );
	Slot & operator= ( void * );

	//template<typename T> T value() { return T(); }

	void copy( const Slot & other );

private:
	Slot( PyrSlot *slot ) : x(slot) {};
	void setOk( int err, bool *ok ) const;
	PyrSlot *x;
};

}
}

#endif
