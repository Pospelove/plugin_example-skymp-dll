#include "skyplug.h"
#include "network3.h"
#include <bitset>
#include "cfg.h"

void MainThread()
{
	//return;
	/*if (ScriptDragon::Actor::GetActorValue(ScriptDragon::Game::GetPlayer(), "Variable03") != 108.0f)
	{
		started_mode = started_mode::offline;
		return ScriptDragon::PrintNote("Игра запущена в одиночном режиме");
	}
	if (started_mode != started_mode::none)
		return ScriptDragon::PrintNote("Ошибка: Клиент SkyMP запущен дважды");*/

	// Отвечает частично за паузу и дебаг, за выключение игры при выходе в главное меню
	std::thread thr(OtherThread);
	thr.detach();
	/* Отвечает за исправление краша игры при действиях с броней 
	(если одеть/снять/выбросить броню или одежду, и поток thr2 не работает, то игра вылетает) */
	std::thread thr2(ButtonsCheckThread);
	thr2.detach();
	started_mode = started_mode::online;

	// Данные игрока
	player_t player(ScriptDragon::Game::GetPlayer(), 
		(ScriptDragon::TESNPC *)ScriptDragon::Game::GetFormById(0x7));
	// Данные о блокировке/разблокировке ф-ций (fast travel, ходьба, бой, дебаг и т.д.)
	controls_t controls;

	// Удаление стандартных заклинаний игрока
	skyplug::clear_default_spell_list(player.actorbase);
	// Остановка главного квеста
	ScriptDragon::Quest::Stop((ScriptDragon::TESQuest*)ScriptDragon::Game::GetFormById(0x3372b));

	//ScriptDragon::Game::FadeOutGame(true, true, 0, 0.001);
	ScriptDragon::PrintNote("Клиент SkyMP запущен");
	ScriptDragon::Actor::SetActorValue(player.actor, "Variable09", -108.f); // отключение Papyrus-скрипта

	read_cfg();
	network::run(g_config[CONFIG_IP].data(), 
		std::stoul(g_config[CONFIG_PORT]), 
		g_config[CONFIG_NAME].data());

	// Главный цикл
	while (1)
	{
		threadcheck = 0;
		ScriptDragon::Wait(0);
		skyplug::tick(player, controls);

		InputManager::GetSingleton()->AllowTextInput(ScriptDragon::GetKeyPressed(0x36));
	}

	// Если этот код выполняется, то что-то не так.
	// Потому что главный цикл каким-то образом завершился, а должен быть бесконечным
	ScriptDragon::PrintNote("Ошибка в работе клиента ...");
	ScriptDragon::Wait(1000);
	for (;;);
}

void skyplug::tick(player_t &player, controls_t &controls)
{
	/*В эту переменную необходимо записывать true всякий раз, когда controls меняется,
	Иначе изменения не будут применены*/
	static bool controlsStateChanged = true;

	// Фикс бага, когда игрок не мог двигаться даже после загрузки клиента (см papyrus-скрипт)
	if (ScriptDragon::Actor::GetActorValue(player.actor, "Variable05") == 0)
		controlsStateChanged = true;

	// Как указано выше, происходит обновление control-ов игрока при controlsStateChanged == true
	if (controlsStateChanged
		|| ScriptDragon::Game::IsFastTravelEnabled() != controls.fasttravel)
	{
		controlsStateChanged = false;
		skyplug::update_controls(controls);
	}
	// Координаты и ячейка игрока закидываются в члены player
	skyplug::update_pos(player);
	skyplug::remove_racial_powers(player);
	skyplug::remove_some_actors(player);

	/* Удаление предметов и всего остального на входе в новую ячейку
	(полная зачистка, даже птичек в небе не останется)*/
	if (player.changedCell)
	{
		skyplug::remove_objectrefs(player);
		player.changedCell = false;
	}

	/* Исправление камеры в инвентаре: раньше в инвентаре не двигалась,
	даже при падении игрока с высоты стояла на месте */
	if (skyplug::fix_inventory_camera(controls))
		controlsStateChanged = true;

	// Если игрока убили, "респавним" его на том же месте, иначе бы он упал на землю и загрузилось сохранение
	skyplug::detect_death(player);

	// Пытается ли игрок войти в режим отладки? Если да, то открываем ему функции тестирования и консоль
	skyplug::debug_login();

	// Постоянное обнуление через SetGodMode спасает от консольной команды tgm (на всякий случай)
	if (!controls.debug)
	{
		ScriptDragon::Debug::SetGodMode(false);
		skyplug::disable_console();
	}

	// Отладка
	skyplug::do_debug(player);

	// Синхронизация
	skyplug::get_my_sync_data(player);
	skyplug::do_sync(player);
	network::tick_udp(player, g_dest_addr);
}

void OtherThread()
{
	Hooks_DirectInput_Commit();
	while (1)
	{
		if (started_mode == started_mode::offline)
			return;

		//Debug
		if (controlsdebug)
		{
			if (ScriptDragon::GetKeyPressed(0x32))
				destpause = 0;
			if (ScriptDragon::GetKeyPressed(0x33))
				destpause = 1;
		}

		// Update SKSE variables
		Hooks_Gameplay_Commit();

		// Enable game after console closed
		if (!ScriptDragon::GetKeyPressed(0xC0))
			speccheck = false;
		if (!speccheck && ScriptDragon::GetKeyPressed(0xC0) && disable_game_by_console)
		{
			wait_for_troll = 3;
			disable_game_by_console = false;
		}

		// Disable main menu:
		if (mm->IsMenuOpen(&menunames[Menu::MainMenu]))
			ScriptDragon::Debug::QuitGame();

		// Disable pause:
		if (
			mm->IsMenuOpen(&menunames[Menu::MapMenu])
			|| mm->IsMenuOpen(&menunames[Menu::LevelUpMenu])
			|| mm->IsMenuOpen(&menunames[Menu::BookMenu])
			|| mm->IsMenuOpen(&menunames[Menu::StatsMenu])
			|| mm->IsMenuOpen(&menunames[Menu::JournalMenu])
			|| mm->IsMenuOpen(&menunames[Menu::SleepWaitMenu])
			|| mm->IsMenuOpen(&menunames[Menu::LoadingMenu])
			|| mm->IsMenuOpen(&menunames[Menu::RaceSexMenu])
			)
			*pause = 1;
		else
			*pause = destpause | (ininventory || mm->IsMenuOpen(&menunames[Menu::LockpickingMenu])  ? disablearm : false);
	}
}

void ButtonsCheckThread()
{
	bool pressed = false,
		islockpicking = false;
	unsigned int starttime = 0;
	while (true)
	{
		if (started_mode == started_mode::offline)
			return;
		islockpicking = mm->IsMenuOpen(&menunames[Menu::LockpickingMenu]);

		// Check R and Click (upd: and lockpicking state)
		if (pressed != (islockpicking || ScriptDragon::GetKeyPressed(0x01) || ScriptDragon::GetKeyPressed(0x52)))
			starttime = (pressed = !pressed) ? clock() : starttime;
		disablearm = (pressed && clock() - starttime > 1000) ? 0 : pressed;
	}
}

void skyplug::clear_default_spell_list(ScriptDragon::TESNPC *npc)
{
	UInt32 max = ((TESNPC *)npc)->spellList.unk04->numSpells;
	for (UInt32 id = 0; id < max; ++id)
		((TESNPC *)npc)->spellList.unk04->spells[id] = nullptr;
}

void skyplug::update_controls(const controls_t &controls)
{
	ScriptDragon::Game::DisablePlayerControls(!controls.movement, !controls.fighting, !controls.camswitch, !controls.looking, !controls.sneaking, !controls.menu, !controls.activate, !controls.journaltabs, 0);
	ScriptDragon::Game::EnablePlayerControls(controls.movement, controls.fighting, controls.camswitch, controls.looking, controls.sneaking, controls.menu, controls.activate, controls.journaltabs, 0);
	ScriptDragon::Game::EnableFastTravel(controls.fasttravel);
	ScriptDragon::Game::SetInChargen(!controls.save, !controls.wait, 0);
}

void skyplug::update_pos(player_t &player)
{
	if (player.cell != ScriptDragon::ObjectReference::GetParentCell(player.object))
	{
		player.changedCell = true;
		player.cell = ScriptDragon::ObjectReference::GetParentCell(player.object);
	}
	else
		player.changedCell = false;

	static float tmpx = 0, tmpy = 0, tmpz = 0;
	static bool got_xyz = false;

	if (got_xyz)
	{
		player.ismoving =
			ScriptDragon::ObjectReference::GetPositionX(player.object) != tmpx
			|| ScriptDragon::ObjectReference::GetPositionY(player.object) != tmpy
			|| ScriptDragon::ObjectReference::GetPositionZ(player.object) != tmpz;
		player.isfalling = tmpz - ScriptDragon::ObjectReference::GetPositionZ(player.object);
		
	}
	player.x = (tmpx = ScriptDragon::ObjectReference::GetPositionX(player.object));
	player.y = (tmpy = ScriptDragon::ObjectReference::GetPositionY(player.object));
	player.z = (tmpz = ScriptDragon::ObjectReference::GetPositionZ(player.object));
	got_xyz = true;

}

void skyplug::remove_racial_powers(player_t &player)
{
	static TESRace *oldrace = nullptr;
	TESRace *newrace = (TESRace *)ScriptDragon::Actor::GetRace(player.actor);
	if (oldrace != newrace)
	{
		oldrace = newrace;
		auto max = newrace->spellList.unk04->numSpells;
		for (SInt32 id = 0; id < max; ++id)
			newrace->spellList.unk04->spells[id] = nullptr;
		max = newrace->spellList.unk04->numShouts;
		for (SInt32 id = 0; id < max; ++id)
			newrace->spellList.unk04->shouts[id] = nullptr;

		tList<ActiveEffect> &effects = *((Actor*)(player.actor))->magicTarget.GetActiveEffects();
		effects.RemoveAll();
	}
}

void skyplug::remove_some_actors(const player_t &player)
{
	clock_t c = clock();
	while (1)
	{
		auto tmp = ScriptDragon::Game::FindRandomActor(player.x, player.y, player.z, 12800);
		if (tmp == player.actor || !tmp)
			break;
		if (ScriptDragon::Actor::GetActorValue(tmp, "Variable03") == 1000.f)
			continue;
		ScriptDragon::ObjectReference::Delete((ScriptDragon::TESObjectREFR *)tmp);
	}
	if (ScriptDragon::GetKeyPressed(0x33))
		ScriptDragon::PrintNote("%d", clock() - c);
}

void skyplug::remove_objectrefs(const player_t &player)
{
	const UInt32 pcell_form_id = ScriptDragon::Form::GetFormID((ScriptDragon::TESForm*)player.cell);
	const SInt32 range = 40;
	for (SInt32 n = -range; n < range; ++n)
	{
		TESObjectCELL *curcell = (TESObjectCELL *)ScriptDragon::Game::GetFormById(pcell_form_id + n);
		if (!curcell) 
			continue;
		if (((TESForm*)curcell)->formType == kFormType_Cell)
			skyplug::clear_cell(curcell);
	}
}

void skyplug::clear_cell(TESObjectCELL *curcell)
{

	const UInt32 count = papyrusCell::GetNumRefs(curcell, kFormType_None);
	for (size_t i = 0; i < count; ++i)
	{
		auto tmp = papyrusCell::GetNthRef(curcell, i, 0);
		bool need_delete = false;

		ScriptDragon::BGSListForm *const fish_form_list[2] = {
			(ScriptDragon::BGSListForm *)ScriptDragon::Game::GetFormById(0x001093F7),
			(ScriptDragon::BGSListForm *)ScriptDragon::Game::GetFormById(0x001093F6)
		};
		for (int ind = 0; ind < 2; ++ind)
		{
			if (ScriptDragon::FormList::HasForm(fish_form_list[ind],
				(ScriptDragon::TESForm*)tmp->baseForm))
				need_delete = true;
		}

		switch (tmp->baseForm->formType)
		{
		case FormType::kFormType_Ingredient:
		case FormType::kFormType_ScrollItem:
		case FormType::kFormType_Armor:
		case FormType::kFormType_Book:
		case FormType::kFormType_Misc:
		case FormType::kFormType_Weapon:
		case FormType::kFormType_Ammo:
		case FormType::kFormType_Key:
		case FormType::kFormType_Potion:
		case FormType::kFormType_SoulGem:
		case FormType::kFormType_LeveledItem:
			need_delete = true;
			break;
		case FormType::kFormType_NPC:
		case FormType::kFormType_LeveledCharacter:
			need_delete = false;
			break;
		case FormType::kFormType_Tree:
			if ((((TESObjectTREE*)(tmp->baseForm))->produce.produce))
				need_delete = true;
			break;
		case FormType::kFormType_Flora:
			if ((((TESFlora*)(tmp->baseForm))->produce.produce))
				need_delete = true;
			break;
		case FormType::kFormType_Container:
			need_delete = false;
			if (tmp)
			{
				ScriptDragon::ObjectReference::SetActorOwner((ScriptDragon::TESObjectREFR *)tmp, (ScriptDragon::TESNPC *)ScriptDragon::Game::GetFormById(0x7));
				ScriptDragon::ObjectReference::RemoveAllItems((ScriptDragon::TESObjectREFR *)tmp, nullptr, false, true);
				ScriptDragon::ObjectReference::Lock((ScriptDragon::TESObjectREFR *)tmp, true, false);
				ScriptDragon::ObjectReference::SetLockLevel((ScriptDragon::TESObjectREFR *)tmp, 255);
			}
			break;
		case FormType::kFormType_Door:
			need_delete = false;
			if (tmp)
			{
				ScriptDragon::ObjectReference::Lock((ScriptDragon::TESObjectREFR *)tmp, true, false);
				ScriptDragon::ObjectReference::SetLockLevel((ScriptDragon::TESObjectREFR *)tmp, 255);
			}
			break;
		case FormType::kFormType_Activator:
			switch ((tmp->baseForm)->formID)
			{
				//Алтари
			case 891015:case 891013:case 891011:case 891009:case 891007:case 891003:case 891005:case 464980:case 1050496:
				// Летающие насекомые
			case 0xd95c5:case 0xd95c7:case 0xc2d3f:case 0xddf4c:case 0x22219:case 0x2221e:case 0xb97af:case 0xa9169:
				// Птицы
			case 0x10581f:case 0x105824:
				// Рыбы
			case 0xad0cc:case 0xd337f:case 0x106d28:case 0x106d29:case 0x106d2A:case 0x106d2B:case 0x106d2C:
				// Корни нирна, корзины с яйцами (оригмы?)
			case 0xe1fb2:case 0xb6fb9:case 0x89a8a:case 0xe2ff9:case 0xb6fcb:
				// waterfall salmons
			case 0x10b035:case 0x10c3ba:
				need_delete = true;
			default:
				break;
			}
		default:
			break;
		}

		if (tmp && need_delete)
			ScriptDragon::ObjectReference::Delete((ScriptDragon::TESObjectREFR *) tmp);
	}
}

bool skyplug::fix_inventory_camera(controls_t &controls)
{
	auto ininventory = (
		mm->IsMenuOpen(&menunames[Menu::BarterMenu])
		|| mm->IsMenuOpen(&menunames[Menu::FavoritesMenu])
		|| mm->IsMenuOpen(&menunames[Menu::ContainerMenu])
		|| mm->IsMenuOpen(&menunames[Menu::GiftMenu])
		|| mm->IsMenuOpen(&menunames[Menu::InventoryMenu])
		|| mm->IsMenuOpen(&menunames[Menu::MagicMenu])
		);
	if (ininventory == controls.camswitch)
	{
		controls.camswitch = !controls.camswitch;
		ScriptDragon::Game::ForceFirstPerson();
		return true;
	}
	return false;
}

void skyplug::detect_death(player_t &player)
{
	const float modificator = 10.f;
	static char *health_av = "health";
	static bool restored = false;
	if (restored)
	{
		restored = false;
		ScriptDragon::Actor::ModActorValue(player.actor, health_av, -modificator);
		return;
	}
	if (ScriptDragon::Actor::IsBleedingOut(player.actor))
	{
		static float restore;
		restore = (1 - ScriptDragon::Actor::GetActorValuePercentage(player.actor, health_av)) * ScriptDragon::Actor::GetActorValue(player.actor, health_av);
		if (restore)
			ScriptDragon::Actor::RestoreActorValue(player.actor, health_av, restore);
		else
		{
			ScriptDragon::Actor::ModActorValue(player.actor, health_av, modificator);
			restored = true;
		}
		ScriptDragon::Game::ForceFirstPerson();
	}
}


void skyplug::debug_login()
{
	static bool alreadyCombPressed = false;
	bool combPressed =(ScriptDragon::GetKeyPressed(0x4F) //'O'
		&& ScriptDragon::GetKeyPressed(0x39) //'9'
		&& ScriptDragon::GetKeyPressed(0x4C) /*'L' */);
	if(!combPressed)
		alreadyCombPressed = false;
	else if (!alreadyCombPressed)
	{
		alreadyCombPressed = !alreadyCombPressed;
		controls_t::debug = !controls_t::debug;
		ScriptDragon::PrintNote(controls_t::debug ? "Debug ON" : "Debug OFF");
	}
}

void skyplug::disable_console()
{
	if (wait_for_troll)
		--wait_for_troll;
	else if (mm->IsMenuOpen(&menunames[Menu::Console]) && !disable_game_by_console)
	{
		speccheck = true;
		disable_game_by_console = true;
		static UInt8 warns = 0, max_warns = 3;
		++warns;
		if (warns == max_warns)
			return ScriptDragon::Game::QuitToMainMenu();
		ScriptDragon::Debug::ShowMessageBox(
			"Игра приостановила свою работу, так как использование консольных команд запрещено. \
			Чтобы продолжить игру, закройте консоль. При повторных попытках использовать консоль, \
			приложение автоматически завершит работу.");
		for (char tmpi = 0; tmpi < 16; ++tmpi)
			ScriptDragon::Wait(0);
		while (disable_game_by_console);
	}
}

bool controls_t::debug = false;
void skyplug::do_debug(const player_t &player)
{
	if (!controls_t::debug || mm->IsMenuOpen(&menunames[Menu::Console]))
		return;

	static bool already_pressed;
	if (ScriptDragon::GetKeyPressed(0x32))
	{
		if (!already_pressed)
		{
			already_pressed = true;
			static size_t id = 0;
			skyplug::add_player(player, id++);
		}
	}
	else
		already_pressed = false;

	static bool already_pressed_1;
	if (ScriptDragon::GetKeyPressed(0x33))
	{
		if (!already_pressed_1)
		{
			already_pressed_1 = true;
			/*const unsigned char *const p = (unsigned char *)&((Actor *)player.actor)->actorState.flags08;
			ScriptDragon::PrintNote("%d %d %d %d",
				p[0], p[1], p[2], p[3]);*/
			BSFixedString jump("bInJumpState");
			UInt8 var;
			((Actor *)player.actor)->animGraphHolder.GetVariableBool(&jump, &var);
			ScriptDragon::PrintNote("jump = %d", var);
		}
	}
	else
		already_pressed_1 = false;

	TESObjectREFR *ref = nullptr;
	if (ScriptDragon::GetKeyPressed(0x31))
	{
		ref = Hooks_Gameplay_GetCrosshairRef();
		if (!ref)
		{
			float f[3]{ 0 };
			/*BSFixedString s("Speed");
			((Actor *)player.actor)->animGraphHolder.GetVariableFloat(&s, &f[0]);
			BSFixedString s1("SpeedAcc");
			((Actor *)player.actor)->animGraphHolder.GetVariableFloat(&s1, &f[1]);
			BSFixedString s2("SpeedSampled");
			((Actor *)player.actor)->animGraphHolder.GetVariableFloat(&s2, &f[2]);*/
			ScriptDragon::PrintNote("%f %f %f", 
				ScriptDragon::ObjectReference::GetPositionX(player.object),
				ScriptDragon::ObjectReference::GetPositionY(player.object),
				ScriptDragon::ObjectReference::GetPositionZ(player.object));
		}
		else
			ScriptDragon::PrintNote("Base: Type=%d;ID=%d", (ref->baseForm)->formType, (ref->baseForm)->formID);
	}
}

void skyplug::quit_with_msg(char *msg)
{
	started_mode = started_mode::offline;
	*pause = destpause = true;
	ScriptDragon::Debug::ShowMessageBox(msg);
}

void skyplug::get_my_sync_data(const player_t& player)
{
	using namespace skyrim_string;
	float direction_tmp = 0;
	((Actor *)player.actor)->animGraphHolder.GetVariableFloat(&dir_fs, &direction_tmp);

	/*my_data.x = (int)ScriptDragon::ObjectReference::GetPositionX(player.object);
	my_data.y = (int)ScriptDragon::ObjectReference::GetPositionY(player.object);
	my_data.z = (int)ScriptDragon::ObjectReference::GetPositionZ(player.object);*/
	my_data.x = ((TESObjectREFR *)player.object)->pos.x;//
	my_data.y = ((TESObjectREFR *)player.object)->pos.y;//
	my_data.z = ((TESObjectREFR *)player.object)->pos.z;//
	my_data.za = (int)ScriptDragon::ObjectReference::GetAngleZ(player.object);//
	my_data.hppercent = ScriptDragon::Actor::GetActorValuePercentage(player.actor, "Health"); //
	my_data.flags04 = ((Actor *)player.object)->actorState.flags04; //
	my_data.flags08 = ((Actor *)player.object)->actorState.flags08; //
	((Actor *)player.object)->animGraphHolder.GetVariableFloat(&speed_fs, &my_data.speed); //
	((Actor *)player.object)->animGraphHolder.GetVariableFloat(&dir_fs, &my_data.direction); //
	my_data.isRunning = ScriptDragon::Actor::IsRunning(player.actor);//
	my_data.isSprinting = ScriptDragon::Actor::IsSprinting(player.actor);//
	my_data.isSneaking = ScriptDragon::Actor::IsSneaking(player.actor);//
	my_data.drawn = ScriptDragon::Actor::IsWeaponDrawn(player.actor);//
	UInt8 tmp;//
	((Actor *)player.object)->animGraphHolder.GetVariableBool(&jump_fs, &tmp); //
	my_data.jumpvar = tmp;//
	my_data.shit = player.isfalling;///
	
}