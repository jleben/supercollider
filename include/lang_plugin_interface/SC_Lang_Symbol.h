#ifndef SC_LANG_SYMBOL_H
#define SC_LANG_SYMBOL_H

#include <PyrSymbol.h>
#include <cassert>

namespace SC {
namespace Lang {

class Slot;
class ObjectBase;
class Class;
class Object;
class SymbolArray;
class Sys;
class State;

class Symbol
{
	friend class Slot;
	friend class ObjectBase;
	friend class Class;
	friend class Object;
	friend class SymbolArray;
	friend class Sys;
	friend class State;
public:
	Symbol() : x(0) {};
	bool isValid() const { return x != 0; }
	const char *name () const { assert(isValid()); return x->name; };
	bool operator== ( const Symbol & other ) const { return ( x == other.x ); }
private:
	Symbol( PyrSymbol *sym ) : x(sym) {};
	PyrSymbol *x;
};

}
}

#endif
