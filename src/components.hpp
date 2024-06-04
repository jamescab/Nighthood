#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include <map>
#include "../ext/stb_image/stb_image.h"

enum PotionType {
	Heal,
	None
};

struct Potion {
	PotionType effect = None;
	float timeout_ms = 0.f;
};

struct Buff {
	std::string buff_type;
};

struct Door {

};

struct Pedestal {

};

struct Saw {
	float render_update_frame = 50.f;
	float render_curr_frame = render_update_frame;
	// for new animation, mod sheetsize, state_map, state_curr, rowMax, colMax, getCurrState accordingly
	vec2 sheetsize = { 1024.f, 64.f };

	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 16} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true }
	};

	glm::uint rowMax = 16;
	glm::uint colMax = 1;
	glm::uint state_index = 1;
	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		return "idle";
	}

	// for jump detection reset index may be problematic
	// invalid action is ignored
	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}
	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct Readable {
	bool beingRead = false;
	std::vector<std::string> lines;
};

struct FireBall {
	// fireball will have direction, speed, position, health, duration before disappearing
	vec2 initialPos;
	int damage = 1;
};


struct RangedEnemy {
	// ranged Enemy will have a roaming range similar to SkeletonEnemy
	// The attack Range will determine when it will start shooting at the player
	// when player is within attack Range the ranged enemy will stand still and will fire Fireballs towards player position
	vec2 initialPos;
	bool stationary;
	float attackRange;
	float idleSpeed = 100.f;
	float roamRange;
	float chargeUpTime = 0;
	float immunity_duration_ms = 500.f;
	int health = 1;
	int honor = 2;
	float skeleton_curr_frame = 0.f;
	// for new animation, mod sheetsize, state_map, state_curr, rowMax, colMax, getCurrState accordingly
	vec2 sheetsize = { 1652.f, 456.f };

	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "attack", {0, 11} },
		{ "idle", {1, 4} },
		{ "run", {2, 8} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true },
		{ "run", false },
		{ "attack", false }
	};

	glm::uint rowMax = 11;
	glm::uint colMax = 3;
	glm::uint state_index = 1;
	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		if (state_curr["run"]) return "run";
		if (state_curr["attack"]) return "attack";

		return "idle";
	}

	// for jump detection reset index may be problematic
	// invalid action is ignored
	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}
	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct Wizard {
	vec2 initialPos;
	bool stationary;
	float attackRange;
	float roamRange;
	float idleSpeed = 100.f;
	float immunity_duration_ms = 500.f;
	float attack_duration_ms = 0.f;
	float attack_delay_ms = 0.f;
	float hurt_duration_ms = 0.f;
	float death_duration_ms = 0.f;
	int health = 2;
	float wizard_curr_frame = 0.f;
	int honor = 3;
	// for new animation, mod sheetsize, state_map, state_curr, rowMax, colMax, getCurrState accordingly
	vec2 sheetsize = { 2048.f, 768.f };

	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 8} },
		{ "walk", {1, 7} },
		{ "dead", {2, 4} },
		{ "attack2", {3, 9} },
		{ "attack1", {4, 16} },
		{ "hurt", {5, 4} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true },
		{ "walk", false },
		{ "dead", false },
		{ "attack1", false },
		{ "attack2", false },
		{ "hurt", false }
	};

	glm::uint rowMax = 16;
	glm::uint colMax = 6;
	glm::uint state_index = 1;
	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		if (state_curr["dead"]) return "dead";
		if (state_curr["hurt"]) return "hurt";
		if (state_curr["attack1"]) return "attack1";
		if (state_curr["attack2"]) return "attack2";

		return "idle";
	}

	// for jump detection reset index may be problematic
	// invalid action is ignored
	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;

			if (value) {
				if (action == "attack1") attack_duration_ms = 1600.f;
				if (action == "attack2") attack_duration_ms = 900.f;
				if (action == "dead") {
					death_duration_ms = 400.f;
					attack_delay_ms = 0;
					attack_duration_ms = 0;
				}
				if (action == "hurt") {
					hurt_duration_ms = 400.f;
					attack_delay_ms = 0;
					attack_duration_ms = 0;
				}
			}
		}
	}

	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct MagicBall1 {
	// fireball will have direction, speed, position, health, duration before disappearing
	vec2 initialPos;
	float render_update_frame = 100.f;
	float render_curr_frame = 0.f;
	float duration_ms = 900.f;
	int damage = 1;

	vec2 sheetsize = { 576.f, 128.f };

	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "fly", {0, 9} },
	};

	std::map<std::string, bool> state_curr =
	{
		{ "fly", true },
	};

	glm::uint rowMax = 9;
	glm::uint colMax = 1;
	glm::uint state_index = 0;
	bool stateChange = true;
	std::string getCurrState()
	{
		return "fly";
	}
	// for jump detection reset index may be problematic
	// invalid action is ignored
	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 0;
		}
		else
		{
			state_index++;
		}
	}
};

struct MagicBall2 {
	// fireball will have direction, speed, position, health, duration before disappearing
	vec2 initialPos;
	int damage = 1;
	float render_update_frame = 100.f;
	float render_curr_frame = 0.f;
	float duration_ms = 600.f;

	vec2 sheetsize = { 384.f, 128.f };

	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "fly", {0, 6} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "fly", true }
	};

	glm::uint rowMax = 6;
	glm::uint colMax = 1;
	glm::uint state_index = 0;
	bool stateChange = true;
	std::string getCurrState()
	{
		return "fly";
	}

	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 0;
		}
		else
		{
			state_index++;
		}
	}
};

struct BatEnemy {
	// limit of distance of flying before changin direction
	vec2 initialPos;
	float flyRange;
	// amount of damage the enem will inflict upon collision with player
	int damage = 1;
	int health = 1;
	int honor = 1;
	float immunity_duration_ms = 500.f;
	float bat_curr_frame = 0.f;
	// for new animation, mod sheetsize, state_map, state_curr, rowMax, colMax, getCurrState accordingly
	vec2 sheetsize = { 900.f, 150.f };

	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 6} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true }
	};

	glm::uint rowMax = 6;
	glm::uint colMax = 1;
	glm::uint state_index = 1;
	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		return "idle";
	}

	// for jump detection reset index may be problematic
	// invalid action is ignored
	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}
	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct SkeletonEnemy {
	vec2 initialPos;
	float attackRange;
	float idleSpeed = 100.f;
	float aggroSpeed = 200.f;
	float roamRange;
	int damage = 1;
	int health = 2;
	int honor = 1;
	float immunity_duration_ms = 500.f;
	float skeleton_curr_frame = 0.f;
	// for new animation, mod sheetsize, state_map, state_curr, rowMax, colMax, getCurrState accordingly
	vec2 sheetsize = { 602.f, 152.f };

	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 4} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true }
	};
	
	glm::uint rowMax = 4;
	glm::uint colMax = 1;
	glm::uint state_index = 1;
	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		return "idle";
	}

	// for jump detection reset index may be problematic
	// invalid action is ignored
	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}
	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct GhostEnemy {
	vec2 initialPos;
	float attackRange;
	float idleSpeed = 100.f;
	float aggroSpeed = 200.f;
	float roamRange;
	int damage = 1;
	int health = 1;
	float skeleton_curr_frame = 0.f;
	// for new animation, mod sheetsize, state_map, state_curr, rowMax, colMax, getCurrState accordingly
	vec2 sheetsize = { 602.f, 152.f };

	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 4} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true }
	};

	glm::uint rowMax = 4;
	glm::uint colMax = 1;
	glm::uint state_index = 1;
	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		return "idle";
	}

	// for jump detection reset index may be problematic
	// invalid action is ignored
	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}
	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct EnergyProjectile {
	int damage = 1;
	float render_update_frame = 100.f;
	float render_curr_frame = 0.f;
	float duration_ms = 5000.f;

	vec2 sheetsize = { 1020.f, 34.f };
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 30} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true }
	};

	glm::uint rowMax = 30;
	glm::uint colMax = 1;
	glm::uint state_index = 1;
	bool stateChange = true;

	std::string getCurrState()
	{
		return "idle";
	}

	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}

	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct ArmProjectile {

	float duration_ms = 5000.f;
	int health = 1;
	int damage = 1;
	float speed = 10;
	float render_update_frame = 100.f;
	float render_curr_frame = 0.f;
	vec2 sheetsize = { 46.f, 36.f };
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 1} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true }
	};

	glm::uint rowMax = 1;
	glm::uint colMax = 1;
	glm::uint state_index = 1;
	bool stateChange = true;

	std::string getCurrState()
	{
		return "idle";
	}

	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}

	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct Golem {
	float deathDuration = 4000.f;
	
	bool engaged = false;
	int burstCount = 0;
	float burst_cd = 200.f;
	bool alive = true;
	int energyBurstCount = 0;
	float energyBurst_cd = 150.f;
	float bob_cd = 0.f;
	bool up = true;
	float attack_range = 1000.f;
	int health = 15;
	int damage = 1;
	float energy_attack_cd_ms = 4000.f;
	float energy_attack_curr_cd_ms = 2000.f;

	float arm_projectile_cd_ms = 2000.f;
	float arm_projectile_curr_cd_ms = 0;

	float immunity_reset_duration = 250.f;
	// if immunity_duration > 0.f, can't take damage from player
	float immunity_duration = 0.f;

	float render_update_frame = 100.f;
	float render_curr_frame = 0.f;

	vec2 sheetsize = { 995.f, 910.f };

	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
	{ "idle", {1, 8} },
	{ "projectile attack", {2, 9} },
	{ "energy attack", {5, 7} },
	{ "melee", {4,7} },
	{ "dead", {7, 10} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true },
		{ "projectile attack", false },
		{ "energy attack", false },
		{ "melee", false},
		{ "dead", false }
	};

	glm::uint rowMax = 10;
	glm::uint colMax = 9;
	glm::uint state_index = 1;
	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		if (state_curr["dead"]) return "dead";
		if (state_curr["projectile attack"]) return "projectile attack";
		if (state_curr["energy attack"]) return "energy attack";
		if (state_curr["melee"]) return "melee";
		return "idle";
	}

	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}

	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			if (getCurrState() == "dead") return;
			if (getCurrState() == "projectile attack")
			{
				changeState("projectile attack", false);
			}
			if (getCurrState() == "energy attack")
			{
				changeState("energy attack", false);
			}
			if (getCurrState() == "melee")
			{
				changeState("melee", false);
			}
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct Demon {
	float idleSpeed = 3.0f;
	float boundLeft;
	float boundRight;

	int damage = 1;
	int health = 5;
	int honor = 10;
	float attack_range = 200.f;

	// attack_curr_cd == 0 -> attack animation start
	float attack_cd_ms = 400.f;
	float attack_curr_cd_ms = attack_cd_ms;

	// attack_actual_timer == 0 -> actual attack finishes. actual attack begins at 250ms, see step()
	float attack_actual_reset_timer = 1000.f;
	float attack_actual_timer = -10.f;

	float immunity_reset_duration = 250.f;
	// if immunity_duration > 0.f, can't take damage from player
	float immunity_duration = 0.f;

	// start walking when walk_timer hits 0
	float walk_reset_timer = 500.f;
	float walk_timer = walk_reset_timer;

	float render_update_frame = 80.f;
	float render_curr_frame = 0.f;
	// for new animation, mod sheetsize, state_map, state_curr, rowMax, colMax, getCurrState accordingly
	vec2 sheetsize = { 6336.f, 800.f };

	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 6} },
		{ "walk", {1, 12} },
		{ "attack", {2, 15} },
		{ "roar", {3, 5} },
		{ "dead", {4, 22} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true },
		{ "walk", false },
		{ "attack", false },
		{ "roar", false },
		{ "dead", false }
	};

	glm::uint rowMax = 22;
	glm::uint colMax = 5;
	glm::uint state_index = 1;
	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		if (state_curr["dead"]) return "dead";
		if (state_curr["attack"]) return "attack";
		if (state_curr["walk"]) return "walk";
		if (state_curr["roar"]) return "roar";
		return "idle";
	}

	// for jump detection reset index may be problematic
	// invalid action is ignored
	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}
	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			if (getCurrState() == "dead") return;
			if (getCurrState() == "attack")
			{
				changeState("attack", false);
			}
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct WolfEnemy {
	vec2 initialPos;
	float attackRange;
	float idleSpeed = 200.f;
	float aggroSpeed = 300.f;
	float roamRange;
	int damage = 1;
	int health = 1;
	int honor = 2;
	float jump_cd;
	float set_jump_cd;
	float jump_speed = 100;
	float wolf_curr_frame = 0.f;
	float immunity_duration_ms = 500.f;
	float render_update_frame = 50.f;
	float render_curr_frame = render_update_frame;
	// for new animation, mod sheetsize, state_map, state_curr, rowMax, colMax, getCurrState accordingly
	vec2 sheetsize = { 382.f, 47.f };

	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 6} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true }
	};

	glm::uint rowMax = 6;
	glm::uint colMax = 1;
	glm::uint state_index = 1;

	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		return "idle";
	}

	// for jump detection reset index may be problematic
	// invalid action is ignored
	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}
	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct Heart {
	vec2 relative_pos = { 0, 0 };

	vec2 sheetsize = { 48.f, 16.f };
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 3} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true } 
	};

	glm::uint rowMax = 3;
	glm::uint colMax = 1;
	glm::uint state_index = 1;
	bool stateChange = true;

	std::string getCurrState()
	{
		return "idle";
	}

	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}

	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

struct Shield {
	vec2 relative_pos = { 0, 0 };

	vec2 sheetsize = { 48.f, 16.f };
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "idle", {0, 3} }
	};

	std::map<std::string, bool> state_curr =
	{
		{ "idle", true }
	};

	glm::uint rowMax = 3;
	glm::uint colMax = 1;
	glm::uint state_index = 1;
	bool stateChange = true;

	std::string getCurrState()
	{
		return "idle";
	}

	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}

	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

// Player component
struct Player
{	
	bool alive = true;
	float camera_x;
	float camera_y;

	float immunity_duration_ms = 1000.f;
	int health = 3;
	int shield = 0;
	int damage = 1;
	// for new animation, mod sheetsize, state_map, state_curr, rowMax, colMax, getCurrState accordingly
	vec2 sheetsize = { 1442.f, 738.f };
	// state name, [col number(vertical offset), row number(horizontal sheet size)]
	std::unordered_map<std::string, std::pair<glm::uint, glm::uint>> state_map = {
		{ "run", {8, 10} },
		{ "roll", {7, 12} },
		{ "jump", {6, 3} },
		{ "idle", {5, 10} },
		{ "hit", {4, 1} },
		{ "fall", {3, 3} },
		{ "death", {2, 10} },
		{ "attack2", {1, 4} },
		{ "attack1", {0, 6} }
	};

	// curr state, can exist multiple, but only render the prioritized one, see GETTER
	// e.g jump is the most prioritized
	std::map<std::string, bool> state_curr = 
	{
		{ "idle", true }, // should leave this always be true
		{ "run", false },
		{ "roll", false },
		{ "hit", false },
		{ "death", false },
		{ "jump", false },
		{ "fall", false },
		{ "attack1", false },
		{ "attack2", false },
	};
	glm::uint rowMax = 12;
	glm::uint colMax = 9;
	glm::uint state_index = 1;
	bool stateChange = true;
	std::string getCurrState()
	{
		// GETTER, will prioritize the first state if there're multiple
		// TODO, render priority can be optimized depends on actual gameplay
		if (state_curr["death"]) return "death";
		if (state_curr["attack1"]) return "attack1";
		if (state_curr["attack2"]) return "attack2";
		if (state_curr["roll"]) return "roll";
		if (state_curr["fall"]) return "fall";
		if (state_curr["jump"]) return "jump";
		if (state_curr["hit"]) return "hit";
		if (state_curr["run"]) return "run";

		return "idle";
	}

	// for jump detection reset index may be problematic
	// invalid action is ignored
	void changeState(const std::string& action, bool value, bool resetIndex = true)
	{
		if (state_curr.count(action))
		{
			state_curr[action] = value;
			if (resetIndex)
			{
				state_index = 1;
			}
			stateChange = true;
		}
	}
	void incrementStateIndex()
	{
		if (state_index >= state_map.at(getCurrState()).second)
		{
			// reset the index
			state_index = 1;
		}
		else
		{
			state_index++;
		}
	}
};

// Leftover from A1
struct Deadly
{

};

// Leftover from A1
struct Eatable
{

};

// Background
struct Background
{

};

// Tile
struct Tile
{

};

// Invisible attack1 obj
struct Attack1
{

};

// Invisible attack2 obj
struct Attack2
{

};

// All data relevant to the shape and motion of entities
struct Motion {
	bool interactable = false;
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 velocity = { 0, 0 };
	vec2 scale = { 10, 10 };
	char attackDirection = 'n'; // n for nothing, w for up, a for left, d for right, s for down
	bool isJumping = 0;
	bool isFalling = 0;
	bool touchingWall = 0;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

struct Deathbox{

};

struct nextLevel {

};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

struct AttackPathTimer
{
	float counter_ms = 2000;
};

// A timer that will be associated to dying player
struct DeathTimer
{
	float counter_ms = 4000;
};

struct NextLevelTimer
{
	float counter_ms = 2000;
};

struct RollTimer
{
	float counter_ms = 600;
};

// String of text
struct Text {
	std::string str;
	vec3 color;
	mat4 trans;
	bool fixed = false;
};

// Character for font
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};


// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & player.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	BAT = 0,
	BACKGROUND = BAT + 1,
	BACKGROUND_CLOUD = BACKGROUND + 1,
	BACKGROUND_TREE = BACKGROUND_CLOUD + 1,
	TILE = BACKGROUND_TREE + 1,
	FIREBALL = TILE + 1,
	PLAYER_SHEET = FIREBALL + 1,
	STATUS_EFFECT = PLAYER_SHEET + 1,
	HEART_SHEET = STATUS_EFFECT + 1,
	SHIELD_SHEET = HEART_SHEET + 1,
	SAW = SHIELD_SHEET + 1,
	MAGICBALL1 = SAW + 1,
	MAGICBALL2 = MAGICBALL1 + 1,
	SKELETON_IDLE = MAGICBALL2 + 1,
	MUSHROOM = SKELETON_IDLE + 1,
	WIZARD = MUSHROOM + 1,
	DEMON = WIZARD + 1,
	DOOR = DEMON + 1,
	STATUE = DOOR + 1,
	ATTACK_BUFF = STATUE + 1,
	DEFENSE_BUFF = ATTACK_BUFF + 1,
	PEDESTAL = DEFENSE_BUFF + 1,
	TILE_VERT = PEDESTAL + 1,
	TILE_VERT_LONG = TILE_VERT + 1,
	TILE_VERT_LONG_THICK = TILE_VERT_LONG + 1,
	INTRO = TILE_VERT_LONG_THICK + 1,
	COMPLETE = INTRO + 1,
	GAMEOVER = COMPLETE + 1,
	GHOST = GAMEOVER + 1,
	WOLF = GHOST + 1,
	CUTSCENE1A = WOLF + 1,
	CUTSCENE1B = CUTSCENE1A + 1,
	CUTSCENE1C = CUTSCENE1B + 1,
	CUTSCENE1D = CUTSCENE1C + 1,
	CUTSCENE1E = CUTSCENE1D + 1,
	CUTSCENE2A = CUTSCENE1E + 1,
	CUTSCENE2B = CUTSCENE2A + 1,
	CUTSCENE2C = CUTSCENE2B + 1,
	CUTSCENE3A = CUTSCENE2C + 1,
	CUTSCENE3B = CUTSCENE3A + 1,
	CUTSCENE3C = CUTSCENE3B + 1,
	CUTSCENE3D = CUTSCENE3C + 1,
	BACKGROUND2 = CUTSCENE3D + 1,
	BACKGROUND3 = BACKGROUND2 + 1,
	GOLEM = BACKGROUND3 + 1,
	ARMPROJECTILE = GOLEM + 1,
	ENERGYPROJECTILE = ARMPROJECTILE + 1,
	TEXTURE_COUNT = ENERGYPROJECTILE + 1

};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	PLAYER = EGG + 1,
	TEXTURED = PLAYER + 1,
	TEXTUREDFIXED = TEXTURED + 1,
	WIND = TEXTUREDFIXED + 1,
	EFFECT_COUNT = WIND + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	PLAYER = 0,
	PLAYER_ATTACK1 = PLAYER + 1,
	PLAYER_ATTACK2 = PLAYER_ATTACK1 + 1,
	SPRITE = PLAYER_ATTACK2 + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	PLAYER_HEART = DEBUG_LINE + 1,
	PLAYER_SHIELD = PLAYER_HEART + 1,
	SAW = PLAYER_SHIELD + 1,
	MAGICBALL1 = SAW + 1,
	MAGICBALL2 = MAGICBALL1 + 1,
	SKELETON_ENEMY = MAGICBALL2 + 1,
	BAT_ENEMY = SKELETON_ENEMY + 1,
	MUSHROOM_ENEMY = BAT_ENEMY + 1,
	WIZARD_ENEMY = MUSHROOM_ENEMY + 1,
	DEMON_ENEMY = WIZARD_ENEMY + 1,
	SCREEN_TRIANGLE = DEMON_ENEMY + 1,
	GHOST_ENEMY = SCREEN_TRIANGLE + 1,
	WOLF_ENEMY = GHOST_ENEMY + 1,
	GOLEM_ENEMY = WOLF_ENEMY + 1,
	ARMPROJECTILE_ENEMY = GOLEM_ENEMY + 1,
	ENERGYPROJECTILE_ENEMY = ARMPROJECTILE_ENEMY + 1,
	GEOMETRY_COUNT = ENERGYPROJECTILE_ENEMY + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

