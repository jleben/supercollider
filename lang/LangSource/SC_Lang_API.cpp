#include <PyrIntfTable.h>

#include <GC.h>
#include <SCBase.h>
#include <VMGlobals.h>
#include <PyrObject.h>
#include <PyrKernel.h>
#include <PyrLexer.h>

extern pthread_mutex_t gLangMutex;
extern void pluginDefinePrimitive(const char *name, PrimitiveHandler handler, int, int);
extern int getScopeBuf (uint32, SndBuf *, bool &);

static void GCWrite (PyrGC *gc, PyrObjectHdr* inParent, PyrSlot* inSlot) {
	gc->GCWrite (inParent, inSlot);
}

static int failGetBuffer (uint32, SndBuf *, bool &changed) { changed = false; return errFailed; }

static void lockLang() { pthread_mutex_lock( &gLangMutex ); }
static bool tryLockLang() { return pthread_mutex_trylock ( &gLangMutex ); }
static void unlockLang() { pthread_mutex_unlock ( &gLangMutex ); }

void SC::Lang::initInterfaceTable( PyrInterfaceTable & intf )
{
	intf.mLock = &lockLang;
	intf.mTryLock = &tryLockLang;
	intf.mUnlock = &unlockLang;
	intf.mGetSymbol = &getsym;
	intf.mFindSymbol = &findsym;
	intf.mInterpretCmdLine = &interpretCmdLine;
	intf.mRunInterpreter = &runInterpreter;
	intf.mInstallFinalizer = &InstallFinalizer;
	intf.mDefinePrimitive = &pluginDefinePrimitive;
	intf.mNewPyrObject = &newPyrObject;
	intf.mNewPyrString = &newPyrString;
	intf.mNewPyrStringN = &newPyrStringN;
	intf.mNewPyrArray = &newPyrArray;
	intf.mNewPyrSymbolArray = &newPyrSymbolArray;
	intf.mNewPyrInt8Array = &newPyrInt8Array;
	intf.mNewPyrInt32Array = &newPyrInt32Array;
	intf.mNewPyrDoubleArray = &newPyrDoubleArray;
	intf.mGCWrite = &GCWrite;
#ifndef NO_INTERNAL_SERVER
	intf.mGetBuffer = &getScopeBuf;
#else
	intf.mGetBuffer = &failGetBuffer;
#endif
	intf.mMainVMGlobals = gMainVMGlobals;
}
