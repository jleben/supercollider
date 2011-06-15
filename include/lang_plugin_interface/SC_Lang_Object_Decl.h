#ifndef SC_LANG_OBJECT_DECL_H
#define SC_LANG_OBJECT_DECL_H

#include <stdint.h>

struct PyrObjectHdr;
struct PyrObject;
struct PyrClass;
struct PyrDoubleArray;
struct PyrSymbolArray;
struct PyrFloatArray;
struct PyrString;
struct PyrSymbol;

namespace SC {
namespace Lang {

class Sys;
class Slot;
class Class;
class Symbol;
class State;

class ObjectBase
{
	friend class Slot;
	friend class Sys;
	friend class State;
public:
	ObjectBase() : mHdr(0) {}
	bool operator== ( const ObjectBase & other ) const;
	bool isValid() const;
	Class type() const;
	bool isKindOf( const Class & ) const;
	int size() const;
	void setSize( int );
	void grow();
	void shrink();
protected:
	ObjectBase( PyrObjectHdr *hdr ) : mHdr(hdr) {};
	PyrObjectHdr *mHdr;
};

class Class : public ObjectBase
{
	friend class Sys;
	friend class Slot;
	friend class ObjectBase;
public:
	Class() {}
	Class( const Symbol & );
	Symbol name() const;
private:
	Class( PyrClass *cls );
	PyrClass *ptr() const;
};

class Object : public ObjectBase
{
	friend class Slot;
	friend class Sys;
public:
	Object() {}
	Slot slotAt ( int index );
	const Slot operator[] ( int index ) const;
	Slot operator[] ( int index );
private:
	Object( PyrObject *obj );
	PyrObject *ptr() const;
};

class String : public ObjectBase
{
	friend class Slot;
	friend class Sys;
public:
	String() {}
	const char *c_str() const;
private:
	String( PyrString * );
};

class SymbolArray : public ObjectBase
{
	friend class Slot;
	friend class Sys;
public:
	SymbolArray() {}
	const Symbol operator[] ( int index ) const;
	Symbol operator[] ( int index );
private:
	SymbolArray( PyrSymbolArray * );
};

template<typename T> class Array : public ObjectBase
{
	friend class Slot;
	friend class Sys;
public:
	//static Array<T> fromSlot( Slot &s );
	Array() {}
	T & operator [] ( int i ) const;
	T operator [] ( int i );
protected:
	Array( PyrObjectHdr *hdr ) : ObjectBase( hdr ) {}
	T* data();
};

typedef Array<float> FloatArray;
typedef Array<double> DoubleArray;
typedef Array<uint32_t> Int32Array;
typedef Array<uint16_t> Int16Array;
typedef Array<uint8_t> Int8Array;

}
}

#endif
