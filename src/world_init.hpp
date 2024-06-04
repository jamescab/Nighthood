#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float STATUS_EFFECT_BB_HEIGHT = 0.6f * 75.f;
const float STATUS_EFFECT_BB_WIDTH = 0.6f * 75.f;

const float SAW_HEIGHT = 80.f;
const float SAW_WIDTH = 80.f;

const float BAT_HEIGHT = 50.f;
const float BAT_WIDTH = 50.f;

const float SKELETON_HEIGHT = 160.f;
const float SKELETON_WIDTH = 80.f;

const float MUSHROOM_HEIGHT = 150.f;
const float MUSHROOM_WIDTH = 100.f;

const float WIZARD_HEIGHT = 100.f;
const float WIZARD_WIDTH = 50.f;

const float MAGICBALL_HEIGHT = 20.f;
const float MAGICBALL_WIDTH = 20.f;

const float DEMON_HEIGHT = 250.f;
const float DEMON_WIDTH = 250.f;

const float GHOST_HEIGHT = 80.f;
const float GHOST_WIDTH = 80.f;

const float WOLF_HEIGHT = 100.f;
const float WOLF_WIDTH = 100.f;

const float GOLEM_HEIGHT = 250.f;
const float GOLEM_WIDTH = 250.f;

//width to height ratio 3:1
const float ARMPROJECTILE_HEIGHT = 20.f;
const float ARMPROJECTILE_WIDTH = 60.f;

const float ENERGYPROJECTILE_HEIGHT = 50.f;
const float ENERGYPROJECTILE_WIDTH = 50.f;

Entity createEnergyProjectile(RenderSystem* renderer, vec2 pos, vec2 distanceVec, float angle);
Entity createArmProjectile(RenderSystem* renderer, vec2 pos, vec2 distanceVec, float angle);
Entity createGolem(RenderSystem* renderer, vec2 pos);

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos, int health);

// GameOver
Entity createScene(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID id);

// the background
// the background, background id = 1 to set background to background.png, it specifies the id of the background image it wants to use
Entity createBackground(RenderSystem* renderer, vec2 pos, int background_id);
Entity createBackgroundCloud(RenderSystem* renderer, vec2 pos);
Entity createBackgroundTrees(RenderSystem* renderer, vec2 pos);
// the tile
Entity createTile(RenderSystem* renderer, vec2 pos);

// the custom size tile
Entity createCustomTile(RenderSystem* renderer, vec2 pos,vec2 scale);
Entity createVerticalTile(RenderSystem* renderer, vec2 pos,vec2 scale);
Entity createVerticalLongTile(RenderSystem* renderer, vec2 pos,vec2 scale);
Entity createVerticalLongThickTile(RenderSystem* renderer, vec2 pos,vec2 scale);

Entity createStatue(RenderSystem* renderer, vec2 position);
Entity createAttackBuff(RenderSystem* renderer, vec2 position);
Entity createDefenseBuff(RenderSystem* renderer, vec2 position);
Entity createPedestal(RenderSystem* renderer, vec2 position);

Entity createSaw(RenderSystem* renderer, vec2 pos);

Entity createPotion(RenderSystem* renderer, vec2 pos, PotionType pot);

Entity createRangedEnemy(RenderSystem* renderer, vec2 pos, float attackRange, float roamRange, bool stationary);

Entity createWizard(RenderSystem* renderer, vec2 pos, float attackRange, float roamRange, bool stationary);

Entity createFireBall(RenderSystem* renderer, vec2 pos, float angle, vec2 velocity);

Entity createMagicBall(RenderSystem* renderer, vec2 pos, float angle, vec2 velocity, TEXTURE_ASSET_ID texture, GEOMETRY_BUFFER_ID geometry);

Entity createWolf(RenderSystem* renderer, vec2 pos, float roamRange, float set_jump_cd);

// attack1 obj
Entity createAttack1(RenderSystem* renderer, vec2 pos);

// attack2 obj
Entity createAttack2(RenderSystem* renderer, vec2 pos);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// bat enemy
// flies up and down a certain distance, no horizontal movement
Entity createBat(RenderSystem* renderer, vec2 pos, float flyRange);

// Skeleton enemy
// walks back and forth in an area, approaches player at certain distance up to range
Entity createSkeleton(RenderSystem* renderer, vec2 pos, float attackRange, float roamRange);

// Demon boss
Entity createDemon(RenderSystem* renderer, vec2 pos, float boundLeft, float boundRight);

// Health entity
Entity createHeart(RenderSystem* renderer, vec2 position, vec2 relative_pos, vec2 scale);

// Shield entity
Entity createShield(RenderSystem* renderer, vec2 position, vec2 relative_pos, vec2 scale);

// Door entity
Entity createDoor(RenderSystem* renderer, vec2 position, vec2 scale);

// Text
Entity createText(std::string s, vec2 position, vec2 scale, vec3 color, mat4 trans, bool fixed);

// Ghost Enemy
Entity createGhost(RenderSystem* renderer, vec2 pos);
