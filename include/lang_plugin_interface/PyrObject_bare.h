#ifndef SC_PYR_OBJECT_BARE_H
#define SC_PYR_OBJECT_BARE_H

#include "PyrSlot_bare.h"

/* special gc colors */
enum {
	obj_permanent		= 1,		// sent to gc->New as a flag
	obj_gcmarker		= 2		// gc treadmill marker
};

/* obj flag fields */
enum {
	obj_inaccessible	= 4,
	obj_immutable		= 16,
	obj_marked			= 128
};

/* format types : */
enum {
	obj_notindexed,
	obj_slot,
	obj_double,
	obj_float,
	obj_int32,
	obj_int16,
	obj_int8,
	obj_char,
	obj_symbol,

	NUMOBJFORMATS
};

/*
	PyrObjectHdr : object header fields
	prev, next : pointers in the GC treadmill
	classptr : pointer to the object's class
	size : number of slots or indexable elements.

	obj_format : what kind of data this object holds
	obj_sizeclass : power of two size class of the object
	obj_flags :
		immutable : set if object may not be updated.
		finalize : set if object requires finalization.
		marked : used by garbage collector debug sanity check. may be used by primitives but must be cleared before exiting primitive.
	gc_color : GC color : black, grey, white, free, permanent
	scratch1 : undefined value. may be used within primitives as a temporary scratch value.
*/

struct PyrObjectHdr {
	struct PyrObjectHdr *prev, *next;
	struct PyrClass *classptr;
	int size;

	unsigned char obj_format;
	unsigned char obj_sizeclass;
	unsigned char obj_flags;
	unsigned char gc_color;

	int scratch1;

	int SizeClass() { return obj_sizeclass; }

	void SetMark() { obj_flags |= obj_marked; }
	void ClearMark() { obj_flags &= ~obj_marked; }
	bool IsMarked() { return ((obj_flags & obj_marked) != 0); } // BG2004-10-09 : the previous version did execute some hidden code in int -> bool conversion
	bool IsPermanent() { return gc_color == obj_permanent; }
};

struct PyrObject : public PyrObjectHdr {
	PyrSlot slots[1];
};

struct PyrList : public PyrObjectHdr
{
	PyrSlot array;
};

struct PyrDoubleArray : public PyrObjectHdr
{
	double d[1];
};

struct PyrFloatArray : public PyrObjectHdr
{
	float f[1];
};

struct PyrInt32Array : public PyrObjectHdr
{
	uint32 i[1];
};

struct PyrInt16Array : public PyrObjectHdr
{
	uint16 i[1];
};

struct PyrInt8Array : public PyrObjectHdr
{
	uint8 b[1];
};

struct PyrString : public PyrObjectHdr
{
	char s[1];
};

struct PyrSymbolArray : public PyrObjectHdr
{
	PyrSymbol* symbols[1];
};

#endif
