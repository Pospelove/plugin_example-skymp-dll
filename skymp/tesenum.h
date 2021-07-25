#pragma once
#include "skse/GameTypes.h"

static BSFixedString menunames[36] =
{
	"BarterMenu",				//0
	"Book Menu",				//1
	"Console",					//2
	"Console Native UI Menu",	//3
	"ContainerMenu",			//4
	"Crafting Menu",			//5
	"Credits Menu",				//6
	"Cursor Menu",				//7
	"Debug Text Menu",			//8
	"Dialogue Menu",			//9
	"Fader Menu",				//10
	"FavoritesMenu",			//11
	"GiftMenu",					//12
	"HUD Menu",					//13
	"InventoryMenu",			//14
	"Journal Menu",				//15
	"Kinect Menu",				//16
	"LevelUp Menu",				//17
	"Loading Menu",				//18
	"Lockpicking Menu",			//19
	"MagicMenu",				//20
	"Main Menu",				//21
	"MapMenu",					//22
	"MessageBoxMenu",			//23
	"Mist Menu",				//24
	"Overlay Interaction Menu",	//25
	"Overlay Menu",				//26
	"Quantity Menu",			//27
	"RaceSex Menu",				//28
	"Sleep / Wait Menu",		//29
	"StatsMenu",				//30
	"TitleSequence Menu",		//31
	"Top Menu",					//32
	"Training Menu",			//33
	"Tutorial Menu",			//34
	"TweenMenu"					//35
};

enum Menu
{
	BarterMenu,
	BookMenu,
	Console,
	ConsoleNativeUIMenu,
	ContainerMenu,
	CraftingMenu,
	CreditsMenu,
	CursorMenu,
	DebugTextMenu,
	DialogueMenu,
	FaderMenu,
	FavoritesMenu,
	GiftMenu,
	HUDMenu,
	InventoryMenu,
	JournalMenu,
	KinectMenu,
	LevelUpMenu,
	LoadingMenu,
	LockpickingMenu,
	MagicMenu,
	MainMenu,
	MapMenu,
	MessageBoxMenu,
	MistMenu,
	OverlayInteractionMenu,
	OverlayMenu,
	QuantityMenu,
	RaceSexMenu,
	SleepWaitMenu,
	StatsMenu,
	TitleSequenceMenu,
	TopMenu,
	TrainingMenu,
	TutorialMenu,
	TweenMenu
};

namespace TESIDArray
{
	static unsigned long ingrIDs[] =
	{
		0x00106E1C,
		0x00106E1B,
		0x00106E1A,
		0x00106E19,
		0x00106E18,
		0x001016B3,
		0x000F11C0,
		0x000EC870,
		0x000D8E3F,
		0x000B701A,
		0x000B08C5,
		0x000A9195,
		0x000A9191,
		0x00085500,
		0x000854FE,
		0x000705B7,
		0x0006BC10,
		0x0006BC0E,
		0x0006BC0B,
		0x0006BC0A,
		0x0006BC07,
		0x0006BC04,
		0x0006BC02,
		0x0006BC00,
		0x00023D77,
		0x00023D6F,
		0x000E7ED0,
		0x000E7EBC,
		0x000E4F0C,
		0x0004DA73,
		0x0004DA25,
		0x0004DA24,
		0x0004DA23,
		0x0004DA22,
		0x0004DA20,
		0x0004DA00,
		0x0001BCBC,
		0x000BB956,
		0x000B2183,
		0x000B18CD,
		0x0009151B,
		0x0006AC4A,
		0x000889A2,
		0x00083E64,
		0x0007EE01,
		0x0007EDF5,
		0x0007E8C8,
		0x0007E8C5,
		0x0007E8C1,
		0x0007E8B7,
		0x00077E1E,
		0x00077E1D,
		0x00077E1C,
		0x000727E0,
		0x000727DF,
		0x000727DE,
		0x0006B689,
		0x0006ABCB,
		0x000516C8,
		0x00074A19,
		0x0006F950,
		0x00063B5F,
		0x00059B86,
		0x00057F91,
		0x00052695,
		0x0005076E,
		0x0004B0BA,
		0x00045C28,
		0x0003F7F8,
		0x0003AD76,
		0x0003AD73,
		0x0003AD72,
		0x0003AD71,
		0x0003AD70,
		0x0003AD6F,
		0x0003AD6A,
		0x0003AD66,
		0x0003AD64,
		0x0003AD63,
		0x0003AD61,
		0x0003AD60,
		0x0003AD5F,
		0x0003AD5E,
		0x0003AD5D,
		0x0003AD5B,
		0x0003AD56,
		0x00034D32,
		0x00034D31,
		0x00034D22,
		0x00034CDF,
		0x00034CDD,
		0x0002F44C,
		0x0001B3BD,
		0x000134AA
	};
}