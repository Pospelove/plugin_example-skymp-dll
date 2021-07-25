#include "skse/skse.h"
#include "skse/PluginAPI.h"		// super
#include "skse/skse_version.h"	// What version of SKSE is running?
#include <shlobj.h>				// CSIDL_MYCODUMENTS

#include "scriptdragon/plugin.h"

static PluginHandle					g_pluginHandle = kPluginHandle_Invalid;
static SKSEPapyrusInterface         * g_papyrus = NULL;

extern HMODULE g_hModule;

extern "C"	{

	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)	{	// Called by SKSE to learn about this plugin and check that it's safe to load it
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim\\SKSE\\MyPluginScript.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_Error);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		_MESSAGE("SkyMP");

		// populate info structure
		info->infoVersion =	PluginInfo::kInfoVersion;
		info->name =		"SkyMP";
		info->version =		1;

		// store plugin handle so we can identify ourselves later
		g_pluginHandle = skse->GetPluginHandle();

		if(skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");

			return false;
		}
		else if(skse->runtimeVersion != RUNTIME_VERSION_1_9_32_0)
		{
			_MESSAGE("unsupported runtime version %08X", skse->runtimeVersion);

			return false;
		}

		// ### do not do anything else in this callback
		// ### only fill out PluginInfo and return true/false

		// supported runtime version
		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)	{	// Called by SKSE to load this plugin
		_MESSAGE("SkyMP loaded");

		g_papyrus = (SKSEPapyrusInterface *)skse->QueryInterface(kInterface_Papyrus);

		//Check if the function registration was a success...
		/*bool btest = g_papyrus->Register(MyPluginNamespace::RegisterFuncs);

		if (btest) {
			_MESSAGE("Register Succeeded");
		}*/

		///* раньше инициализация скрипт дрэгона была тут: */
		ScriptDragon::DragonPluginInit(g_hModule);
		return true;
	}
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		g_hModule = hModule;
		//SKSE_Initialize();
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		//SKSE_DeInitialize();
		break;
	}
	}
	return TRUE;
}

void MainThread(void);

extern "C" __declspec(dllexport) void main()
{
	__try
	{
		MainThread();
		//SKSE_Initialize();
		//InitializeGame();
		/*while (true)
		{
			//Update();
			Wait(0);
			PrintNote("YO");
		}*/
	}
	__except (/*GenerateDump(GetExceptionInformation())*/ true)
	{}
}
