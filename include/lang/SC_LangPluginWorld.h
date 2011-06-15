#ifndef SC_LANG_PLUGIN_WORLD_H
#define SC_LANG_PLUGIN_WORLD_H

#include <SC_Lang_Plugin.h>

#include <vector>

class SC_LangPluginWorld
{
public:
	typedef std::vector<SC::Lang::Plugin*> PluginVector;
	static void loadAll ();
	static void loadDir (const char *dir);
	static SC::Lang::Plugin *load (const char *name);
	static SC::Lang::Plugin *loadFile (const char *filename);
	static SC::Lang::Plugin *get (const char *name);
	static PluginVector plugins;
};

#endif // SC_LANG_PLUGIN_WORLD_H
