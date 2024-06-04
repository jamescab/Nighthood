#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

Entity createPlayer(RenderSystem* renderer, vec2 pos, int health)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = { 50.f, 100.f };
	motion.scale.y *= 1; // point front to the right

	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.players.emplace(entity);
	registry.players.get(entity).health = health;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_SHEET, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::PLAYER });

	registry.players.get(entity).camera_x = registry.motions.get(entity).position.x;
	registry.players.get(entity).camera_y = registry.motions.get(entity).position.y;

	return entity;
}

Entity createIntro(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();
	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	motion.scale = vec2({ window_width_px, window_height_px });

	registry.background.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::INTRO, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTUREDFIXED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}


Entity createScene(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID id) {
	auto entity = Entity();
	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	motion.scale = vec2({ window_width_px, window_height_px });

	registry.background.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ id, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTUREDFIXED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBackground(RenderSystem* renderer, vec2 pos, int background_id)
{
	auto entity = Entity();
	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ window_width_px, window_height_px });
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.background.emplace(entity);
	// Designate which background mapping to use according to the background_id parameter
	TEXTURE_ASSET_ID bg_id = TEXTURE_ASSET_ID::BACKGROUND;
	if (background_id == 2)
	{
		bg_id = TEXTURE_ASSET_ID::BACKGROUND2;
	}
	
	if (background_id == 3)
	{
		bg_id = TEXTURE_ASSET_ID::BACKGROUND3;
	}
	registry.renderRequests.insert(
		entity,
		{ bg_id, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTUREDFIXED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBackgroundCloud(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ 576.f, -324.f });
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.background.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BACKGROUND_CLOUD, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTUREDFIXED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBackgroundTrees(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ 800.f, 600.f });
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.background.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BACKGROUND_TREE, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTUREDFIXED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createAttack1(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PLAYER_ATTACK1);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	// NOTE* range of attack does not neccessarily depend on SCALE! See mesh box collision for range
	motion.scale = vec2({ 180.f, -100.f });
	registry.player_attack1.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::PLAYER,
			GEOMETRY_BUFFER_ID::PLAYER_ATTACK1 });

	return entity;
}

Entity createAttack2(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PLAYER_ATTACK2);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	// NOTE* range of attack does not neccessarily depend on SCALE! See mesh box collision for range
	motion.scale = vec2({ 180.f, -100.f });
	registry.player_attack1.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::PLAYER,
			GEOMETRY_BUFFER_ID::PLAYER_ATTACK2 });

	return entity;
}

Entity createTile(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -100.0f, 100.0f });
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.tiles.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TILE, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createCustomTile(RenderSystem* renderer, vec2 pos, vec2 scale)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = scale;
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.tiles.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TILE, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createVerticalTile(RenderSystem* renderer, vec2 pos, vec2 scale)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = scale;
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.tiles.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TILE_VERT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createVerticalLongTile(RenderSystem* renderer, vec2 pos, vec2 scale)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = scale;
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.tiles.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TILE_VERT_LONG, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createVerticalLongThickTile(RenderSystem* renderer, vec2 pos, vec2 scale)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = scale;
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.tiles.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TILE_VERT_LONG, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createDoor(RenderSystem* renderer, vec2 pos, vec2 scale)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	// motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = scale;
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.doors.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DOOR, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createStatue(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	// motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { 50, 100 };
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.readables.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::STATUE, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createAttackBuff(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	// motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { 15, 20 };
	registry.buffs.emplace(entity);
	registry.buffs.get(entity).buff_type = "attack";
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ATTACK_BUFF, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createDefenseBuff(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	// motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { 15, 20 };
	registry.buffs.emplace(entity);
	registry.buffs.get(entity).buff_type = "defense";
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DEFENSE_BUFF, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createPedestal(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	// motion.interactable = false;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { 40, 50 };
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PEDESTAL, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBat(RenderSystem* renderer, vec2 pos,float flyRange)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 200.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = {BAT_WIDTH + 20,BAT_HEIGHT + 20 };
	// Create and (empty) Chicken component to be able to refer to all eagles
	BatEnemy& batEnemy = registry.batEnemy.emplace(entity);
	batEnemy.flyRange = flyRange;
	batEnemy.initialPos = pos;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BAT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::BAT_ENEMY });

	return entity;
}

Entity createFireBall(RenderSystem* renderer, vec2 pos, float angle, vec2 velocity) {

	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = angle;
	motion.velocity = velocity;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -20, 20 });
	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.fireBalls.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FIREBALL, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createMagicBall(RenderSystem* renderer, vec2 pos, float angle, vec2 velocity, TEXTURE_ASSET_ID texture, GEOMETRY_BUFFER_ID geometry) {

	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = angle;
	motion.velocity = velocity;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { MAGICBALL_WIDTH, MAGICBALL_HEIGHT };

	// Create and (empty) Chicken component to be able to refer to all eagles
	if (texture == TEXTURE_ASSET_ID::MAGICBALL1) {
		registry.magicBalls1.emplace(entity);
	}

	if (texture == TEXTURE_ASSET_ID::MAGICBALL2) {
		registry.magicBalls2.emplace(entity);
	}
	registry.renderRequests.insert(
		entity,
		{ texture, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			geometry});

	return entity;
}


Entity createRangedEnemy(RenderSystem* renderer, vec2 pos, float attackRange, float roamRange, bool stationary) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 200.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { MUSHROOM_WIDTH,MUSHROOM_HEIGHT };
	// Create and (empty) Chicken component to be able to refer to all eagles
	RangedEnemy& rangedEnemy = registry.rangedEnemy.emplace(entity);
	rangedEnemy.attackRange = attackRange;
	rangedEnemy.initialPos = pos;
	rangedEnemy.roamRange = roamRange;
	rangedEnemy.stationary = stationary;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MUSHROOM, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::MUSHROOM_ENEMY });

	return entity;
}

Entity createWizard(RenderSystem* renderer, vec2 pos, float attackRange, float roamRange, bool stationary) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 200.f, 0.f };

	motion.scale = { WIZARD_WIDTH, WIZARD_HEIGHT };

	Wizard& wizard = registry.wizards.emplace(entity);
	wizard.attackRange = attackRange;
	wizard.initialPos = pos;
	wizard.roamRange = roamRange;
	wizard.stationary = stationary;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WIZARD, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::WIZARD_ENEMY });

	return entity;
}

Entity createGolem(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { GOLEM_WIDTH,GOLEM_HEIGHT };
	// Create and (empty) Chicken component to be able to refer to all eagles
	Golem& golem = registry.golem.emplace(entity);


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GOLEM, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::GOLEM_ENEMY });

	return entity;
}

Entity createArmProjectile(RenderSystem* renderer, vec2 pos,vec2 distanceVec,float angle) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = angle;

	motion.velocity.x = distanceVec.x * 600;
	motion.velocity.y = distanceVec.y * 600;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { ARMPROJECTILE_WIDTH,ARMPROJECTILE_HEIGHT};
	// Create and (empty) Chicken component to be able to refer to all eagles
	ArmProjectile& ArmProjectile = registry.armProjectile.emplace(entity);


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ARMPROJECTILE, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::ARMPROJECTILE_ENEMY });

	return entity;
}

Entity createEnergyProjectile(RenderSystem* renderer, vec2 pos, vec2 distanceVec, float angle) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = angle;
	motion.velocity.x = distanceVec.x * 500;
	motion.velocity.y = distanceVec.y * 500;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { ENERGYPROJECTILE_WIDTH,ENERGYPROJECTILE_HEIGHT };
	// Create and (empty) Chicken component to be able to refer to all eagles
	EnergyProjectile& EnergyProjectile = registry.energyProjectile.emplace(entity);


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENERGYPROJECTILE, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::ENERGYPROJECTILE_ENEMY });

	return entity;
}

Entity createDemon(RenderSystem* renderer, vec2 pos, float boundLeft, float boundRight) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { DEMON_WIDTH,DEMON_HEIGHT };
	// Create and (empty) Chicken component to be able to refer to all eagles
	Demon& demon = registry.demonBoss.emplace(entity);
	demon.boundLeft = boundLeft;
	demon.boundRight = boundRight;


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DEMON, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::DEMON_ENEMY });

	return entity;
}

Entity createSkeleton(RenderSystem* renderer, vec2 pos, float attackRange, float roamRange) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 200.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { SKELETON_WIDTH,SKELETON_HEIGHT };
	// Create and (empty) Chicken component to be able to refer to all eagles
	SkeletonEnemy& skeletonEnemy = registry.skeletonEnemy.emplace(entity);
	skeletonEnemy.attackRange = attackRange;
	skeletonEnemy.initialPos = pos;
	skeletonEnemy.roamRange = roamRange;


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SKELETON_IDLE, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SKELETON_ENEMY });

	return entity;
}

Entity createGhost(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 200.f, 200.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { GHOST_HEIGHT, GHOST_WIDTH };
	// Create and (empty) Chicken component to be able to refer to all eagles
	GhostEnemy& ghostEnemy = registry.ghostEnemy.emplace(entity);
	ghostEnemy.initialPos = pos;


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GHOST, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

//set jump_cd minimum 3000 for balancing
Entity createWolf(RenderSystem* renderer, vec2 pos, float roamRange,float set_jump_cd) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.interactable = true;
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 300.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { WOLF_WIDTH, WOLF_HEIGHT };
	// Create and (empty) Chicken component to be able to refer to all eagles
	WolfEnemy& wolfEnemy = registry.wolfEnemy.emplace(entity);
	wolfEnemy.initialPos = pos;
	wolfEnemy.roamRange = roamRange;
	wolfEnemy.set_jump_cd = set_jump_cd;
	wolfEnemy.jump_cd = set_jump_cd;


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WOLF, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::WOLF_ENEMY });

	return entity;
}


Entity createSaw(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { SAW_WIDTH, -SAW_HEIGHT };
	// Create and (empty) status component to be able to refer to all eagles
	Saw& saw = registry.saws.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SAW, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SAW });

	return entity;
}

Entity createPotion(RenderSystem* renderer, vec2 pos, PotionType pot) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);	

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = { -STATUS_EFFECT_BB_HEIGHT, STATUS_EFFECT_BB_HEIGHT };
	// Create and (empty) status component to be able to refer to all eagles

	Potion& status = registry.potions.emplace(entity);
	status.effect = pot;
	status.timeout_ms = 120000;
	

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::STATUS_EFFECT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EGG,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}


Entity createHeart(RenderSystem* renderer, vec2 pos, vec2 r_pos, vec2 scale) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos + r_pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = scale;
	// Create and (empty) status component to be able to refer to all eagles

	Heart& heart = registry.player_health.emplace(entity);
	heart.relative_pos = r_pos;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HEART_SHEET, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTUREDFIXED,
			GEOMETRY_BUFFER_ID::PLAYER_HEART });

	return entity;
}

Entity createShield(RenderSystem* renderer, vec2 pos, vec2 r_pos, vec2 scale) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos + r_pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = scale;
	// Create and (empty) status component to be able to refer to all eagles

	Shield& shield = registry.shields.emplace(entity);
	shield.relative_pos = r_pos;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SHIELD_SHEET, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTUREDFIXED,
			GEOMETRY_BUFFER_ID::PLAYER_SHIELD });

	return entity;
}

Entity createText(std::string s, vec2 position, vec2 scale, vec3 color, mat4 trans, bool fixed = false) {
	auto entity = Entity();

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = scale;

	Text& text = registry.texts.emplace(entity);
	text.str = s;
	text.color = color;
	text.trans = trans;
	text.fixed = fixed;

	return entity;
}