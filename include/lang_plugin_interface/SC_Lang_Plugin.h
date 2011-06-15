#ifndef SC_LANG_PLUGIN_H
#define SC_LANG_PLUGIN_H

#include <SC_Lang_Sys.h>

namespace SC {
namespace Lang {

class PluginIntf {
public:
	PluginIntf( PyrInterfaceTable *intf ) : _intf(intf) {}
	Sys system() const { return Sys(_intf); }
private:
	PyrInterfaceTable *_intf;
};

struct Plugin
{
	enum Type {
		Default,
		Main
	};
	Plugin() : type(Default), startup(0) {}
	const char * name;
	const char * description;
	const Type type;
	void (*startup) (const PluginIntf &);
	void (*initPrimitives) ();
	void (*cleanup) ();

protected:
	Plugin(Type t) : type(t) {}
};

struct MainPlugin : public Plugin
{
	MainPlugin() : Plugin( Main ) {}
	int (*run)();
};

typedef Plugin * (*PluginEntry)();

#if defined _WIN32 || defined __CYGWIN__
#  define SC_LANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#  if __GNUC__ >= 4
#    define SC_LANG_PLUGIN_EXPORT extern "C" __attribute__ ((visibility("default")))
#  else
#    define SC_LANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

#define SC_LANG_PLUGIN_ENTRY "load"

#ifdef STATIC_PLUGINS
	#define SC_LANG_PLUGIN_ENTRY_HEADER(name) SC::Lang::Plugin * name##_Load()
#else
	#define SC_LANG_PLUGIN_ENTRY_HEADER(name) SC_LANG_PLUGIN_EXPORT SC::Lang::Plugin * load()
#endif

#define DeclareLanguagePlugin(aName, aDesc, aInitPrim, aStartup, aCleanup) \
	SC_LANG_PLUGIN_ENTRY_HEADER(nameArg) { \
		static char n[] = aName; \
		static char d[] = aDesc; \
		static SC::Lang::Plugin plugin; \
		plugin.name = n; \
		plugin.description = d; \
		plugin.initPrimitives = aInitPrim; \
		plugin.startup = aStartup; \
		plugin.cleanup = aCleanup; \
		return &plugin; \
	}

}
}

#endif // !defined SC_LANG_PLUGIN_H
