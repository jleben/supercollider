#include "SC_LangPluginWorld.h"
#include "SC_DirUtils.h"
#include "SCBase.h"

#include <string.h>
#ifdef _WIN32
# include <windows.h>
#else
# include <dlfcn.h>
#endif

using namespace SC::Lang;

SC_LangPluginWorld::PluginVector SC_LangPluginWorld::plugins;

void SC_LangPluginWorld::loadAll()
{
#ifdef SC_LANG_PLUGIN_DIR
	loadDir(SC_LANG_PLUGIN_DIR);
#endif
}

void SC_LangPluginWorld::loadDir (const char *dirName)
{
	printf("Loading plugin directory: %s\n", dirName);
	if (!sc_DirectoryExists(dirName)) { printf("Plugin directory does not exist!\n"); return; }
	SC_DirHandle *dir = sc_OpenDir(dirName);
	if (!dir) { printf("Could not open plugin directory!\n"); return; }

	char filename[MAXPATHLEN];
	bool skip = false;

	while (sc_ReadDir(dir, dirName, filename, skip)) {
		//printf("Checking if file is plugin: %s\n", filename);
		char *suffix = strrchr(filename, '.');
		if (!suffix ||
			strcmp(suffix, SC_LANG_PLUGIN_SUFFIX)) continue;
		loadFile(filename);
	}
}

Plugin *SC_LangPluginWorld::load (const char *pluginName)
{
#ifdef SC_LANG_PLUGIN_DIR
	char filename[MAXPATHLEN] = SC_LANG_PLUGIN_DIR;
	char file[MAXPATHLEN];
	sprintf(file, "%s%s%s", SC_LANG_PLUGIN_PREFIX, pluginName, SC_LANG_PLUGIN_SUFFIX);
	sc_AppendToPath(filename, file);
	return loadFile(filename);
#else
	post( "No language plugin directory given. Can not load plugin '%s' by name!", pluginName );
#endif
}

Plugin *SC_LangPluginWorld::loadFile (const char *filename)
{
	post("Loading plugin at: %s\n", filename);

	Plugin *plugin = 0;

#if defined _WIN32

	HINSTANCE hinstance = LoadLibrary(filename);
	if (!hinstance) {
		char *s;
		DWORD lastErr = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				0, lastErr , 0, (char*)&s, 1, 0);
		printf("ERROR: LoadLibrary '%s'\n", filename);
		printf("%s\n", s);
		LocalFree(s);
		return 0;
	}

	void *ptr = (void *)GetProcAddress(hinstance, SC_LANG_PLUGIN_ENTRY);
	if (!ptr) {
		char *s;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				0, GetLastError(), 0, (char*)&s, 1, 0);
		printf("ERROR: GetProcAddress '%s'\n", SC_LANG_PLUGIN_ENTRY);
		printf("%s\n", s);
		LocalFree(s);

		FreeLibrary(hinstance);
		return 0;
	}

	PluginEntry loadPlugin = (PluginEntry) ptr;
	plugin = (*loadPlugin)();

	// FIXME: at the moment we never call FreeLibrary() on a loaded plugin

#else

	char *error;

	void *hnd = dlopen(filename, RTLD_NOW);
	if (!hnd) {
		printf("dlopen failed:\n");
		fprintf(stderr, "%s\n", dlerror());
		return 0;
	}

	dlerror();
	PluginEntry loadPlugin = (PluginEntry)dlsym(hnd, SC_LANG_PLUGIN_ENTRY);
	if ((error = dlerror()) != 0) {
		printf("dsym failed:\n");
		fprintf(stderr, "%s\n", error);

		dlclose(hnd);
		return 0;
	}

	plugin = (*loadPlugin)();

	// FIXME close handle when finished using the plugin

#endif

	if (plugin) {
		printf("Plugin '%s' - %s\n", plugin->name, plugin->description);
		plugins.push_back(plugin);
	}

	return plugin;
}

Plugin *SC_LangPluginWorld::get (const char *name)
{
	PluginVector::iterator it;
	for (it = plugins.begin(); it < plugins.end(); ++it)
	{
		Plugin *p = *it;
		if (!strcmp(p->name, name)) return p;
	}
	return 0;
}
