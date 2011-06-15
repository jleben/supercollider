#ifndef SC_LANG_OBJECT_IMPL_H
#define SC_LANG_OBJECT_IMPL_H

#include "SC_Lang_Object_Decl.h"
#include "SC_Lang_Slot.h"
#include "SC_Lang_Symbol.h"

#include <PyrObject_bare.h>
#include <PyrKernel_bare.h>

#define CHECK_INDEX( i ) \
	assert( isValid() ); \
	assert( i >= 0 && i < mHdr->size );

namespace SC {
namespace Lang {

inline bool ObjectBase::operator== ( const ObjectBase & other ) const
{
	return mHdr == other.mHdr;
}
inline bool ObjectBase::isValid() const { return mHdr != 0; }
inline Class ObjectBase::type() const { return Class( mHdr->classptr ); }
inline bool ObjectBase::isKindOf( const Class &c ) const {
	return ::isKindOf( mHdr, c.ptr() );
}
inline int ObjectBase::size() const { return mHdr->size; }
inline void ObjectBase::setSize( int size )
{
	assert( isValid() );
	assert( mHdr->obj_format != obj_notindexed );
	mHdr->size = size;
}
inline void ObjectBase::grow()
{
	assert( isValid() );
	assert( mHdr->obj_format != obj_notindexed );
	mHdr->size++;
}
inline void ObjectBase::shrink()
{
	assert( isValid() );
	assert( mHdr->obj_format != obj_notindexed );
	assert( mHdr->size > 0 );
	mHdr->size--;
}

//-------------------------------------------

inline Class::Class( const Symbol &sym ) : ObjectBase( sym.x->u.classobj ) {}
inline PyrClass *Class::ptr() const { return static_cast<PyrClass*>(mHdr); }
inline Class::Class( PyrClass *cls ) : ObjectBase( cls ) {};
inline Symbol Class::name() const
{
	assert(isValid());
	return Symbol( slotRawSymbol(&ptr()->name) );
}

//-------------------------------------------

inline Object::Object( PyrObject *obj ) : ObjectBase( obj ) {};
inline PyrObject *Object::ptr() const { return static_cast<PyrObject*>(mHdr); }
inline Slot Object::slotAt( int i )
{
	CHECK_INDEX(i);
	return Slot( &static_cast<PyrObject*>(mHdr)->slots[i] );
}
inline const Slot Object::operator[] ( int i ) const
{
	CHECK_INDEX(i);
	return Slot( &static_cast<PyrObject*>(mHdr)->slots[i] );
}
inline Slot Object::operator[] ( int i ) { return slotAt(i); }

//-------------------------------------------

inline String::String( PyrString *str ) : ObjectBase( str ) {};
inline const char *String::c_str() const { return static_cast<PyrString*>(mHdr)->s; }

//-------------------------------------------

inline SymbolArray::SymbolArray( PyrSymbolArray *symArray ) : ObjectBase( symArray ) {};

inline const Symbol SymbolArray::operator[] ( int i ) const {
	CHECK_INDEX(i);
	return Symbol( static_cast<PyrSymbolArray*>(mHdr)->symbols[i] );
}
inline Symbol SymbolArray::operator[] ( int i ) {
	CHECK_INDEX(i);
	return Symbol( static_cast<PyrSymbolArray*>(mHdr)->symbols[i] );
}

//-------------------------------------------


template<typename T> inline T & Array<T>::operator [] ( int i ) const {
	CHECK_INDEX(i);
	return data()[i];
}

template<typename T> inline T Array<T>::operator [] ( int i ) {
	CHECK_INDEX(i);
	return data()[i];
}

template<> inline float * Array<float>::data()
{
	return static_cast<PyrFloatArray*>(mHdr)->f;
}

template<> inline double * Array<double>::data()
{
	return static_cast<PyrDoubleArray*>(mHdr)->d;
}

template<> inline uint32_t * Array<uint32_t>::data()
{
	return static_cast<PyrInt32Array*>(mHdr)->i;
}

template<> inline uint16_t * Array<uint16_t>::data()
{
	return static_cast<PyrInt16Array*>(mHdr)->i;
}

template<> inline uint8_t * Array<uint8_t>::data()
{
	return static_cast<PyrInt8Array*>(mHdr)->b;
}

} // namespace Lang
} // namespace SC

#endif
