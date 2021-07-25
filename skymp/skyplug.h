#pragma once
#pragma warning(disable : 4996)

#include "scriptdragon/skyscript.h"
#include "scriptdragon/plugin.h"
#include "scriptdragon/enums.h"
#include <skse/GameData.h>
#include <skse/GameForms.h>
#include <skse/GameTypes.h>
#include <skse/GameMenus.h>
#include <skse/GameInput.h>
#include <skse/Hooks_Gameplay.h>
#include <skse/Hooks_DirectInput8Create.h>
#include <skse/PapyrusInput.h>
#include <skse/PapyrusCell.h>
#include <skse/PapyrusActor.h>
#include <ctime>
#include <queue>
#include <map>
#include <array>
#include <thread>
#include "tesenum.h"

static MenuManager *const mm = MenuManager::GetSingleton();
static InputStringHolder *const ish = InputStringHolder::GetSingleton();

static unsigned short threadcheck = 0;
static unsigned long destpause = 0;
static unsigned long disablearm = 0;
static bool ininventory = false;
#define pause ((unsigned long *)0x1B3E428)
static bool ismenuopen[36] = { false };
enum struct started_mode
{
	none = NULL,
	offline,
	online
};
static enum struct started_mode started_mode = started_mode::none;

void OtherThread();
void ButtonsCheckThread();
void MainThread();

struct actor_info
{
	float x = 0, y = 0, z = 0;
	union
	{
		ScriptDragon::TESObjectREFR *object;
		ScriptDragon::CActor *actor;
	};
};

struct player_t : public actor_info
{
	player_t(ScriptDragon::CActor *player_actor, ScriptDragon::TESNPC *player_actorbase) :
		actorbase(player_actorbase)
	{
		this->actor_info::actor = player_actor;
		ScriptDragon::ActorBase::SetEssential(player_actorbase, true);
		ScriptDragon::ObjectReference::RemoveAllItems((ScriptDragon::TESObjectREFR *)player_actor, nullptr, false, true);
		ScriptDragon::Game::ForceFirstPerson();
	}

	ScriptDragon::TESNPC *const actorbase;
	ScriptDragon::TESObjectCELL *cell = nullptr;
	bool ismoving = false;
	float isfalling = false;
	bool changedCell = false;
};

struct controls_t
{
	bool movement = true,
		fighting = true,
		camswitch = true,
		looking = true,
		sneaking = true,
		menu = true,
		activate = true,
		journaltabs = false;
	bool wait = false,
		save = false;
	bool fasttravel = false;

	static bool debug;
};

static bool disable_game_by_console = 0;
static bool speccheck = 0;
static UInt8 wait_for_troll = 0;
static bool controlsdebug = 0;

namespace skyplug
{
	/**static int id_to_add = -1;**/

	void tick(player_t &player, controls_t &controls);
	void clear_default_spell_list(ScriptDragon::TESNPC *npc);
	void update_controls(const controls_t &controls);
	void update_pos(player_t &player);
	void remove_racial_powers(player_t &player);
	void remove_some_actors(const player_t &player);
	void remove_objectrefs(const player_t &player);
	void clear_cell(TESObjectCELL *cell);
	bool fix_inventory_camera(controls_t &controls);
	void detect_death(player_t &player);
	void debug_login();
	void disable_console();
	void do_debug(const player_t &player);
	void do_sync(const player_t &player);
	void add_player(const player_t &player, size_t id);
	void remove_player(size_t id);
	void set_deleteable(ScriptDragon::CActor *ac, bool modificator);
	void quit_with_msg(char *);
	void get_my_sync_data(const player_t&);
}

struct packet_t
{
	float x = 0,
		y = 0,
		z = 0,
		za = 0;
	float hppercent = 0;
	unsigned long flags04 = 0,
		flags08 = 0;
	float speed = 0,
		direction = 0;
	int isRunning = 0,
		isSprinting = 0,
		isSneaking = 0,
		drawn = 0;
	int jumpvar = 0;
	float shit = 0;

	int id = -1;
	bool is_active = false;

private:
	#define _format  "%f %f %f %f %f %d %d %f %f %d %d %d %d %d %f %d"
public:

	const char* serialize()
	{
		static char data[128]{ 0 };
		sprintf_s<sizeof data>(data, _format, 
			x, y, z, za, hppercent, flags04, flags08, speed, direction, isRunning, isSprinting, isSneaking, drawn, jumpvar, shit, id);
		return data;
	}

	void deserialize(const char* data)
	{
		sscanf(data, _format, 
			&x, &y, &z, &za, &hppercent, &flags04, &flags08, &speed, &direction, &isRunning, &isSprinting, &isSneaking, &drawn, &jumpvar, &shit, &id);
	}
};

extern packet_t my_data, other_players[1000];
static bool connected_to_server = false;
static int my_id = -1;

namespace skyrim_string
{
	static BSFixedString dir_fs("direction"),
		speed_fs("SpeedSampled"),
		mdriven_fs("bMotionDriven"),
		adriven_fs("bAnimationDriven"),
		movestart_fs("movestart"),
		sprintstart_fs("sprintstart"),
		sprintstop_fs("sprintstop"),
		jump_fs("bInJumpState"),
		velocityz_fs("VelocityZ"),
		JumpStandingStart("JumpStandingStart"),
		JumpDirectionalStart("JumpDirectionalStart"),
		JumpLand("JumpLand");
}

static sockaddr_in g_dest_addr;