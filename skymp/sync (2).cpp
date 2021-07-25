#include "skyplug.h"
#include <list>

void xy_behind(float angle, float& x, float& y, int distance)
{
	const double PI = 3.14159265;
	x += float(distance * std::sin(angle * PI / 180));
	y += float(distance * std::cos(angle * PI / 180));
}

struct mate_t : actor_info
{
	int old_dirid_dest;
	float speed_dest_last = 0;
	ScriptDragon::TESObjectREFR *dop = nullptr;
	bool sneaking = false;

	void tick(const player_t &player, ScriptDragon::TESObjectREFR *local_pl_obj)
	{
		float direction_tmp = 0;
		static BSFixedString dir_fs("direction"), 
			speed_fs("SpeedSampled"), 
			mdriven_fs("bMotionDriven"), 
			adriven_fs("bAnimationDriven"),
			movestart_fs("movestart"),
			sprintstart_fs("sprintstart"),
			sprintstop_fs("sprintstop");
		((Actor *)local_pl_obj)->animGraphHolder.GetVariableFloat(&dir_fs, &direction_tmp);

		const float x_dest = ScriptDragon::ObjectReference::GetPositionX(local_pl_obj) + 128,
			y_dest = ScriptDragon::ObjectReference::GetPositionY(local_pl_obj) + 128,
			z_dest = ScriptDragon::ObjectReference::GetPositionZ(local_pl_obj),
			za_dest = ScriptDragon::ObjectReference::GetAngleZ(local_pl_obj);
		const float hpmax_dest = 32100, // Эта константа не должна меняться, меняются только проценты здоровья (hppercent_dest)
			hppercent_dest = 100;
		const UInt32 flags04_dest = ((Actor *)local_pl_obj)->actorState.flags04,
			flags08_dest = ((Actor *)local_pl_obj)->actorState.flags08;
		float speed_dest = 0, direction_dest = 0;
		((Actor *)local_pl_obj)->animGraphHolder.GetVariableFloat(&speed_fs, &speed_dest);
		((Actor *)local_pl_obj)->animGraphHolder.GetVariableFloat(&dir_fs, &direction_dest);
		const bool isRunning_dest = ScriptDragon::Actor::IsRunning(player.actor),
			isSprinting_dest = ScriptDragon::Actor::IsSprinting(player.actor),
			isSneaking_dest = ScriptDragon::Actor::IsSneaking(player.actor);
		
		{
			int translate_speed = 1000000;
			NiPoint3 offset{ NULL, NULL, -1000 };
			float za_dest_ = za_dest + direction_dest * 360;
			while (za_dest_ > 360)
				za_dest_ -= 360;
			xy_behind(za_dest_, offset.x, offset.y, 256);
			/*if (!this->dop)
			{
				TESNPC *const base = (TESNPC *)ScriptDragon::Game::GetFormById(0x10d13e);
				this->dop = (ScriptDragon::TESObjectREFR *)ScriptDragon::ObjectReference::PlaceActorAtMe(player.object, (ScriptDragon::TESNPC *)base, 0, nullptr);
				skyplug::set_deleteable((ScriptDragon::CActor *)this->dop, false);
			}*/
			ScriptDragon::ObjectReference::TranslateTo(this->dop, x_dest + offset.x,
				y_dest + offset.y,
				z_dest + offset.z,
				0,
				0,
				za_dest,
				1000000,
				1000000);

			if (isSprinting_dest) // Sprinting
			{
				translate_speed = 500;
				ScriptDragon::Actor::ForceActorValue(this->actor, "Stamina", 32000);
				((Actor *)this->actor)->animGraphHolder.SendAnimationEvent(&sprintstart_fs);
				ScriptDragon::Actor::KeepOffsetFromActor(this->actor, (ScriptDragon::CActor *)this->dop, 0, 0, offset.z * -1, 0, 0, 0, 1, 1);
			}
			else
			{
				((Actor *)this->actor)->animGraphHolder.SendAnimationEvent(&sprintstop_fs);
				if (isRunning_dest || speed_dest > 147) //Running
				{
					translate_speed = (direction_dest == 0.5f) // S
						? 205 : 370;
					
					ScriptDragon::Actor::KeepOffsetFromActor(this->actor, (ScriptDragon::CActor *)this->dop, 0, 0, offset.z * -1, 0, 0, 0, 1, 1);
				}
				else if (speed_dest) // Walking
				{
					if (isSneaking_dest)
					{
						((Actor *)this->actor)->animGraphHolder.SetVariableFloat(&speed_fs, speed_dest);
						translate_speed = 50;
						ScriptDragon::Actor::KeepOffsetFromActor(this->actor, (ScriptDragon::CActor *)this->dop, 0, 0, offset.z * -1, 0, 0, 0, 1, 1);
					}
					else
					{
						((Actor *)this->actor)->animGraphHolder.SetVariableFloat(&speed_fs, speed_dest);
						translate_speed = 80;
						ScriptDragon::Actor::KeepOffsetFromActor(this->actor, (ScriptDragon::CActor *)this->dop, 0, 0, offset.z * -1, 0, 0, 0, 10100, 0.1);
					}
				}
				else
				{
					ScriptDragon::Actor::KeepOffsetFromActor(this->actor, this->actor, 0, 0, 0, 0, 0, 0, 1, 1);
				}

				static BSFixedString sneak[2]{ "sneakStop", "sneakStart" };
				if (this->sneaking != isSneaking_dest)
				{
					((Actor *)this->actor)->animGraphHolder.SendAnimationEvent(&sneak[isSneaking_dest]);
					this->sneaking = !this->sneaking;
				}
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

		if (ScriptDragon::Actor::IsInCombat(this->actor))
			ScriptDragon::Actor::StopCombat(this->actor);
	}
};
std::list<mate_t> mates;

void skyplug::do_sync(const player_t &player)
{
	for (auto it = mates.begin(); it != mates.end(); ++it)
		it->tick(player, player.object);
}

void skyplug::add_player(const player_t &player, size_t id)
{
	TESNPC *const base = (TESNPC *)ScriptDragon::Game::GetFormById(0x10d13e);
	base->actorData.voiceType = (BGSVoiceType *)ScriptDragon::Game::GetFormById(0x2f7c3);
	base->aiForm.flags = 0;

	mate_t new_mate;
	new_mate.actor = ScriptDragon::ObjectReference::PlaceActorAtMe(player.object, (ScriptDragon::TESNPC *)base, 0, nullptr);
	ScriptDragon::Actor::KeepOffsetFromActor(new_mate.actor, new_mate.actor, 0, 0, 0, 0, 0, 0, 1, 1);
	skyplug::set_deleteable(new_mate.actor, false);
	new_mate.dop = (ScriptDragon::TESObjectREFR *)ScriptDragon::ObjectReference::PlaceActorAtMe(player.object, (ScriptDragon::TESNPC *)base, 0, nullptr);
	skyplug::set_deleteable((ScriptDragon::CActor *)new_mate.dop, false);

	/*BSFixedString mdriven_fs("bMotionDriven"),
		adriven_fs("bAnimationDriven");
	((Actor *)&new_mate.actor)->animGraphHolder.SetVariableBool(&mdriven_fs, false);
	((Actor *)&new_mate.actor)->animGraphHolder.SetVariableBool(&adriven_fs, false);*/

	mates.push_back(new_mate);
}

void skyplug::set_deleteable(ScriptDragon::CActor *ac, bool modificator)
{
	if(ac)
		ScriptDragon::Actor::SetActorValue(ac, "Variable03", modificator ? NULL : 1000.f);
}