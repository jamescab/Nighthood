// internal
#include "physics_system.hpp"
#include "world_init.hpp"
// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// AABB collision
bool collides(const Motion& motion1, const Motion& motion2)
{
	auto isPointInBox = [&](const Motion& bbox, float x, float y) {
		if (((bbox.position.x + (abs(bbox.scale.x) / 2.0f)) >= x &&
			(bbox.position.x - (abs(bbox.scale.x) / 2.0f)) <= x) &&
			(bbox.position.y + (abs(bbox.scale.y) / 2.0f) >= y && 
			(bbox.position.y - (abs(bbox.scale.y) / 2.0f)) <= y))
			return true;
		return false;
	};
	if (isPointInBox(motion1, motion2.position.x + (abs(motion2.scale.x) / 2.0f), motion2.position.y + (abs(motion2.scale.y) / 2.0f)) ||
		isPointInBox(motion1, motion2.position.x + (abs(motion2.scale.x) / 2.0f), motion2.position.y - (abs(motion2.scale.y) / 2.0f)) ||
		isPointInBox(motion1, motion2.position.x - (abs(motion2.scale.x) / 2.0f), motion2.position.y + (abs(motion2.scale.y) / 2.0f)) ||
		isPointInBox(motion1, motion2.position.x - (abs(motion2.scale.x) / 2.0f), motion2.position.y - (abs(motion2.scale.y) / 2.0f)))
	{
		return true;
	}
	// neccessary for overlap case
	if (isPointInBox(motion2, motion1.position.x + (abs(motion1.scale.x) / 2.0f), motion1.position.y + (abs(motion1.scale.y) / 2.0f)) ||
		isPointInBox(motion2, motion1.position.x + (abs(motion1.scale.x) / 2.0f), motion1.position.y - (abs(motion1.scale.y) / 2.0f)) ||
		isPointInBox(motion2, motion1.position.x - (abs(motion1.scale.x) / 2.0f), motion1.position.y + (abs(motion1.scale.y) / 2.0f)) ||
		isPointInBox(motion2, motion1.position.x - (abs(motion1.scale.x) / 2.0f), motion1.position.y - (abs(motion1.scale.y) / 2.0f)))
	{
		return true;
	}
	return false;
}

// mesh box collision
bool collidesMeshBox(const Motion& mesh, const Motion& box, Entity meshE)
{
	// Here we assume we already check AABB collision (caller's responsibility)
	if (!registry.meshPtrs.has(meshE)) return false;
	const float offset = 185.f;
	const float sizeOffset_x = registry.motions.get(meshE).scale.x > 0 ? offset : -offset;
	const float sizeOffset_y = 115.f;
	const float& posX = mesh.position.x;
	const float& posY = mesh.position.y;
	auto isPointInBox = [&](const Motion& bbox, float x, float y) {
		if (((bbox.position.x + (abs(bbox.scale.x) / 2.0f)) >= x &&
			(bbox.position.x - (abs(bbox.scale.x) / 2.0f)) <= x) &&
			(bbox.position.y + (abs(bbox.scale.y) / 2.0f) >= y &&
			(bbox.position.y - (abs(bbox.scale.y) / 2.0f)) <= y))
			return true;
		return false;
	};
	for (uint i = 0; i < registry.meshPtrs.get(meshE)->vertices.size(); i += 25)
	{
		vec2 curr = {
			posX + registry.meshPtrs.get(meshE)->vertices[i].position.x * sizeOffset_x,
			posY - registry.meshPtrs.get(meshE)->vertices[i].position.y * sizeOffset_y 
		};
		if (isPointInBox(box, curr.x, curr.y))
			return true;
	}
	return false;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	
	if (registry.players.size() > 0) {
		auto& player = registry.motions.get(registry.players.entities[0]);


		std::set<unsigned int> exclusion;
		for (Entity e : registry.tiles.entities)
		{
			exclusion.insert(e);
		}
		for (Entity e : registry.background.entities)
		{
			exclusion.insert(e);
		}
		for (uint i = 0; i < motion_registry.size(); i++)
		{
			if (exclusion.count(motion_registry.entities[i]))
				continue;

			//update player movement
			if (registry.players.has(motion_registry.entities[i])) {
				Motion& motion = motion_registry.components[i];
				Entity entity = motion_registry.entities[i];
				float step_seconds = elapsed_ms / 1000.f;

				// update immunity duration
				if (registry.players.get(entity).immunity_duration_ms > 0) {
					registry.players.get(entity).immunity_duration_ms -= elapsed_ms;
				}
				motion.position.x += (motion.velocity.x * step_seconds);

				// Jump/gravity
				if (motion.velocity.y < 0 && motion.isJumping == 0) {
					motion.isJumping = 1;
				}
				motion.position.y += motion.velocity.y;
				if (motion.isJumping = 1 && motion.velocity.y > 0) {
					motion.isFalling = 1;
				}
			}
			// update NPC (enemy) movement
			else {
				float step_seconds = elapsed_ms / 1000.f;

				//update arm projectile
				if (registry.armProjectile.has(motion_registry.entities[i]))
				{
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];

					motion.position += motion.velocity * step_seconds;
				}

				//update energy projectile
				if (registry.energyProjectile.has(motion_registry.entities[i]))
				{
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];

					motion.position += motion.velocity * step_seconds;
				}

				//update magicBalls1
				if (registry.magicBalls1.has(motion_registry.entities[i]))
				{
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];

					motion.position += motion.velocity * step_seconds;
				}

				//update magicBalls2
				if (registry.magicBalls2.has(motion_registry.entities[i]))
				{
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];

					motion.position += motion.velocity * step_seconds;
				}

				//update golem
				if (registry.golem.has(motion_registry.entities[i])) {
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];

					if (registry.golem.get(entity).health <= 0) {
						motion.velocity.y = -500;
					}
					else {
						if (registry.golem.get(entity).immunity_duration > 0.f)
						{
							registry.golem.get(entity).immunity_duration -= elapsed_ms;
						}

						if (registry.golem.get(entity).bob_cd > 0.f)
						{
							registry.golem.get(entity).bob_cd -= elapsed_ms;
						}

						if (registry.golem.get(entity).up && registry.golem.get(entity).bob_cd <= 0) {
							motion.velocity.y = -5;
							registry.golem.get(entity).bob_cd = 250.f;
							registry.golem.get(entity).up = false;
						}
						else if (!registry.golem.get(entity).up && registry.golem.get(entity).bob_cd <= 0) {
							motion.velocity.y = 5;
							registry.golem.get(entity).bob_cd = 250.f;
							registry.golem.get(entity).up = true;
						}

						// direction check
						if (pow(player.position.x - motion.position.x, 2) < pow(registry.golem.get(entity).attack_range, 2))
						{
							registry.golem.get(entity).engaged = true;
							if (player.position.x < registry.motions.get(entity).position.x)
							{
								registry.motions.get(entity).scale.x =
									-abs(registry.motions.get(entity).scale.x);
							}
							else if (player.position.x > registry.motions.get(entity).position.x)
							{
								registry.motions.get(entity).scale.x =
									abs(registry.motions.get(entity).scale.x);
							}
						}
						else {
							registry.golem.get(entity).engaged = false;
						}
					}
					motion.position += motion.velocity*step_seconds;
				}

				// update Ghost enemies
				if (registry.ghostEnemy.has(motion_registry.entities[i])) {
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];

					//based on player position vs ghost position
					//player is left ghost
					if (player.position.x < (motion.position.x - 25)) {
						motion.position.x -= (motion.velocity.x * step_seconds);
						motion.angle = 0;
					}
					//player is right of ghost
					else if (player.position.x > (motion.position.x + 25)) {
						motion.position.x += (motion.velocity.x * step_seconds);
						motion.angle = 135;
					}
					//player is above ghost
					if (player.position.y < (motion.position.y - 25)) {
						motion.position.y -= (motion.velocity.y * step_seconds);
					}
					else if (player.position.y > (motion.position.y + 25)) {
						motion.position.y += (motion.velocity.y * step_seconds);
					}

					if (sqrt(pow(player.position.x - motion.position.x, 2) + pow(player.position.y - motion.position.y, 2)) > 1000) {

						int randomX;
						int randomY;
						// 50% chance either between 600 to 750 or -600 to -750
						if (rand() % 2 == 0) {
							//between 600 to 750
							randomX = ((rand() % 151) + 600);
						}
						else {
							//between -600 to -750
							randomX = -((rand() % 151) + 600);
						}
						if (rand() % 2 == 0) {
							//between 600 to 750
							randomY = ((rand() % 151) + 600);
						}
						else {
							//between -600 to -750
							randomY = -((rand() % 151) + 600);
						}
						motion.position.y = player.position.y + randomY;
						motion.position.x = player.position.x + randomX;
					}
				}

				// update wolf
				if (registry.wolfEnemy.has(motion_registry.entities[i])) {
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];
					float leftRoamLimit = registry.wolfEnemy.get(entity).initialPos.x - registry.wolfEnemy.get(entity).roamRange;
					float rightRoamLimit = registry.wolfEnemy.get(entity).initialPos.x + registry.wolfEnemy.get(entity).roamRange;

					// update immunity duration
					if (registry.wolfEnemy.get(entity).immunity_duration_ms > 0) {
						registry.wolfEnemy.get(entity).immunity_duration_ms -= elapsed_ms;
					}

					// check if skeleton is still within its bounds (roaming range)
					if (motion.position.x > leftRoamLimit && motion.position.x < rightRoamLimit) {
						if (motion.velocity.x > 0) {
							motion.velocity.x = registry.wolfEnemy.get(entity).aggroSpeed;
						}
						else if (motion.velocity.x < 0) {
							motion.velocity.x = -registry.wolfEnemy.get(entity).aggroSpeed;
						}
					}
					// flip velocity when reached the side of roaming limit
					else if (motion.position.x < leftRoamLimit) {
						motion.velocity.x *= -1;
						motion.angle = 0;
						motion.position.x = leftRoamLimit + (motion.velocity.x * step_seconds);
					}
					else if (motion.position.x > rightRoamLimit) {
						motion.velocity.x *= -1;
						motion.angle = 135;
						motion.position.x = rightRoamLimit + (motion.velocity.x * step_seconds);
					}

					if (motion.position.y >= registry.wolfEnemy.get(entity).initialPos.y) {
						if (registry.wolfEnemy.get(entity).jump_cd > 0) {
							registry.wolfEnemy.get(entity).jump_cd -= 50;
						}
					}

					if (registry.wolfEnemy.get(entity).jump_cd <= 0) {
						motion.velocity.y = -300;
						registry.wolfEnemy.get(entity).jump_cd = registry.wolfEnemy.get(entity).set_jump_cd;
					}

					if (motion.position.y < registry.wolfEnemy.get(entity).initialPos.y) {
						motion.velocity.y += 10;
					}

					if (motion.position.y >= registry.wolfEnemy.get(entity).initialPos.y) {
						motion.position.y = registry.wolfEnemy.get(entity).initialPos.y;
					}

					// update x position
					motion.position.x += (motion.velocity.x * step_seconds);
					motion.position.y += (motion.velocity.y * step_seconds);
				}

				// update Bat enemies
				if (registry.batEnemy.has(motion_registry.entities[i])) {
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];

					// update immunity duration
					if (registry.batEnemy.get(entity).immunity_duration_ms > 0) {
						registry.batEnemy.get(entity).immunity_duration_ms -= elapsed_ms;
					}

					float TopPos = registry.batEnemy.get(entity).initialPos.y - registry.batEnemy.get(entity).flyRange;
					float BottomPos = registry.batEnemy.get(entity).initialPos.y + registry.batEnemy.get(entity).flyRange;
					if (motion.position.y > BottomPos) {
						motion.velocity.y *= -1;
						motion.position.y = BottomPos;
						motion.position.y += (motion.velocity.y * step_seconds);
					}
					else if (motion.position.y < TopPos) {
						motion.velocity.y *= -1;
						motion.position.y = TopPos;
						motion.position.y += (motion.velocity.y * step_seconds);
					}
					motion.position.y += (motion.velocity.y * step_seconds);
				}

				// update fireball
				if (registry.fireBalls.has(motion_registry.entities[i])) {
					Entity entity = motion_registry.entities[i];
					Motion& motion = motion_registry.components[i];

					motion.velocity.y += 10;
					motion.position.x += (motion.velocity.x * step_seconds);
					motion.position.y += (motion.velocity.y * step_seconds);
				}

				// update RangedEnemy enemies (decision tree)
				if (registry.rangedEnemy.has(motion_registry.entities[i])) {
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];
					float leftRoamLimit = registry.rangedEnemy.get(entity).initialPos.x - registry.rangedEnemy.get(entity).roamRange;
					float rightRoamLimit = registry.rangedEnemy.get(entity).initialPos.x + registry.rangedEnemy.get(entity).roamRange;

					// update immunity duration
					if (registry.rangedEnemy.get(entity).immunity_duration_ms > 0) {
						registry.rangedEnemy.get(entity).immunity_duration_ms -= elapsed_ms;
					}

					//LOGIC TO SPAWN FIREBALL IS LOCATED IN WORLD_SYSTEM.CPP BECAUSE NEED TO USE RENDERER TO CREATE FIREBALL

					// IF ranged enemy is stationary skip all movement update
					// check if skeleton is still within its bounds (roaming range)
					if (registry.rangedEnemy.get(entity).stationary) {
						if ((pow(player.position.x - motion.position.x, 2) < pow(registry.rangedEnemy.get(entity).attackRange, 2))) {

							if (player.position.x < motion.position.x) {
								motion.angle = 135;
								motion.velocity.x = 0;
							}

							else if (player.position.x > motion.position.x) {
								motion.angle = 0;
								motion.velocity.x = 0;
							}


						}
					}
					else {
						if (motion.position.x > leftRoamLimit && motion.position.x < rightRoamLimit) {
							// check if player is within skeleton engage range

							if ((pow(player.position.x - motion.position.x, 2) < pow(registry.rangedEnemy.get(entity).attackRange, 2))) {

								if (player.position.x < motion.position.x) {
									motion.angle = 135;
									motion.velocity.x = 0;
								}

								else if (player.position.x > motion.position.x) {
									motion.angle = 0;
									motion.velocity.x = 0;
								}

							}
							// player is not within range of skeleton engage range
							else {
								// ranged enemy will walk away from player
								if (player.position.x < motion.position.x) {
									motion.velocity.x = registry.rangedEnemy.get(entity).idleSpeed;
								}
								else if (player.position.x > motion.position.x) {
									motion.velocity.x = -registry.rangedEnemy.get(entity).idleSpeed;
								}
							}
						}
						// flip velocity when reached the side of roaming limit
						else if (motion.position.x < leftRoamLimit) {
							motion.velocity.x *= -1;
							motion.angle = 0;
							motion.position.x = leftRoamLimit + (motion.velocity.x * step_seconds);
						}
						else if (motion.position.x > rightRoamLimit) {
							motion.velocity.x *= -1;
							motion.angle = 135;
							motion.position.x = rightRoamLimit + (motion.velocity.x * step_seconds);
						}
						// update x position
						motion.position.x += (motion.velocity.x * step_seconds);
					}
				}

				// update Wizards enemies (decision tree)
				if (registry.wizards.has(motion_registry.entities[i])) {
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];
					float leftRoamLimit = registry.wizards.get(entity).initialPos.x - registry.wizards.get(entity).roamRange;
					float rightRoamLimit = registry.wizards.get(entity).initialPos.x + registry.wizards.get(entity).roamRange;


					// update immunity duration
					if (registry.wizards.get(entity).immunity_duration_ms > 0) {
						registry.wizards.get(entity).immunity_duration_ms -= elapsed_ms;
					}

					//LOGIC TO SPAWN FIREBALL IS LOCATED IN WORLD_SYSTEM.CPP BECAUSE NEED TO USE RENDERER TO CREATE FIREBALL

					// If wizard is stationary skip all movement update
					// check if skeleton is still within its bounds (roaming range)
					if (registry.wizards.get(entity).stationary) {
						if ((pow(player.position.x - motion.position.x, 2) < pow(registry.wizards.get(entity).attackRange, 2))) {
							Wizard& wizard = registry.wizards.get(motion_registry.entities[i]);
							if (player.position.x < motion.position.x) {
								motion.angle = 135;
								motion.velocity.x = 0;
							}

							else if (player.position.x > motion.position.x) {
								motion.angle = 0;
								motion.velocity.x = 0;
							}
						}
					}
					else {
						if (motion.position.x > leftRoamLimit && motion.position.x < rightRoamLimit) {
							// check if player is within skeleton engage range

							if ((pow(player.position.x - motion.position.x, 2) < pow(registry.wizards.get(entity).attackRange, 2))) {

								if (player.position.x < motion.position.x) {
									motion.angle = 135;
									motion.velocity.x = 0;
								}

								else if (player.position.x > motion.position.x) {
									motion.angle = 0;
									motion.velocity.x = 0;
								}

							}
							// player is not within range of skeleton engage range
							else {
								// ranged enemy will walk away from player
								if (player.position.x < motion.position.x) {
									motion.velocity.x = registry.wizards.get(entity).idleSpeed;
								}
								else if (player.position.x > motion.position.x) {
									motion.velocity.x = -registry.wizards.get(entity).idleSpeed;
								}
							}
						}
						// flip velocity when reached the side of roaming limit
						else if (motion.position.x < leftRoamLimit) {
							motion.velocity.x *= -1;
							motion.angle = 0;
							motion.position.x = leftRoamLimit + (motion.velocity.x * step_seconds);
						}
						else if (motion.position.x > rightRoamLimit) {
							motion.velocity.x *= -1;
							motion.angle = 135;
							motion.position.x = rightRoamLimit + (motion.velocity.x * step_seconds);
						}
						// update x position
						motion.position.x += (motion.velocity.x * step_seconds);
					}
				}

				// update Skeleton enemies (decision tree)
				if (registry.skeletonEnemy.has(motion_registry.entities[i])) {
					Motion& motion = motion_registry.components[i];
					Entity entity = motion_registry.entities[i];
					float leftRoamLimit = registry.skeletonEnemy.get(entity).initialPos.x - registry.skeletonEnemy.get(entity).roamRange;
					float rightRoamLimit = registry.skeletonEnemy.get(entity).initialPos.x + registry.skeletonEnemy.get(entity).roamRange;

					// update immunity duration
					if (registry.skeletonEnemy.get(entity).immunity_duration_ms > 0) {
						registry.skeletonEnemy.get(entity).immunity_duration_ms -= elapsed_ms;
					}

					// check if skeleton is still within its bounds (roaming range)
					if (motion.position.x > leftRoamLimit && motion.position.x < rightRoamLimit) {
						// check if player is within skeleton engage range
						if ((pow(player.position.x - motion.position.x, 2) < pow(registry.skeletonEnemy.get(entity).attackRange, 2)) &&
							(player.position.x > leftRoamLimit && player.position.x < rightRoamLimit) &&
							!registry.attackPathTimers.has(entity)) {
							registry.attackPathTimers.emplace(entity);
							// check if player is left of skeleton
							if (player.position.x < motion.position.x) {
								motion.angle = 135;
								motion.velocity.x = -(registry.skeletonEnemy.get(entity).aggroSpeed);
							}
							// check if player is right of skeleton
							else if (player.position.x > motion.position.x) {
								motion.angle = 0;
								motion.velocity.x = (registry.skeletonEnemy.get(entity).aggroSpeed);
							}

						}
						// player is not within range of skeleton engage range
						else {
							// maintain direction but at idleSpeed
							if (motion.velocity.x > 0) {
								motion.velocity.x = registry.skeletonEnemy.get(entity).idleSpeed;
							}
							else if (motion.velocity.x < 0) {
								motion.velocity.x = -registry.skeletonEnemy.get(entity).idleSpeed;
							}
						}
					}
					// flip velocity when reached the side of roaming limit
					else if (motion.position.x < leftRoamLimit) {
						motion.velocity.x *= -1;
						motion.angle = 0;
						motion.position.x = leftRoamLimit + (motion.velocity.x * step_seconds);
					}
					else if (motion.position.x > rightRoamLimit) {
						motion.velocity.x *= -1;
						motion.angle = 135;
						motion.position.x = rightRoamLimit + (motion.velocity.x * step_seconds);
					}
					// update x position
					motion.position.x += (motion.velocity.x * step_seconds);
				}
			}
		}
		// Check for collisions between all moving entities
		ComponentContainer<Motion>& motion_container = registry.motions;
		for (uint i = 0; i < motion_container.components.size(); i++)
		{
			Motion& motion_i = motion_container.components[i];
			Entity entity_i = motion_container.entities[i];
			if (registry.background.has(entity_i)) continue;
			bool mesh_i = false, mesh_j = false;
			if (registry.player_attack1.has(entity_i))
			{
				mesh_i = true;
			}

			// state 0: i is non-mesh, j is non-mesh
			// state 1: i is mesh, j is non-mesh
			// state 2: i is non-mesh, j is mesh
			// state 3: i is mesh, j is mesh ***Note: currently not possible

			// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
			for (uint j = i + 1; j < motion_container.components.size(); j++)
			{
				Motion& motion_j = motion_container.components[j];
				Entity entity_j = motion_container.entities[j];
				if (registry.background.has(entity_j)) continue;
				if (registry.player_attack1.has(entity_j))
				{
					mesh_j = true;
				}
				if (mesh_i || mesh_j)
				{
					if (collides(motion_i, motion_j))
					{
						bool mesh_collide = false;
						if (mesh_i)
						{
							if (collidesMeshBox(motion_i, motion_j, entity_i))
							{
								mesh_collide = true;
							}
						}
						else if (mesh_j)
						{
							if (collidesMeshBox(motion_j, motion_i, entity_j))
							{
								mesh_collide = true;
							}
						}
						if (mesh_collide)
						{
							// Create a collisions event
							// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
							registry.collisions.emplace_with_duplicates(entity_i, entity_j);
							registry.collisions.emplace_with_duplicates(entity_j, entity_i);
						}
					}
				}
				else
				{
					if (collides(motion_i, motion_j))
					{
						// Create a collisions event
						// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
						registry.collisions.emplace_with_duplicates(entity_i, entity_j);
						registry.collisions.emplace_with_duplicates(entity_j, entity_i);
					}
				}
			}
		}
	}
}