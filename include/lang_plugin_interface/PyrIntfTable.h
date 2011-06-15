#ifndef SC_PYR_INTF_TABLE_H
#define SC_PYR_INTF_TABLE_H

struct PyrSymbol;
struct PyrObjectHdr;
struct PyrString;
struct PyrObject;
struct PyrSymbolArray;
struct PyrInt8Array;
struct PyrInt32Array;
struct PyrDoubleArray;

struct VMGlobals;
class PyrGC;
struct SndBuf;

#include <PyrSlot_bare.h>

typedef int (*ObjFuncPtr)(struct VMGlobals*, struct PyrObject*);
typedef int (*PrimitiveHandler)(struct VMGlobals *g, int numArgsPushed);

struct PyrInterfaceTable {
	void (*mLock) ();
	bool (*mTryLock) ();
	void (*mUnlock) ();

	void (*mDefinePrimitive) (const char *name, PrimitiveHandler handler,
		int numArgs, int varArgs);

	PyrSymbol * (*mGetSymbol) (const char *);
	PyrSymbol * (*mFindSymbol) (const char *);

	void (*mInterpretCmdLine) (const char *textbuf, int textlen, char *methodname);
	void (*mRunInterpreter) (VMGlobals *g, PyrSymbol *selector, int argc);
	void (*mInstallFinalizer) (VMGlobals *g, PyrObject *inObj, int slotIndex, ObjFuncPtr inFunc);

	PyrObject * (*mNewPyrObject) (PyrGC *gc, size_t nbytes, int flags, int format, bool collect);
	PyrString * (*mNewPyrString) (PyrGC *gc, const char *s, int flags, bool collect);
	PyrString* (*mNewPyrStringN) (PyrGC *gc, int size, int flags, bool collect);
	PyrObject * (*mNewPyrArray) (PyrGC *gc, int size, int flags, bool collect);
	PyrSymbolArray* (*mNewPyrSymbolArray) (PyrGC *gc, int size, int flags, bool collect);
	PyrInt8Array* (*mNewPyrInt8Array) (PyrGC *gc, int size, int flags, bool collect);
	PyrInt32Array* (*mNewPyrInt32Array) (PyrGC *gc, int size, int flags, bool collect);
	PyrDoubleArray* (*mNewPyrDoubleArray) (PyrGC *gc, int size, int flags, bool collect);

	void (*mGCWrite) (PyrGC *gc, PyrObjectHdr* inParent, PyrSlot* inSlot);

	int (*mGetBuffer) (uint32 index, SndBuf *buf, bool& didChange);

	void (*mTick)();
	void (*mReadCmdLine)();
	bool (*mShouldBeRunning)();

	VMGlobals *mMainVMGlobals;
};

namespace SC {
namespace Lang {

extern void initInterfaceTable( PyrInterfaceTable & );

}
}

#endif // !defined SC_PYR_INTF_TABLE_H
