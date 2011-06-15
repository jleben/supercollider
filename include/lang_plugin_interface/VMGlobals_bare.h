#ifndef SC_VM_GLOBALS_BARE_H
#define SC_VM_GLOBALS_BARE_H

#include <setjmp.h>

class RGen;

#define TAILCALLOPTIMIZE 1

struct VMGlobals {
	VMGlobals();

	// global context
	class AllocPool *allocPool;
	struct PyrProcess *process;
	class SymbolTable *symbolTable;
	class PyrGC *gc;		// garbage collector for this process
	PyrObject *classvars;
#if TAILCALLOPTIMIZE
	int tailCall; // next byte code is a tail call.
#endif
	bool canCallOS;

	// thread context
	struct PyrThread *thread;
	struct PyrMethod *method;
	struct PyrBlock *block;
	struct PyrFrame *frame;
	struct PyrMethod *primitiveMethod;
	unsigned char *ip;		// current instruction pointer
	PyrSlot *sp;	// current stack ptr
	PyrSlot *args;
	PyrSlot receiver;	// the receiver
	PyrSlot result;
	int numpop; // number of args to pop for primitive
	long primitiveIndex;
	RGen *rgen;
	jmp_buf escapeInterpreter;

	// scratch context
	long execMethod;
};

#endif
