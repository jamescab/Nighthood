#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<NextLevelTimer> nextLevelTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Eatable> eatables;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Background> background;
	ComponentContainer<Tile> tiles;
	ComponentContainer<Deathbox> deathboxes;
	ComponentContainer<nextLevel> nextLevels;
	ComponentContainer<Potion> potions;
	ComponentContainer<BatEnemy> batEnemy;
	ComponentContainer<SkeletonEnemy> skeletonEnemy;
	ComponentContainer<Demon> demonBoss;
	ComponentContainer<WolfEnemy> wolfEnemy;
	ComponentContainer<Attack1> player_attack1;
	ComponentContainer<Attack2> player_attack2;
	ComponentContainer<Heart> player_health;
	ComponentContainer<Readable> readables;
	ComponentContainer<GhostEnemy> ghostEnemy;
	ComponentContainer<Buff> buffs;
	ComponentContainer<Pedestal> pedestals;
	ComponentContainer<Door> doors;
	ComponentContainer<Saw> saws;
	ComponentContainer<Text> texts;
	ComponentContainer<RollTimer> rollTimers;
	ComponentContainer<RangedEnemy> rangedEnemy;
	ComponentContainer<FireBall> fireBalls;
	ComponentContainer<AttackPathTimer> attackPathTimers;
	ComponentContainer<Shield> shields;
	ComponentContainer<ArmProjectile> armProjectile;
	ComponentContainer<EnergyProjectile> energyProjectile;
	ComponentContainer<Golem> golem;
	ComponentContainer<Wizard> wizards;
	ComponentContainer<MagicBall1> magicBalls1;
	ComponentContainer<MagicBall2> magicBalls2;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&nextLevelTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&eatables);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&background);
		registry_list.push_back(&tiles);
		registry_list.push_back(&deathboxes);
		registry_list.push_back(&nextLevels);
		registry_list.push_back(&potions);
		registry_list.push_back(&batEnemy);
		registry_list.push_back(&skeletonEnemy);
		registry_list.push_back(&demonBoss);
		registry_list.push_back(&wolfEnemy);
		registry_list.push_back(&doors);
		registry_list.push_back(&saws);
		registry_list.push_back(&player_health);
		registry_list.push_back(&player_attack1);
		registry_list.push_back(&player_attack2);
		registry_list.push_back(&texts);
		registry_list.push_back(&rollTimers);
		registry_list.push_back(&rangedEnemy);
		registry_list.push_back(&fireBalls);
		registry_list.push_back(&attackPathTimers);
		registry_list.push_back(&readables);
		registry_list.push_back(&ghostEnemy);
		registry_list.push_back(&buffs);
		registry_list.push_back(&pedestals);
		registry_list.push_back(&shields);
		registry_list.push_back(&armProjectile);
		registry_list.push_back(&energyProjectile);
		registry_list.push_back(&golem);
		registry_list.push_back(&wizards);
		registry_list.push_back(&magicBalls1);
		registry_list.push_back(&magicBalls2);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;