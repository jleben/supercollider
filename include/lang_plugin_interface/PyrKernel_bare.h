#ifndef SC_PYR_KERNEL_BARE_H
#define SC_PYR_KERNEL_BARE_H

#include "PyrObject_bare.h"
#include "PyrSlot_bare.h"

struct PyrClass : public PyrObjectHdr
{
	PyrSlot name;
	PyrSlot nextclass;
	PyrSlot superclass;
	PyrSlot subclasses;
	PyrSlot methods;

	PyrSlot instVarNames;
	PyrSlot classVarNames;
	PyrSlot iprototype;		// instance prototype
	PyrSlot cprototype;		// class var prototype

	PyrSlot constNames;
	PyrSlot constValues;	// const values

	PyrSlot instanceFormat;
	PyrSlot instanceFlags;
	PyrSlot classIndex;
	PyrSlot classFlags;
	PyrSlot maxSubclassIndex; // used by isKindOf
	PyrSlot filenameSym;
	PyrSlot charPos;
	PyrSlot classVarIndex;
};

inline bool isKindOf(PyrObjectHdr *obj, struct PyrClass *testclass)
{
	int objClassIndex = slotRawInt(&obj->classptr->classIndex);
	return objClassIndex >= slotRawInt(&testclass->classIndex) && objClassIndex <= slotRawInt(&testclass->maxSubclassIndex);
}

inline bool isKindOfSlot(PyrSlot *slot, struct PyrClass *testclass)
{
	return IsObj(slot) && isKindOf(slotRawObject(slot), testclass);
}

#endif
