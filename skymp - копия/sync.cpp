#include "skyplug.h"
#include <list>
#include <mutex>

void xy_behind(float angle, float& x, float& y, int distance)
{
	const double PI = 3.14159265;
	x += float(distance * std::sin(angle * PI / 180));
	y += float(distance * std::cos(angle * PI / 180));
}

struct mate_t : actor_info
{
	unsigned id = NULL;

	int old_dirid_dest;
	float speed_dest_last = 0;
	ScriptDragon::TESObjectREFR *dop = nullptr;
	bool sneaking = false,
		drawn = false,
		synced = false,
		dop_flag = false,
		isjumping = false;
	UInt8 jampvar_current = 0;

	void tick(const player_t &player, ScriptDragon::TESObjectREFR *local_pl_obj)
	{
		using namespace skyrim_string;

		const float x_dest = other_players[id].x + 128,
			y_dest = other_players[id].y + 128,
			z_dest = other_players[id].z,
			za_dest = other_players[id].za;
		const float hpmax_dest = 32100,
			hppercent_dest = 100;
		const UInt32 flags04_dest = other_players[id].flags04,
			flags08_dest = other_players[id].flags08;
		const float speed_dest = other_players[id].speed, direction_dest = other_players[id].direction;
		const bool isRunning_dest = other_players[id].isRunning != 0,
			isSprinting_dest = other_players[id].isSprinting != 0,
			isSneaking_dest = other_players[id].isSneaking != 0,
			drawn_dest = other_players[id].drawn != 0;
		const UInt8 jumpvar_dest = other_players[id].jumpvar;
		const float shit_dest = other_players[id].shit;
		
		{
			int translate_speed = 1000000;

			const float val = 1/*ScriptDragon::Actor::IsInCombat(this->actor)*/ ? 250.f : 100000.f;
			NiPoint3 offset{ NULL, NULL, val};

			float za_dest_ = za_dest + direction_dest * 360;
			while (za_dest_ > 360)
				za_dest_ -= 360;
			xy_behind(za_dest_, offset.x, offset.y, 20);
			ScriptDragon::ObjectReference::TranslateTo(this->dop, x_dest + offset.x,
				y_dest + offset.y,
				z_dest + offset.z,
				0,
				0,
				za_dest,
				1000000,
				1000000);

			if (jumpvar_dest)
			{
				if (!this->isjumping)
				{
					this->isjumping = true;
					((Actor *)this->actor)->animGraphHolder.SendAnimationEvent(&JumpDirectionalStart);
				}
			}
			else
			{
				if (this->isjumping)
				{
					this->isjumping = false;
					((Actor *)this->actor)->animGraphHolder.SendAnimationEvent(&JumpLand);
				}
			}

			if (isSprinting_dest) // Sprinting
			{
				translate_speed = 500;
				ScriptDragon::Actor::ForceActorValue(this->actor, "Stamina", 32000);
				ScriptDragon::Actor::IsSneaking(this->actor) ?
					NULL : ((Actor *)this->actor)->animGraphHolder.SendAnimationEvent(&sprintstart_fs);
				ScriptDragon::Actor::KeepOffsetFromActor(this->actor, (ScriptDragon::CActor *)this->dop, 0, 0, offset.z * -1, 0, 0, 0, 1, 0.00001f);
			}
			else
			{
				((Actor *)this->actor)->animGraphHolder.SendAnimationEvent(&sprintstop_fs);
				if (isRunning_dest || speed_dest > 147) //Running
				{
					translate_speed = (direction_dest == 0.5f) // S
						? (isSneaking_dest ? 150 : 205) : (isSneaking_dest ? 222 : 370);
					
					ScriptDragon::Actor::KeepOffsetFromActor(this->actor, (ScriptDragon::CActor *)this->dop, 0, 0, offset.z * -1, 0, 0, 0, 1, 0.00001f);
				}
				else if (speed_dest) // Walking
				{
					if (isSneaking_dest)
					{
						translate_speed = 50;
						ScriptDragon::Actor::KeepOffsetFromActor(this->actor, (ScriptDragon::CActor *)this->dop, 0, 0, offset.z * -1, 0, 0, 0, 100, 0.00001f);
					}
					else
					{
						translate_speed = 80;
						ScriptDragon::Actor::KeepOffsetFromActor(this->actor, (ScriptDragon::CActor *)this->dop, 0, 0, offset.z * -1, 0, 0, 0, 100, 0.00001f);
					}
				}
				else
				{
					ScriptDragon::Actor::KeepOffsetFromActor(this->actor, this->actor, 0, 0, 0, 0, 0, 0, 1, 1);
				}

				static BSFixedString sneak[2]{ "sneakStop", "sneakStart" };
				if (this->sneaking != isSneaking_dest || !synced)
				{
					((Actor *)this->actor)->animGraphHolder.SendAnimationEvent(&sneak[isSneaking_dest]);
					this->sneaking = !this->sneaking;
				}

				static BSFixedString drawn[2]{ "BeginWeaponDraw", "WeapEquip_Out" };
				((Actor *)this->actor)->DrawSheatheWeapon(drawn_dest);
				if (drawn_dest)
				{
					ScriptDragon::Actor::StartCombat(this->actor, (ScriptDragon::CActor *) this->dop);ScriptDragon::Actor::SetActorValue((ScriptDragon::CActor *) this->dop, "Sneak", -1000000000);
				}
				if (ScriptDragon::Actor::IsInCombat(this->actor) && (void *)ScriptDragon::Actor::GetCombatTarget(this->actor) == player.actor)
					ScriptDragon::Actor::StopCombat(this->actor);

				if (jumpvar_dest)
					translate_speed = 2000;

				synced = true;
			}

			ScriptDragon::Actor::ForceActorValue((ScriptDragon::CActor *)this->dop, "Health", 32000);
			ScriptDragon::ObjectReference::TranslateTo(this->object, x_dest, y_dest, z_dest, 0, 0, za_dest, translate_speed, 100000);
		}

		ScriptDragon::Actor::SetHeadTracking(this->actor, false);
		ScriptDragon::Actor::SetRelationshipRank(this->actor, (ScriptDragon::CActor *)local_pl_obj, 4);
		ScriptDragon::ObjectReference::IgnoreFriendlyHits(this->object, true);

		char *const hp_str = "health";
		if (ScriptDragon::Actor::GetActorValue(this->actor, hp_str) != hpmax_dest)
			ScriptDragon::Actor::SetActorValue(this->actor, hp_str, hpmax_dest);
		const float hp_to_add = hpmax_dest * (hppercent_dest - ScriptDragon::Actor::GetActorValuePercentage(this->actor, hp_str));
		if (hp_to_add > 0)
			ScriptDragon::Actor::RestoreActorValue(this->actor, hp_str, hp_to_add);
		else if (hp_to_add < 0)
			ScriptDragon::Actor::DamageActorValue(this->actor, hp_str, hp_to_add);
	}
};
std::list<mate_t> mates;
//std::deque<size_t> players_to_add;

void skyplug::do_sync(const player_t &player)
{
	/*while (!players_to_add.empty())
	{
		skyplug::add_player(player, players_to_add.front());
		players_to_add.pop_front();
	}*/
	if (id_to_add != -1)
	{
		skyplug::add_player(player, id_to_add);
		ScriptDragon::PrintNote("add player 2");
		id_to_add = -1;
	}
	for (auto it = mates.begin(); it != mates.end(); ++it)
		it->tick(player, player.object);
}

/*void skyplug::force_add_player(const player_t &player, size_t id)
{
	std::mutex m;
	m.lock();
	players_to_add.push_back(id);
}*/

void skyplug::add_player(const player_t &player, size_t id)
{
	TESNPC *const base = (TESNPC *)ScriptDragon::Game::GetFormById(0x10d13e);
	TESNPC *const base_dop = (TESNPC *)ScriptDragon::Game::GetFormById(0x71e6b);
	base->actorData.voiceType = (BGSVoiceType *)ScriptDragon::Game::GetFormById(0x2f7c3);
	base->aiForm.flags = 0;
	base_dop->aiForm.flags = 0;

	mate_t new_mate;
	new_mate.id = id;
	new_mate.actor = ScriptDragon::ObjectReference::PlaceActorAtMe(player.object, (ScriptDragon::TESNPC *)base, 0, nullptr);
	ScriptDragon::Actor::KeepOffsetFromActor(new_mate.actor, new_mate.actor, 0, 0, 0, 0, 0, 0, 1, 1);
	skyplug::set_deleteable(new_mate.actor, false);
	ScriptDragon::Actor::StartCombat(new_mate.actor, (ScriptDragon::CActor *)new_mate.dop);
	ScriptDragon::Actor::StopCombat(new_mate.actor);
	new_mate.dop = (ScriptDragon::TESObjectREFR *)ScriptDragon::ObjectReference::PlaceActorAtMe(player.object, (ScriptDragon::TESNPC *)base_dop, 0, nullptr);
	ScriptDragon::Actor::EnableAI((ScriptDragon::CActor *)new_mate.dop, 0);
	skyplug::set_deleteable((ScriptDragon::CActor *)new_mate.dop, false);
	mates.push_back(new_mate);
}


void skyplug::remove_player(size_t id)
{
	for (auto it = mates.begin(); it != mates.end(); ++it)
		if (it->id == id)
		{
			ScriptDragon::ObjectReference::Delete(it->dop);
			ScriptDragon::ObjectReference::Delete(it->object);
			mates.erase(it);
		}
}

void skyplug::set_deleteable(ScriptDragon::CActor *ac, bool modificator)
{
	if(ac)
		ScriptDragon::Actor::SetActorValue(ac, "Variable03", modificator ? NULL : 1000.f);
}