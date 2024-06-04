// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include <iostream>

// json
#include <json.hpp>
using json = nlohmann::json;

// Game configuration
int playerHealth = 3;
bool movingLeft = false;
bool movingRight = false;
bool playerCanRead = false;
bool playerCanPickUp = false;
bool playerIsReading = false;
bool inCutscene = false;
int currentSlide = 0;
int whichCutscene = 1;
int honorGained = 0;
int level_start_honor = 0;
int bossHealth = 0;
bool defenseBuffGained = false;
bool attackBuffGained = false;
bool gameStarted = false;
bool displayHonor = false;
bool showBossHealth = false;
bool previousKeyA = false;
bool previousKeyD = false;

float lerp(float start, float end, float t) {
	return start * (1 - t) + end * t;
}

float calculateAngle(vec2& first, vec2& second) {
	float normalized_y = (first.y - second.y) / sqrt(pow(second.x - first.x, 2) + pow(second.y - first.y, 2));
	float normalized_x = (first.x - second.x) / sqrt(pow(second.x - first.x, 2) + pow(second.y - first.y, 2));

	return atan2(normalized_y, normalized_x);
}

float cubicInterp(float timeCurrent, float velocity)
{
	float a, b, c, d;
	if (velocity > 0) {
		a = -1.f / pow(600, 3);
		b = 3.f / pow(600, 2);
		c = -1.f;
		d = velocity;
	}
	else {
		a = 1.f / pow(600, 3);
		b = -3.f / pow(600, 2);
		c = 1.f;
		d = velocity;
	}

	return (a * pow(timeCurrent, 3)) + (b * pow(timeCurrent, 2) + (c * timeCurrent) + d);
}

void to_json(json& j, const GameState& gs, const Player& p, const Motion& m) {
	j = json{
		{ "game_state", gs },
		{ "health", p.health },
		{ "shield", p.shield },
		{ "interactable", m.interactable},
		{ "position.x", m.position.x },
		{ "position.y", m.position.y },
		{ "isJumping", m.isJumping},
		{ "isFalling", m.isFalling },
		{ "touchingWall", m.touchingWall },
		{ "honorGained" , honorGained }
	};
}

void from_json(json& j, GameState &gs, Player&p, Motion& m) {
	j.at("game_state").get_to(gs);
	j.at("health").get_to(p.health);
	j.at("shield").get_to(p.shield);
	j.at("interactable").get_to(m.interactable);
	j.at("position.x").get_to(m.position.x);
	j.at("position.y").get_to(m.position.y);
	j.at("isJumping").get_to(m.isJumping);
	j.at("isFalling").get_to(m.isFalling);
	j.at("touchingWall").get_to(m.touchingWall);
	j.at("honorGained").get_to(honorGained);
}

// Create the bug world
WorldSystem::WorldSystem()
	: points(0)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy audio components
	if (player_death_sound != nullptr)
		Mix_FreeChunk(player_death_sound);
	if (player_heal_sound != nullptr)
		Mix_FreeChunk(player_heal_sound);
	if (player_buff_sound != nullptr)
		Mix_FreeChunk(player_buff_sound);
	if (player_jump_sound != nullptr)
		Mix_FreeChunk(player_jump_sound);
	if (player_roll_sound != nullptr)
		Mix_FreeChunk(player_roll_sound);
	if (player_take_damage != nullptr)
		Mix_FreeChunk(player_take_damage);
	if (enemy_take_damage != nullptr)
		Mix_FreeChunk(enemy_take_damage);
	if (potion_disappear_sound != nullptr)
		Mix_FreeChunk(potion_disappear_sound);
	if (next_area_sound != nullptr)
		Mix_FreeChunk(next_area_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// DO NOT WORK WELL WITH CAMERA, see render_system.cpp ln 190
void WorldSystem::drawDebugBoundingBox(
	const vec2& pos,
	const vec2& scale,
	vec3 color = { 1, 0.8f, 0.8f },
	float lineThickns = 5.0f)
{
	// left
	Entity left = createLine({ pos.x - abs(scale.x) / 2.0f, pos.y }, { lineThickns, abs(scale.y) });
	registry.colors.insert(left, color);
	// right
	Entity right = createLine({ pos.x + abs(scale.x) / 2.0f, pos.y }, { lineThickns, abs(scale.y) });
	registry.colors.insert(right, color);
	// top
	Entity top = createLine({ pos.x, pos.y - abs(scale.y) / 2.0f }, { abs(scale.x), lineThickns });
	registry.colors.insert(top, color);
	// bottom
	Entity bottom = createLine({ pos.x, pos.y + abs(scale.y) / 2.0f }, { abs(scale.x), lineThickns });
	registry.colors.insert(bottom, color);
}

// DO NOT WORK WELL WITH CAMERA, see render_system.cpp ln 190
void WorldSystem::drawDebugDot(
	Entity& e,
	vec3 color = { 1, 0.8f, 0.8f },
	float lineThickns = 5.0f)
{
	if (registry.meshPtrs.has(e))
	{
		float xMin = INT_MAX, xMax = INT_MIN, yMin = INT_MAX, yMax = INT_MIN;

		// these offset should match those in mesh collision
		// to get a tighter bound
		const float offset = 185.f;
		const float sizeOffset_x = registry.motions.get(e).scale.x > 0 ? offset : -offset;
		const float sizeOffset_y = 115.f;
		float& posX = registry.motions.get(e).position.x;
		float& posY = registry.motions.get(e).position.y;

		for (uint i = 0; i < registry.meshPtrs.get(e)->vertices.size(); i += 25)
		{

			vec2 curr = {
				posX + registry.meshPtrs.get(e)->vertices[i].position.x * sizeOffset_x,
				posY - registry.meshPtrs.get(e)->vertices[i].position.y * sizeOffset_y };

			// draw outline dots, for debug
			Entity currP = createLine(curr, { lineThickns, lineThickns });
			registry.colors.insert(currP, color);

			xMin = std::min(xMin, curr.x);
			xMax = std::max(xMax, curr.x);
			yMin = std::min(yMin, curr.y);
			yMax = std::max(yMax, curr.y);
		}
		float nw = (xMax - xMin) / 2.0f;
		float nh = (yMax - yMin) / 2.0f;

		vec2 npos = vec2({ (xMax + xMin) / 2.0f, (yMax + yMin) / 2.0f });

		// left
		Entity left = createLine({ npos.x - nw, npos.y }, { lineThickns, nh * 2.0f });
		registry.colors.insert(left, color);
		// right
		Entity right = createLine({ npos.x + nw, npos.y }, { lineThickns, nh * 2.0f });
		registry.colors.insert(right, color);
		// top
		Entity top = createLine({ npos.x, npos.y - nh }, { nw * 2.0f, lineThickns });
		registry.colors.insert(top, color);
		// bottom
		Entity bottom = createLine({ npos.x, npos.y + nh }, { nw * 2.0f, lineThickns });
		registry.colors.insert(bottom, color);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Nighthood", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	player_death_sound = Mix_LoadWAV(audio_path("player_death.wav").c_str());
	player_heal_sound = Mix_LoadWAV(audio_path("player_heal.wav").c_str());
	player_buff_sound = Mix_LoadWAV(audio_path("player_buff.wav").c_str());
	player_jump_sound = Mix_LoadWAV(audio_path("player_jump.wav").c_str());
	player_roll_sound = Mix_LoadWAV(audio_path("player_roll.wav").c_str());
	player_take_damage = Mix_LoadWAV(audio_path("player_take_damage.wav").c_str());
	enemy_take_damage = Mix_LoadWAV(audio_path("enemy_take_damage.wav").c_str());
	potion_disappear_sound = Mix_LoadWAV(audio_path("potion_disappear.wav").c_str());
	next_area_sound = Mix_LoadWAV(audio_path("next_area.wav").c_str());
	boss_death_sound = Mix_LoadWAV(audio_path("boss_death.wav").c_str());
	background_music = Mix_LoadWAV(audio_path("background_music.wav").c_str());
	boss_music = Mix_LoadWAV(audio_path("boss_music.wav").c_str());
	intro_music = Mix_LoadWAV(audio_path("intro.wav").c_str());

	if (player_death_sound == nullptr || player_heal_sound == nullptr || player_jump_sound == nullptr || player_roll_sound == nullptr || player_buff_sound == nullptr
		|| potion_disappear_sound == nullptr || next_area_sound == nullptr || player_take_damage == nullptr || enemy_take_damage == nullptr || boss_death_sound == nullptr
		|| background_music == nullptr || boss_music == nullptr || intro_music == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("player_death.wav").c_str(),
			audio_path("player_heal.wav").c_str(),
			audio_path("player_buff.wav").c_str(),
			audio_path("player_jump.wav").c_str(),
			audio_path("player_roll.wav").c_str(),
			audio_path("player_take_damage.wav").c_str(),
			audio_path("enemy_take_damage.wav").c_str(),
			audio_path("potion_disappear.wav").c_str(),
			audio_path("next_area.wav").c_str(),
			audio_path("boss_death.wav").c_str(),
			audio_path("background_music.wav").c_str(),
			audio_path("boss_music.wav").c_str(),
			audio_path("intro.wav").c_str());

		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	Mix_PlayChannel(2, intro_music, -1);
	Mix_Volume(2, MIX_MAX_VOLUME / 2);
	this->renderer = renderer_arg;
	renderer->fps_bool = false;
	renderer->help_bool = false;

	intro();
}

void WorldSystem::load_game_save() {
	Mix_HaltChannel(2);
	Mix_PlayChannel(0, background_music, -1);
	std::string filename = "game_save.json";
	std::ifstream infile;
	std::ofstream outfile;


	infile.open(std::string(PROJECT_SOURCE_DIR) + filename);

	// If file doesn't exist create
	if (infile.fail()) {
		infile.close();
		outfile.open(std::string(PROJECT_SOURCE_DIR) + filename);
		outfile.close();
		printf("CREATED SAVE FILE\n");
		game_state = LevelOne;
	}
	else {
		json data = json::parse(infile);

		Motion m;
		Player p;

		from_json(data, game_state, p, m);
		level_start_honor = honorGained;

		switch_state(game_state);

		Player& p_player = registry.players.get(player);
		p_player.health = p.health;
		p_player.shield = p.shield;
		update_hearts();
		update_shields();


		Motion& p_motion = registry.motions.get(player);
		p_motion.interactable = m.interactable;
		p_motion.position.x = m.position.x;
		p_motion.position.y = m.position.y;
		p_motion.isJumping = m.isJumping;
		p_motion.isFalling = m.isFalling;
		p_motion.touchingWall = m.touchingWall;
	}
}

void WorldSystem::save_game() {
	if (registry.players.has(player) && !registry.deathTimers.has(player)) {
		std::string filename = "game_save.json";

		std::ofstream file(std::string(PROJECT_SOURCE_DIR) + filename, std::ofstream::trunc);

		json data;
		Motion& m = registry.motions.get(player);
		Player& p = registry.players.get(player);
		to_json(data, game_state, p, m);

		file << std::setw(4) << data << std::endl;
	}
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	auto& motions_registry = registry.motions;

	// Player related step
	if (registry.players.has(player)) {

		if (registry.players.get(player).health <= 0) {
			registry.players.get(player).alive = false;
		}
		else {
			registry.players.get(player).alive = true;
		}

		if (player_prev_position != -1000.f)
		{
			player_position_derivative = motions_registry.get(player).position.x - player_prev_position;
		}
		player_prev_position = motions_registry.get(player).position.x;

		if (movingLeft) {
			registry.motions.get(player).velocity.x = -700.f;
			if (registry.players.get(player).getCurrState() == "idle") {
				registry.players.get(player).changeState("run", true);
			}
		}

		if (movingRight) {
			registry.motions.get(player).velocity.x = 700.0f;
			if (registry.players.get(player).getCurrState() == "idle") {
				registry.players.get(player).changeState("run", true);
			}
		}

		if (registry.players.get(player).getCurrState() == "roll")
		{
			// roll animation is too long for actual move, we make animation faster to fit
			player_curr_frame -= elapsed_ms_since_last_update;
		}
		else
		{
			player_curr_frame -= elapsed_ms_since_last_update;
		}

		if (player_curr_frame <= 0.0f)
		{
			player_curr_frame = player_update_frame;
			if (registry.players.get(player).getCurrState() != "death")
			{
				registry.players.get(player).incrementStateIndex();
				registry.players.get(player).stateChange = true;
			}
			else
			{
				// death animation only play once
				// death animation has 10 frames, we stop incrementing at 9
				if (registry.players.get(player).state_index != 9)
				{
					registry.players.get(player).incrementStateIndex();
					registry.players.get(player).stateChange = true;
				}
			}
		}

		if (player_attack_curr_cd > 0.f)
		{
			player_attack_curr_cd -= elapsed_ms_since_last_update;
			if (player_attack_curr_cd <= 0.f)
			{
				// remove the attack entity if there's any
				if (registry.player_attack1.entities.size())
				{
					for (Entity e : registry.player_attack1.entities)
					{
						registry.remove_all_components_of(e);
					}
				}
				else if (registry.player_attack2.entities.size())
				{
					for (Entity e : registry.player_attack2.entities)
					{
						registry.remove_all_components_of(e);
					}
				}
				registry.players.get(player).changeState("attack1", false);
				registry.players.get(player).changeState("attack2", false);
			}
		}

		// Remove entities that leave the screen on the left side
		// Iterate backwards to be able to remove without unterfering with the next object to visit
		// (the containers exchange the last element with the current)
		for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
			Motion& motion = motions_registry.components[i];
			Entity entity = motions_registry.entities[i];

			// attack entity follows player
			if ((registry.player_attack1.has(entity) || registry.player_attack2.has(entity))
				&& registry.players.has(player))
			{
				motion.position = registry.motions.get(player).position;
				if ((registry.motions.get(player).angle > 0.f && motion.scale.x > 0.f) ||
					(registry.motions.get(player).angle == 0.f && motion.scale.x < 0.f))
				{
					motion.scale.x *= -1.f;
				}

			}
		if (debugging.in_debug_mode)
		{
			if (registry.demonBoss.has(entity))
			{
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.players.has(entity))
			{
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.eatables.has(entity))
			{
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.batEnemy.has(entity))
			{
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.skeletonEnemy.has(entity))
			{
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.rangedEnemy.has(entity))
			{
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.saws.has(entity))
			{
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.wizards.has(entity))
			{
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.player_attack1.has(entity) || registry.player_attack2.has(entity))
			{
				drawDebugDot(entity);
			}
			if (registry.wolfEnemy.has(entity))
			{
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.golem.has(entity)) {
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.armProjectile.has(entity)) {
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.energyProjectile.has(entity)) {
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.magicBalls1.has(entity)) {
				drawDebugBoundingBox(motion.position, motion.scale);
			}
			if (registry.magicBalls2.has(entity)) {
				drawDebugBoundingBox(motion.position, motion.scale);
			}
		}

		//if (motion.position.x + abs(motion.scale.x) < 0.f) {
		//	if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
		//		registry.remove_all_components_of(motions_registry.entities[i]);
		//}
		}

		float min_counter_ms = 1000.f;
		for (Entity entity : registry.rollTimers.entities) {
			RollTimer& counter = registry.rollTimers.get(entity);
			counter.counter_ms -= elapsed_ms_since_last_update;
			char direction = registry.motions.get(player).attackDirection;
			if (direction == 'n' || direction == 'd') {
				registry.motions.get(player).velocity.x = cubicInterp(600 - counter.counter_ms, 1000.f);
			}
			else if (direction == 'a') {
				registry.motions.get(player).velocity.x = cubicInterp(600 - counter.counter_ms, -1000.f);
			}

			if (counter.counter_ms < min_counter_ms) {
				min_counter_ms = counter.counter_ms;
			}

			if (counter.counter_ms < 0) {
				registry.players.get(player).changeState("roll", false);
				registry.rollTimers.remove(entity);
				registry.motions.get(player).velocity.x = 0.f;
			}
		}

		// update camera
		camera_curr_frame -= elapsed_ms_since_last_update;
		if (camera_curr_frame < 0.f)
		{
			camera_curr_frame = camera_update_frame;
			float& cam_x = registry.players.get(player).camera_x;
			const float& pos_x = registry.motions.get(player).position.x;
			const float& vel_x = registry.motions.get(player).velocity.x;
			float& cam_y = registry.players.get(player).camera_y;
			const float& pos_y = registry.motions.get(player).position.y;
			const float& vel_y = registry.motions.get(player).velocity.y;
			if (vel_y != 0.f)
			{
				// no lerp for y since y is already linear
				cam_y = pos_y + vel_y * y_camera_multiplier;
			}
			else if (vel_y == 0.f && cam_y != pos_y)
			{

				cam_y = lerp(cam_y, pos_y, 0.5f);
			}

			if (vel_x != 0.f)
			{
				cam_x = lerp(cam_x, pos_x + vel_x * x_camera_multiplier, 0.5f);
			}
			else if (vel_x == 0.f && cam_x != pos_x)
			{

				cam_x = lerp(cam_x, pos_x, 0.5f);
			}
		}

		// rolling clouds based on player movement;
		for (Entity e : background_cloud)
		{
			registry.motions.get(e).position.x -=
				player_position_derivative / cloud_relative_speed_factor;
			// left bound
			if (registry.motions.get(e).position.x < window_width_px - background_cloud.size() * cloud_width + cloud_width / 2.f)
			{
				registry.motions.get(e).position.x += cloud_width * background_cloud.size();
			}
			// right bound
			if (registry.motions.get(e).position.x > background_cloud.size() * cloud_width - cloud_width / 2.f)
			{
				registry.motions.get(e).position.x -= cloud_width * background_cloud.size();
			}
		}

		for (Entity e : background_tree)
		{

			registry.motions.get(e).position.x -=
				player_position_derivative / tree_relative_speed_factor;

			// left bound
			if (registry.motions.get(e).position.x < window_width_px - background_tree.size() * tree_width + tree_width / 2.f)
			{
				registry.motions.get(e).position.x += tree_width * background_tree.size();
			}
			// right bound
			if (registry.motions.get(e).position.x > background_tree.size() * tree_width - tree_width / 2.f)
			{
				registry.motions.get(e).position.x -= tree_width * background_tree.size();
			}
		}
	}
	else
	{
		// reset player related data if player died
		player_prev_position = -1000.f;
		player_position_derivative = 0.f;
	}// END of player related step

	for (uint i = 0; i < registry.golem.entities.size(); i++)
	{


		registry.golem.get(registry.golem.entities[i]).render_curr_frame -= elapsed_ms_since_last_update;

		if (registry.golem.get(registry.golem.entities[i]).render_curr_frame <= 0.0f)
		{
			// time to update
			registry.golem.get(registry.golem.entities[i]).render_curr_frame =
				registry.golem.get(registry.golem.entities[i]).render_update_frame;
			registry.golem.get(registry.golem.entities[i]).incrementStateIndex();
			registry.golem.get(registry.golem.entities[i]).stateChange = true;
		}

		if (!registry.golem.get(registry.golem.entities[i]).alive) {
			registry.golem.get(registry.golem.entities[i]).deathDuration -= elapsed_ms_since_last_update;
		}

		if (registry.golem.get(registry.golem.entities[i]).deathDuration <= 0) {
			registry.remove_all_components_of(registry.golem.entities[i]);
			switch_state(Complete);
		}
		else if (registry.golem.get(registry.golem.entities[i]).engaged)
		{
			//stop ghosts from spawning
			ghost_spawn_cd = set_ghost_spawn_cd;
			// shoot arm projectile
			if (registry.golem.get(registry.golem.entities[i]).arm_projectile_curr_cd_ms > 0) {
				registry.golem.get(registry.golem.entities[i]).arm_projectile_curr_cd_ms -= elapsed_ms_since_last_update;
			}

			if (registry.golem.get(registry.golem.entities[i]).arm_projectile_curr_cd_ms <= 0) {
				registry.golem.get(registry.golem.entities[i]).changeState("projectile attack", true);


				float player_x = registry.motions.get(player).position.x;
				float player_y = registry.motions.get(player).position.y;

				float golem_x = registry.motions.get(registry.golem.entities[i]).position.x;
				float golem_y = registry.motions.get(registry.golem.entities[i]).position.y;

				float normalized_y = (player_y - golem_y) / sqrt(pow(golem_x - player_x, 2) + pow(golem_y - player_y, 2));
				float normalized_x = (player_x - golem_x) / sqrt(pow(golem_x - player_x, 2) + pow(golem_y - player_y, 2));

				float angle = atan2(normalized_y, normalized_x);

				vec2 distanceVec = vec2(normalized_x, normalized_y);

				createArmProjectile(renderer, vec2(golem_x, golem_y), distanceVec, angle);

				if (registry.golem.get(registry.golem.entities[i]).health <= 5) {
					createArmProjectile(renderer, vec2(player_x, player_y + 300), vec2(0, -0.5), M_PI / 4 + angle);
					createArmProjectile(renderer, vec2(player_x, player_y - 300), vec2(0, 0.5), M_PI / 4 + angle);

					/*createArmProjectile(renderer, vec2(player_x + 300, player_y), vec2(-0.5, 0), angle);
					createArmProjectile(renderer, vec2(player_x - 300, player_y), vec2(0.5, 0), angle);*/
				}



				if (registry.golem.get(registry.golem.entities[i]).burstCount < 1) {
					registry.golem.get(registry.golem.entities[i]).arm_projectile_curr_cd_ms = registry.golem.get(registry.golem.entities[i]).burst_cd;
					registry.golem.get(registry.golem.entities[i]).burstCount++;
				}
				else {
					registry.golem.get(registry.golem.entities[i]).arm_projectile_curr_cd_ms = registry.golem.get(registry.golem.entities[i]).arm_projectile_cd_ms;
					registry.golem.get(registry.golem.entities[i]).burstCount = 0;
				}

			}

			// shoot energy projectile
			if (registry.golem.get(registry.golem.entities[i]).energy_attack_curr_cd_ms > 0) {
				registry.golem.get(registry.golem.entities[i]).energy_attack_curr_cd_ms -= elapsed_ms_since_last_update;
			}

			if (registry.golem.get(registry.golem.entities[i]).energy_attack_curr_cd_ms <= 0) {
				registry.golem.get(registry.golem.entities[i]).changeState("energy attack", true);
				float player_x = registry.motions.get(player).position.x;
				float player_y = registry.motions.get(player).position.y;

				float golem_x = registry.motions.get(registry.golem.entities[i]).position.x;
				float golem_y = registry.motions.get(registry.golem.entities[i]).position.y;

				float normalized_y = (player_y - golem_y) / sqrt(pow(golem_x - player_x, 2) + pow(golem_y - player_y, 2));
				float normalized_x = (player_x - golem_x) / sqrt(pow(golem_x - player_x, 2) + pow(golem_y - player_y, 2));

				float angle = atan2(normalized_y, normalized_x);

				vec2 distanceVec = vec2(normalized_x, normalized_y);

				createEnergyProjectile(renderer, vec2(golem_x, golem_y), distanceVec, angle);
				if (registry.golem.get(registry.golem.entities[i]).health <= 5)
				{
					createEnergyProjectile(renderer, vec2(golem_x, golem_y), vec2(-1, 1) * distanceVec, M_PI / 2 + angle);
					createEnergyProjectile(renderer, vec2(golem_x, golem_y), vec2(1, -1) * distanceVec, M_PI / 2 + angle);
					createEnergyProjectile(renderer, vec2(golem_x, golem_y), vec2(-1, -1) * distanceVec, -angle);

					createEnergyProjectile(renderer, vec2(golem_x, golem_y), vec2(0.4, 0.4) * distanceVec, angle);
					createEnergyProjectile(renderer, vec2(golem_x, golem_y), vec2(-0.4, 0.4) * distanceVec, M_PI / 2 + angle);
					createEnergyProjectile(renderer, vec2(golem_x, golem_y), vec2(0.4, -0.4) * distanceVec, M_PI / 2 + angle);
					createEnergyProjectile(renderer, vec2(golem_x, golem_y), vec2(-0.4, -0.4) * distanceVec, -angle);
				}

				if (registry.golem.get(registry.golem.entities[i]).energyBurstCount < 4) {
					registry.golem.get(registry.golem.entities[i]).energy_attack_curr_cd_ms = registry.golem.get(registry.golem.entities[i]).energyBurstCount;
					registry.golem.get(registry.golem.entities[i]).energyBurstCount++;
				}
				else {
					registry.golem.get(registry.golem.entities[i]).energy_attack_curr_cd_ms = registry.golem.get(registry.golem.entities[i]).energy_attack_cd_ms;
					registry.golem.get(registry.golem.entities[i]).arm_projectile_curr_cd_ms = registry.golem.get(registry.golem.entities[i]).arm_projectile_cd_ms;
					registry.golem.get(registry.golem.entities[i]).energyBurstCount = 0;
				}
			}
		}
	}

	for (uint i = 0; i < registry.armProjectile.entities.size(); i++)
	{
		

		registry.armProjectile.get(registry.armProjectile.entities[i]).duration_ms -= elapsed_ms_since_last_update;

		//timeout duration
		if (registry.armProjectile.get(registry.armProjectile.entities[i]).duration_ms <= 0) {
			registry.remove_all_components_of(registry.armProjectile.entities[i]);
		}
		else
		{
			registry.armProjectile.get(registry.armProjectile.entities[i]).render_curr_frame -= elapsed_ms_since_last_update;
			if (registry.armProjectile.get(registry.armProjectile.entities[i]).render_curr_frame <= 0.0f)
			{
				// time to update
				registry.armProjectile.get(registry.armProjectile.entities[i]).render_curr_frame =
					registry.armProjectile.get(registry.armProjectile.entities[i]).render_update_frame;
				registry.armProjectile.get(registry.armProjectile.entities[i]).incrementStateIndex();
				registry.armProjectile.get(registry.armProjectile.entities[i]).stateChange = true;
			}
		}
	}

	for (uint i = 0; i < registry.energyProjectile.entities.size(); i++)
	{
		
		
		registry.energyProjectile.get(registry.energyProjectile.entities[i]).duration_ms -= elapsed_ms_since_last_update;
		//timeout duration
		if (registry.energyProjectile.get(registry.energyProjectile.entities[i]).duration_ms <= 0) {
			registry.remove_all_components_of(registry.energyProjectile.entities[i]);
		}

		else
		{
			registry.energyProjectile.get(registry.energyProjectile.entities[i]).render_curr_frame -= elapsed_ms_since_last_update;
			if (registry.energyProjectile.get(registry.energyProjectile.entities[i]).render_curr_frame <= 0.0f)
			{
				// time to update
				registry.energyProjectile.get(registry.energyProjectile.entities[i]).render_curr_frame =
					registry.energyProjectile.get(registry.energyProjectile.entities[i]).render_update_frame;
				registry.energyProjectile.get(registry.energyProjectile.entities[i]).incrementStateIndex();
				registry.energyProjectile.get(registry.energyProjectile.entities[i]).stateChange = true;
			}
		}
	}

	for (uint i = 0; i < registry.demonBoss.entities.size(); i++)
	{
		// NOTE: Demon entity should only be 1 in the map for it to work (it has automated attack)
		// update demon sprite sheet index

		registry.demonBoss.get(registry.demonBoss.entities[i]).render_curr_frame -= elapsed_ms_since_last_update;
		if (registry.demonBoss.get(registry.demonBoss.entities[i]).render_curr_frame <= 0.0f)
		{
			// time to update
			registry.demonBoss.get(registry.demonBoss.entities[i]).render_curr_frame =
				registry.demonBoss.get(registry.demonBoss.entities[i]).render_update_frame;
			registry.demonBoss.get(registry.demonBoss.entities[i]).incrementStateIndex();
			registry.demonBoss.get(registry.demonBoss.entities[i]).stateChange = true;
		}

		if (registry.demonBoss.get(registry.demonBoss.entities[i]).immunity_duration > 0.f)
		{
			registry.demonBoss.get(registry.demonBoss.entities[i]).immunity_duration -= elapsed_ms_since_last_update;
		}

		if (registry.demonBoss.get(registry.demonBoss.entities[i]).getCurrState() == "dead")
		{
			if (registry.demonBoss.get(registry.demonBoss.entities[i]).state_index >=
				registry.demonBoss.get(registry.demonBoss.entities[i]).state_map["dead"].second)
			{
				// delete entity after dead
				registry.remove_all_components_of(registry.demonBoss.entities[i]);
				break;
			}
			continue;
		}

		if (registry.demonBoss.get(registry.demonBoss.entities[i]).walk_timer > 0.f
			&& registry.demonBoss.get(registry.demonBoss.entities[i]).getCurrState() == "idle")
		{
			registry.demonBoss.get(registry.demonBoss.entities[i]).walk_timer -= elapsed_ms_since_last_update;
			if (registry.demonBoss.get(registry.demonBoss.entities[i]).walk_timer < 0.f)
			{
				// start walking
				if (registry.demonBoss.get(registry.demonBoss.entities[i]).state_curr["walk"] == false)
					registry.demonBoss.get(registry.demonBoss.entities[i]).changeState("walk", true);
			}
		}
		// only track player when idle or walk, walk after a certain amount of time of idle
		if (registry.demonBoss.get(registry.demonBoss.entities[i]).getCurrState() == "idle" ||
			registry.demonBoss.get(registry.demonBoss.entities[i]).getCurrState() == "walk")
		{
			if (registry.players.has(player))
			{
				// direction check
				if (registry.motions.get(player).position.x < registry.motions.get(registry.demonBoss.entities[i]).position.x)
				{
					registry.motions.get(registry.demonBoss.entities[i]).scale.x =
						abs(registry.motions.get(registry.demonBoss.entities[i]).scale.x);
				}
				else if (registry.motions.get(player).position.x > registry.motions.get(registry.demonBoss.entities[i]).position.x)
				{
					registry.motions.get(registry.demonBoss.entities[i]).scale.x =
						-abs(registry.motions.get(registry.demonBoss.entities[i]).scale.x);
				}
			}

			if (registry.demonBoss.get(registry.demonBoss.entities[i]).getCurrState() == "walk")
			{
				float multiplier = 1.f;
				// speed up the walking if player is too far
				if (abs(registry.motions.get(registry.demonBoss.entities[i]).position.x - registry.motions.get(player).position.x) > 500.f)
				{
					multiplier = 3.5f;
				}
				if (registry.motions.get(registry.demonBoss.entities[i]).scale.x > 0.f)
				{
					// player <> demon

					// walk if:
					// player's outiside of attack range
					// demon's NOT going out of bound
					if (registry.motions.get(registry.demonBoss.entities[i]).position.x - DEMON_WIDTH / 2.f > registry.demonBoss.get(registry.demonBoss.entities[i]).boundLeft
						&& abs(registry.motions.get(registry.demonBoss.entities[i]).position.x - registry.motions.get(player).position.x) > 100.f)
					{
						// go left
						registry.motions.get(registry.demonBoss.entities[i]).position.x -=
							registry.demonBoss.get(registry.demonBoss.entities[i]).idleSpeed * multiplier;
						registry.demonBoss.get(registry.demonBoss.entities[i]).state_curr["walk"] = true;
					}
					else
					{
						registry.demonBoss.get(registry.demonBoss.entities[i]).state_curr["walk"] = false;
					}
				}
				else
				{
					// demon <> player

					// walk if:
					// player's outiside of attack range
					// demon's NOT going out of bound
					if (registry.motions.get(registry.demonBoss.entities[i]).position.x + DEMON_WIDTH / 2.f < registry.demonBoss.get(registry.demonBoss.entities[i]).boundRight
						&& abs(registry.motions.get(registry.demonBoss.entities[i]).position.x - registry.motions.get(player).position.x) > 100.f)
					{
						// go right
						registry.motions.get(registry.demonBoss.entities[i]).position.x +=
							registry.demonBoss.get(registry.demonBoss.entities[i]).idleSpeed * multiplier;
						registry.demonBoss.get(registry.demonBoss.entities[i]).state_curr["walk"] = true;
					}
					else
					{
						registry.demonBoss.get(registry.demonBoss.entities[i]).state_curr["walk"] = false;
					}
				}
			}
		}

		registry.demonBoss.get(registry.demonBoss.entities[i]).attack_curr_cd_ms -= elapsed_ms_since_last_update;
		if (registry.demonBoss.get(registry.demonBoss.entities[i]).attack_curr_cd_ms <= 0.0f)
		{
			// attack
			// randomize next attack [4000, 8000] ms, more fun
			registry.demonBoss.get(registry.demonBoss.entities[i]).attack_cd_ms = 4000.f + (float)(rand() % 4000);
			registry.demonBoss.get(registry.demonBoss.entities[i]).attack_curr_cd_ms =
				registry.demonBoss.get(registry.demonBoss.entities[i]).attack_cd_ms;

			// initate actual attack
			registry.demonBoss.get(registry.demonBoss.entities[i]).attack_actual_timer =
				registry.demonBoss.get(registry.demonBoss.entities[i]).attack_actual_reset_timer;

			registry.demonBoss.get(registry.demonBoss.entities[i]).changeState("attack", true);

			registry.demonBoss.get(registry.demonBoss.entities[i]).state_curr["walk"] = false;

			registry.demonBoss.get(registry.demonBoss.entities[i]).incrementStateIndex();
			registry.demonBoss.get(registry.demonBoss.entities[i]).stateChange = true;
		}

		if (registry.demonBoss.get(registry.demonBoss.entities[i]).attack_actual_timer > 0.f)
		{
			// attack (actual)
			registry.demonBoss.get(registry.demonBoss.entities[i]).attack_actual_timer -= elapsed_ms_since_last_update;
			if (registry.demonBoss.get(registry.demonBoss.entities[i]).attack_actual_timer < 250.f)
			{
				if ((registry.motions.get(registry.demonBoss.entities[i]).position.x - DEMON_WIDTH / 2.f - registry.demonBoss.get(registry.demonBoss.entities[i]).attack_range < registry.motions.get(player).position.x + abs(registry.motions.get(player).scale.x) 
					&& registry.motions.get(registry.demonBoss.entities[i]).position.x > registry.motions.get(player).position.x)
					|| (registry.motions.get(registry.demonBoss.entities[i]).position.x + DEMON_WIDTH / 2.f + registry.demonBoss.get(registry.demonBoss.entities[i]).attack_range > registry.motions.get(player).position.x - abs(registry.motions.get(player).scale.x) 
						&& registry.motions.get(registry.demonBoss.entities[i]).position.x < registry.motions.get(player).position.x))
				{
					auto collides = [&](const Motion& p, const Motion& d, float attack_range)
					{
						auto isPointInBox = [&](const Motion& bbox, float x, float y) {
							if (((bbox.position.x + (abs(bbox.scale.x) / 2.0f)) >= x &&
								(bbox.position.x - (abs(bbox.scale.x) / 2.0f)) <= x) &&
								(bbox.position.y + (abs(bbox.scale.y) / 2.0f) >= y &&
								(bbox.position.y - (abs(bbox.scale.y) / 2.0f)) <= y))
								return true;
							return false;
						};

						// boss entity & player
						if (isPointInBox(p, d.position.x + (abs(d.scale.x) / 2.0f), d.position.y + (abs(d.scale.y) / 2.0f)) ||
							isPointInBox(p, d.position.x + (abs(d.scale.x) / 2.0f), d.position.y - (abs(d.scale.y) / 2.0f)) ||
							isPointInBox(p, d.position.x - (abs(d.scale.x) / 2.0f), d.position.y + (abs(d.scale.y) / 2.0f)) ||
							isPointInBox(p, d.position.x - (abs(d.scale.x) / 2.0f), d.position.y - (abs(d.scale.y) / 2.0f)))
						{
							return true;
						}

						if (isPointInBox(d, p.position.x + (abs(p.scale.x) / 2.0f), p.position.y + (abs(p.scale.y) / 2.0f)) ||
							isPointInBox(d, p.position.x + (abs(p.scale.x) / 2.0f), p.position.y - (abs(p.scale.y) / 2.0f)) ||
							isPointInBox(d, p.position.x - (abs(p.scale.x) / 2.0f), p.position.y + (abs(p.scale.y) / 2.0f)) ||
							isPointInBox(d, p.position.x - (abs(p.scale.x) / 2.0f), p.position.y - (abs(p.scale.y) / 2.0f)))
						{
							return true;
						}

						bool face_left = d.scale.x > 0.f ? true : false;
						vec2 p_TL = { p.position.x - (abs(p.scale.x) / 2.0f), p.position.y - (abs(p.scale.y) / 2.0f) };
						//vec2 p_TR = { p.position.x + (abs(p.scale.x) / 2.0f), p.position.y - (abs(p.scale.y) / 2.0f) };
						//vec2 p_BL = { p.position.x - (abs(p.scale.x) / 2.0f), p.position.y + (abs(p.scale.y) / 2.0f) };
						vec2 p_BR = { p.position.x + (abs(p.scale.x) / 2.0f), p.position.y + (abs(p.scale.y) / 2.0f) };

						// attack box, depends on face direction
						/*				____________
										|			|
						________________|			|
						|	attack		|	boss	|
						|_______________|___________|
						*/
						vec2 d_TL = face_left ?
							vec2({ d.position.x - (abs(d.scale.x) / 2.0f) - attack_range, d.position.y }) :
							vec2({ d.position.x + (abs(d.scale.x) / 2.0f), d.position.y });
						//vec2 d_TR = face_left ?
						//	vec2({ d.position.x - (abs(d.scale.x) / 2.0f), d.position.y }) :
						//	vec2({ d.position.x + (abs(d.scale.x) / 2.0f) + attack_range, d.position.y });
						//vec2 d_BL = face_left ?
						//	vec2({ d.position.x - (abs(d.scale.x) / 2.0f) - attack_range, d.position.y + (abs(d.scale.y) / 2.0f) }) :
						//	vec2({ d.position.x + (abs(d.scale.x) / 2.0f), d.position.y + (abs(d.scale.y) / 2.0f) });
						vec2 d_BR = face_left ? 
							vec2({ d.position.x - (abs(d.scale.x) / 2.0f), d.position.y + (abs(d.scale.y) / 2.0f) }) :
							vec2({ d.position.x + (abs(d.scale.x) / 2.0f) + attack_range, d.position.y + (abs(d.scale.y) / 2.0f) });
						
						// boss attack & player
						if (!(p_BR.x < d_TL.x || d_BR.x < p_TL.x || p_TL.y > d_BR.y || d_TL.y > p_BR.y)) {
							return true;
						}
						return false;
					};
					if (collides(registry.motions.get(player),
						registry.motions.get(registry.demonBoss.entities[i]), 
							registry.demonBoss.get(registry.demonBoss.entities[i]).attack_range))
					{
						
						auto& p = registry.players.get(player);
						if (p.immunity_duration_ms <= 0 && !registry.rollTimers.has(player)) {
							update_health(-1);
							p.immunity_duration_ms = 1000.f;
							if (p.health >= 0)
								Mix_PlayChannel(-1, player_take_damage, 0);
						}
					}
				}
			}
			if (registry.demonBoss.get(registry.demonBoss.entities[i]).attack_actual_timer <= 0.f)
			{
				registry.demonBoss.get(registry.demonBoss.entities[i]).changeState("attack", false);
				registry.demonBoss.get(registry.demonBoss.entities[i]).attack_actual_timer = -10.f;
				registry.demonBoss.get(registry.demonBoss.entities[i]).walk_timer =
					registry.demonBoss.get(registry.demonBoss.entities[i]).walk_reset_timer;
			}

		}
	}

	for (uint i = 0; i < registry.skeletonEnemy.entities.size(); i++)
	{
		// update skeleton sprite sheet index
		registry.skeletonEnemy.get(registry.skeletonEnemy.entities[i]).skeleton_curr_frame -= elapsed_ms_since_last_update;
		if (registry.skeletonEnemy.get(registry.skeletonEnemy.entities[i]).skeleton_curr_frame <= 0.0f)
		{
			// time to update
			registry.skeletonEnemy.get(registry.skeletonEnemy.entities[i]).skeleton_curr_frame = 
				skeleton_update_frame;
			registry.skeletonEnemy.get(registry.skeletonEnemy.entities[i]).incrementStateIndex();
			registry.skeletonEnemy.get(registry.skeletonEnemy.entities[i]).stateChange = true;
		}
	}



	for (uint i = 0; i < registry.wolfEnemy.entities.size(); i++)
	{
		// update wolf sprite sheet index
		registry.wolfEnemy.get(registry.wolfEnemy.entities[i]).wolf_curr_frame -= elapsed_ms_since_last_update;
		if (registry.wolfEnemy.get(registry.wolfEnemy.entities[i]).wolf_curr_frame <= 0.0f)
		{
			// time to update
			registry.wolfEnemy.get(registry.wolfEnemy.entities[i]).wolf_curr_frame = wolf_update_frame;
			registry.wolfEnemy.get(registry.wolfEnemy.entities[i]).incrementStateIndex();
			registry.wolfEnemy.get(registry.wolfEnemy.entities[i]).stateChange = true;
		}
	}

	for (uint i = 0; i < registry.saws.entities.size(); i++)
	{
		// update saw sheet index
		registry.saws.get(registry.saws.entities[i]).render_curr_frame -= elapsed_ms_since_last_update;
		if (registry.saws.get(registry.saws.entities[i]).render_curr_frame <= 0.0f)
		{
			// time to update
			registry.saws.get(registry.saws.entities[i]).render_curr_frame =
				registry.saws.get(registry.saws.entities[i]).render_update_frame;
			registry.saws.get(registry.saws.entities[i]).incrementStateIndex();
			registry.saws.get(registry.saws.entities[i]).stateChange = true;
		}
	}
	for (uint i = 0; i < registry.batEnemy.entities.size(); i++)
	{
		// update skeleton sprite sheet index
		registry.batEnemy.get(registry.batEnemy.entities[i]).bat_curr_frame -= elapsed_ms_since_last_update;
		if (registry.batEnemy.get(registry.batEnemy.entities[i]).bat_curr_frame <= 0.0f)
		{
			// time to update
			registry.batEnemy.get(registry.batEnemy.entities[i]).bat_curr_frame =
				bat_update_frame;
			registry.batEnemy.get(registry.batEnemy.entities[i]).incrementStateIndex();
			registry.batEnemy.get(registry.batEnemy.entities[i]).stateChange = true;
		}
	}


	for (uint i = 0; i < registry.rangedEnemy.entities.size(); i++)
	{
		// update mushroom sprite sheet index
		// TODO: Mushroom entity movement detection does not rely on velocity. Need another source
		// of truth to determine if it's moving

		//if (abs(registry.motions.get(registry.rangedEnemy.entities[i]).velocity.x) > 0.f)
		//{
		//	registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).changeState("run", true, false);
		//}
		//else
		//{
		//	registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).changeState("run", false, false);
		//}
		registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).skeleton_curr_frame -= elapsed_ms_since_last_update;
		if (registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).skeleton_curr_frame <= 0.0f)
		{
			// time to update
			registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).skeleton_curr_frame =
				skeleton_update_frame;
			registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).incrementStateIndex();
			registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).stateChange = true;
		}

		// if in player in range of ranged enemy
		if ((pow(registry.motions.get(player).position.x - registry.motions.get(registry.rangedEnemy.entities[i]).position.x, 2) <
			pow(registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).attackRange, 2))) {
			//charge up throw
			// registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).changeState("attack", true, false);
			if (registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).chargeUpTime > 0) {
				registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).chargeUpTime -= 5;
			}

			// throw
			if (registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).chargeUpTime <= 0) {
				// find x distance between enemy and player
				// add extra at the end to adjust x rock velocity for now
				float xRockVelocity = std::abs(registry.motions.get(player).position.x - registry.motions.get(registry.rangedEnemy.entities[i]).position.x) + 200;



				if (registry.motions.get(player).position.x < registry.motions.get(registry.rangedEnemy.entities[i]).position.x) {
					//larger distance between player and enemy the faster the x velocity
					createFireBall(renderer, registry.motions.get(registry.rangedEnemy.entities[i]).position, 45, vec2(-xRockVelocity, -100));
				}
				// check if player is right of skeleton
				else if (registry.motions.get(player).position.x > registry.motions.get(registry.rangedEnemy.entities[i]).position.x) {
					createFireBall(renderer, registry.motions.get(registry.rangedEnemy.entities[i]).position, 45, vec2(xRockVelocity, -100));
				}



				registry.rangedEnemy.get(registry.rangedEnemy.entities[i]).chargeUpTime = 500;
			}
		}
	}

	// Wizards Step
	for (uint i = 0; i < registry.wizards.entities.size(); i++)
	{
		float multiplier = 1.f;
		// Wizard step
		registry.wizards.get(registry.wizards.entities[i]).wizard_curr_frame -= elapsed_ms_since_last_update;
		if (registry.wizards.get(registry.wizards.entities[i]).wizard_curr_frame <= 0.0f)
		{
			// time to update
			registry.wizards.get(registry.wizards.entities[i]).wizard_curr_frame =
				wizard_update_frame;
			registry.wizards.get(registry.wizards.entities[i]).incrementStateIndex();
			registry.wizards.get(registry.wizards.entities[i]).stateChange = true;
		}


		// Add non stationary movement later
		if (registry.wizards.get(registry.wizards.entities[i]).health > 0 && registry.wizards.get(registry.wizards.entities[i]).getCurrState() != "dead") {

			// Hurt Timers
			if (registry.wizards.get(registry.wizards.entities[i]).hurt_duration_ms > 0) {
				registry.wizards.get(registry.wizards.entities[i]).hurt_duration_ms -= elapsed_ms_since_last_update;
			}

			if (registry.wizards.get(registry.wizards.entities[i]).hurt_duration_ms <= 0) {
				registry.wizards.get(registry.wizards.entities[i]).hurt_duration_ms = 0;
				if (registry.wizards.get(registry.wizards.entities[i]).getCurrState() == "hurt") {
					registry.wizards.get(registry.wizards.entities[i]).changeState("hurt", false);
				}
			}

			// Attack timers
			if (registry.wizards.get(registry.wizards.entities[i]).attack_duration_ms > 0) {
				registry.wizards.get(registry.wizards.entities[i]).attack_duration_ms -= elapsed_ms_since_last_update;
			}

			if (registry.wizards.get(registry.wizards.entities[i]).attack_delay_ms > 0) {
				registry.wizards.get(registry.wizards.entities[i]).attack_delay_ms -= elapsed_ms_since_last_update;
			}

			if (registry.wizards.get(registry.wizards.entities[i]).attack_delay_ms <= 0) {
				registry.wizards.get(registry.wizards.entities[i]).attack_delay_ms = 0;
			}

			// if in player in range of ranged enemy play attack animation
			if ((pow(registry.motions.get(player).position.x - registry.motions.get(registry.wizards.entities[i]).position.x, 2) <
				pow(registry.wizards.get(registry.wizards.entities[i]).attackRange, 2))) {
				
				std::string attack[] = {"attack1", "attack2"};
				
				if (registry.wizards.get(registry.wizards.entities[i]).attack_duration_ms <= 0 &&
					registry.wizards.get(registry.wizards.entities[i]).attack_delay_ms <= 0) {
					registry.wizards.get(registry.wizards.entities[i]).changeState(attack[rand() %2], true);
				}
			}

			// Fire
			if (registry.wizards.get(registry.wizards.entities[i]).attack_duration_ms <= 150 && 
				registry.wizards.get(registry.wizards.entities[i]).hurt_duration_ms <= 0 &&
				(registry.wizards.get(registry.wizards.entities[i]).getCurrState() == "attack1" || 
					registry.wizards.get(registry.wizards.entities[i]).getCurrState() == "attack2")) { 
				float velocity = 1000;


				Motion wizard = registry.motions.get(registry.wizards.entities[i]);
				Motion target = registry.motions.get(player);
				if (cos(wizard.angle) > 0) {
					wizard.position.x += 50;
				}
				else {
					wizard.position.x -= 50;
				}
				wizard.position.y -= 15;
				target.position.y -= 25;

				
				float angle = calculateAngle(target.position, wizard.position);

				if (registry.wizards.get(registry.wizards.entities[i]).getCurrState() == "attack1") {
					Entity magicball1 = createMagicBall(renderer, { wizard.position.x, wizard.position.y }, angle, { velocity*cos(angle), velocity*sin(angle) }, TEXTURE_ASSET_ID::MAGICBALL1, GEOMETRY_BUFFER_ID::MAGICBALL1);
				}

				if (registry.wizards.get(registry.wizards.entities[i]).getCurrState() == "attack2") {
					Entity magicball2 = createMagicBall(renderer, { wizard.position.x, wizard.position.y }, angle, { velocity*cos(angle), velocity*sin(angle)}, TEXTURE_ASSET_ID::MAGICBALL2, GEOMETRY_BUFFER_ID::MAGICBALL2);
				}
				registry.wizards.get(registry.wizards.entities[i]).attack_delay_ms = 2000.f;
				registry.wizards.get(registry.wizards.entities[i]).attack_duration_ms = 0;
				registry.wizards.get(registry.wizards.entities[i]).changeState("attack1", false);
				registry.wizards.get(registry.wizards.entities[i]).changeState("attack2", false);
			}

		}
		else {
			// Death
			if (registry.wizards.get(registry.wizards.entities[i]).death_duration_ms > 0) {
				registry.wizards.get(registry.wizards.entities[i]).death_duration_ms -= elapsed_ms_since_last_update;
			}

			if (registry.wizards.get(registry.wizards.entities[i]).death_duration_ms <= 0) {
				registry.remove_all_components_of(registry.wizards.entities[i]);
				break;
			}
		}
	}

	// Magic Balls1
	for (uint i = 0; i < registry.magicBalls1.entities.size(); i++)
	{
		registry.magicBalls1.get(registry.magicBalls1.entities[i]).duration_ms -= elapsed_ms_since_last_update;
		//timeout duration
		if (registry.magicBalls1.get(registry.magicBalls1.entities[i]).duration_ms <= 0) {
			registry.remove_all_components_of(registry.magicBalls1.entities[i]);
		}

		else
		{
			registry.magicBalls1.get(registry.magicBalls1.entities[i]).render_curr_frame -= elapsed_ms_since_last_update;
			if (registry.magicBalls1.get(registry.magicBalls1.entities[i]).render_curr_frame <= 0.0f)
			{
				// time to update
				registry.magicBalls1.get(registry.magicBalls1.entities[i]).render_curr_frame =
					registry.magicBalls1.get(registry.magicBalls1.entities[i]).render_update_frame;
				registry.magicBalls1.get(registry.magicBalls1.entities[i]).incrementStateIndex();
				registry.magicBalls1.get(registry.magicBalls1.entities[i]).stateChange = true;
			}
		}
	}

	// Magic Balls2
	for (uint i = 0; i < registry.magicBalls2.entities.size(); i++)
	{
		registry.magicBalls2.get(registry.magicBalls2.entities[i]).duration_ms -= elapsed_ms_since_last_update;
		//timeout duration
		if (registry.magicBalls2.get(registry.magicBalls2.entities[i]).duration_ms <= 0) {
			registry.remove_all_components_of(registry.magicBalls2.entities[i]);
		}

		else
		{
			registry.magicBalls2.get(registry.magicBalls2.entities[i]).render_curr_frame -= elapsed_ms_since_last_update;
			if (registry.magicBalls2.get(registry.magicBalls2.entities[i]).render_curr_frame <= 0.0f)
			{
				// time to update
				registry.magicBalls2.get(registry.magicBalls2.entities[i]).render_curr_frame =
					registry.magicBalls2.get(registry.magicBalls2.entities[i]).render_update_frame;
				registry.magicBalls2.get(registry.magicBalls2.entities[i]).incrementStateIndex();
				registry.magicBalls2.get(registry.magicBalls2.entities[i]).stateChange = true;
			}
		}
	}

	// only spawn if player alive
	if (registry.players.has(player)) {
		if (registry.ghostEnemy.entities.size() < ghost_spawn_limit) {
			if (ghost_spawn_cd > 0) {
				ghost_spawn_cd -= 20;
			}
			if (ghost_spawn_cd <= 0) {
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
				createGhost(renderer, { registry.motions.get(player).position.x + randomX, registry.motions.get(player).position.y + randomY });
				ghost_spawn_cd = set_ghost_spawn_cd;
			}
		}
	}


	// Processing the player state
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
		    min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.motions.get(entity).position.x = window_width_px + 30;
			registry.motions.get(entity).velocity.x = 0;
			movingLeft = false;
			movingRight = false;
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
			switch_state(GameOver);
			return true;
		}
	}

	// Timer for next level
	for (Entity entity : registry.nextLevelTimers.entities) {
		// progress timer
		NextLevelTimer& counter = registry.nextLevelTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// load next level once the timer expires
		if (counter.counter_ms < 0) {
			registry.motions.get(entity).position.x = window_width_px + 30;
			registry.motions.get(entity).velocity.x = 0;
			movingLeft = false;
			movingRight = false;
			inCutscene = false;
			currentSlide = 0;
			registry.nextLevelTimers.remove(entity);
			screen.darken_screen_factor = 0;
			if (game_state == 1) {
				switch_state(Tutorial);
			}
			else if (game_state == 2) {
				switch_state(LevelOne);
				level_start_honor = honorGained;
			}
			else if (game_state == 3) {
				switch_state(SecondCutscene);
			}
			else if (game_state == 4) {
				switch_state(LevelTwo);
				 level_start_honor = honorGained;
			}
			else if (game_state == 5) {
				switch_state(ThirdCutscene);
			}
			else if (game_state == 6) {
				switch_state(BuffRoom);
				level_start_honor = honorGained;
			}
			else if (game_state == 7) {
				switch_state(LevelThree);
				level_start_honor = honorGained;
			}
			else if (game_state == 8) {
				switch_state(Complete);
			}
			return true;
		}
	}

	// reduce window brightness if player dies
	screen.darken_screen_factor = lerp(1, 0, min_counter_ms / 3000);

	// Potion timeout period
	// Timeout disabled for now
	min_counter_ms = 1000.f; 
	for (Entity entity : registry.potions.entities) {
		Potion& counter = registry.potions.get(entity);
		counter.timeout_ms -= elapsed_ms_since_last_update;
		if (counter.timeout_ms < min_counter_ms) {
			min_counter_ms = counter.timeout_ms;
		}

		if (counter.timeout_ms < 0) {
			registry.remove_all_components_of(entity);
			Mix_PlayChannel(-1, potion_disappear_sound, 0);
		}
	}

	// Intervals for enemies to make decisions
	for (Entity entity : registry.attackPathTimers.entities) {
		AttackPathTimer& counter = registry.attackPathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		if (counter.counter_ms < 0) {
			registry.attackPathTimers.remove(entity);
		}
	}
	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	Mix_HaltChannel(0);
	Mix_HaltChannel(2);
	Mix_HaltChannel(1);
	Mix_PlayChannel(0, background_music, -1);
	Mix_Volume(0, MIX_MAX_VOLUME / 15);
	// Debugging for memory/component leaks
	// Updating window title
	std::stringstream title_ss;
	title_ss << "Nighthood" << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());
	renderer->fps_bool = false;
	renderer->help_bool = false;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// NOTE: Things (layers) will be created in order, so order matter
	tiles.clear();
	switch_state(FirstCutscene);
}

void WorldSystem::switch_state(GameState gs) {
	if (gs != GameOver) {
		game_state = gs;
	}

	cleanup();
	switch (gs) {
	case Intro:
		intro();
		break;
	case FirstCutscene:
		firstCutscene();
		break;
	case Tutorial:
		tutorial();
		break;
	case LevelOne:
		level_one();
		break;
	case SecondCutscene:
		secondCutscene();
		break;
	case LevelTwo:
		level_two();
		break;
	case ThirdCutscene:
		thirdCutscene();
		break;
	case BuffRoom:
		buff_room();
		break;
	case LevelThree:
		level_three();
		break;
	case Complete:
		game_complete();
		break;
	case GameOver:
		game_over();
		break;
	}
}

void WorldSystem::intro() {
	game_state = Intro;
	background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::INTRO); // Background
}

void WorldSystem::firstCutscene() {
	inCutscene = true;
	switch (currentSlide) {
		case 0:
			background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE1A);
			break;
		case 1:
			background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE1B);
			break;
		case 2:
			background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE1C);
			break;
		case 3:
			background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE1D);
			break;
		case 4:
			background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE1E);
			break;
		default:
			Entity player = createPlayer(renderer, { -500, -500 }, playerHealth);
			registry.nextLevelTimers.emplace(player);
			registry.nextLevelTimers.get(player).counter_ms = 2000;
			Mix_PlayChannel(-1, next_area_sound, 0);
			whichCutscene++;
	}
}

void WorldSystem::secondCutscene() {
	inCutscene = true;
	switch (currentSlide) {
	case 0:
		background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE2A);
		break;
	case 1:
		background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE2B);
		break;
	case 2:
		background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE2C);
		break;
	default:
		Entity player = createPlayer(renderer, { -500, -500 }, playerHealth);
		registry.nextLevelTimers.emplace(player);
		registry.nextLevelTimers.get(player).counter_ms = 2000;
		Mix_PlayChannel(-1, next_area_sound, 0);
		whichCutscene++;
	}
}

void WorldSystem::thirdCutscene() {
	inCutscene = true;
	switch (currentSlide) {
	case 0:
		background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE3A);
		break;
	case 1:
		background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE3B);
		break;
	case 2:
		background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE3C);
		break;
	case 3:
		background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::CUTSCENE3D);
		break;
	default:
		Entity player = createPlayer(renderer, { -500, -500 }, playerHealth);
		registry.nextLevelTimers.emplace(player);
		registry.nextLevelTimers.get(player).counter_ms = 2000;
		Mix_PlayChannel(-1, next_area_sound, 0);
		whichCutscene++;
	}
}

void WorldSystem::tutorial() {
	gameStarted = true;
	// Create floors and walls
	// Divided into columns, then quadrants (A1 = top quadrant of leftmost column, H3 = bottom quadrant of rightmost column)
	// Quadrants are 1280 x 1000 in size
	background = createBackground(renderer, { window_width_px / 2.f, window_height_px / 2.f }, 1); // Background
	background_cloud.clear();
	background_tree.clear();

	ghost_spawn_limit = 0;
	set_ghost_spawn_cd = 4000;
	ghost_spawn_cd = set_ghost_spawn_cd;

	float cloud_rightEdge = cloud_width / 2.f;
	while (cloud_rightEdge - cloud_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_cloud.push_back(createBackgroundCloud(
			renderer, { cloud_rightEdge - cloud_width / 2.0f, cloud_height / 2.f }));
		cloud_rightEdge += cloud_width;
	}
	float tree_rightEdge = tree_width / 2.f;
	while (tree_rightEdge - tree_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_tree.push_back(createBackgroundTrees(
			renderer, { tree_rightEdge - tree_width / 2.0f, window_height_px - tree_height / 2.f }));
		tree_rightEdge += tree_width;
	}

	// Next level box
	Entity nextLevel = createCustomTile(renderer, { 20, 370 }, { 60, 90 });
	registry.tiles.remove(nextLevel);
	registry.nextLevels.emplace(nextLevel);
	registry.colors.insert(nextLevel, { 0.3f, 0.3f, 0.3f });

	tiles.push_back(createCustomTile(renderer, { 640, 1050 }, { 1280, 300 })); // Floor all the way down
	tiles.push_back(createCustomTile(renderer, { 1640, 1050 }, { 1280, 300 })); // Floor all the way down
	tiles.push_back(createCustomTile(renderer, { 1080, 700 }, { 240, 79 })); // Lower platform
	tiles.push_back(createCustomTile(renderer, { 790, 450 }, { 320, 79 })); // Higher platform
	tiles.push_back(createCustomTile(renderer, { 315, 450 }, { 640, 79 })); // Higher floor
	tiles.push_back(createVerticalTile(renderer, { -100, 100 }, { 240, 640 })); // Higher floor wall
	for (int i = 0; i < 2; i++)
	{
		tiles.push_back(createVerticalTile(renderer, { -150, 50 + 1280 * i }, { 300, 1280 }));// Left Wall all the way down
	}
	for (int i = 0; i < 2; i++)
	{
		tiles.push_back(createVerticalTile(renderer, { 1350, 50 + 1280 * i }, { 300, 1280 }));// Right Wall all the way down
	}

	createDoor(renderer, { 20, 360 }, { 70, 100 }); // Door to next level

	// Controls sign
	Entity sign1 = createStatue(renderer, { 100, 850 });
	registry.readables.get(sign1).lines.push_back("Press 'H' key to see player controls");
	registry.readables.get(sign1).lines.push_back("on the bottom left of the screen.");

	// Potion sign
	Entity sign2 = createStatue(renderer, { 1000, 850 });
	registry.readables.get(sign2).lines.push_back("Healing flasks can be scavenged in the cursed lands.");
	registry.readables.get(sign2).lines.push_back("Take care to not neglect their presence.");
	registry.readables.get(sign2).lines.push_back("The denizens of the land will steal them in time.");

	for (Entity tile : tiles) {
		registry.colors.insert(tile, { 0.3f, 0.3f, 0.3f });
	}

	//Player
	player = createPlayer(renderer, { 100, 880 }, playerHealth);
	registry.colors.insert(player, { 1, 0.8f, 0.8f });

	// Player hearts
	update_hearts();

	// Potions
	createPotion(renderer, { 1150, 885 }, Heal);

	save_game();
}

void WorldSystem::level_one() {
	displayHonor = true;
	gameStarted = true;
	showBossHealth = false;
	// Create floors and walls
	// Divided into columns, then quadrants (A1 = top quadrant of leftmost column, H3 = bottom quadrant of rightmost column)
	// Quadrants are 1280 x 1000 in size
	background = createBackground(renderer, { window_width_px / 2.f, window_height_px / 2.f }, 2); // Background
	background_cloud.clear();
	background_tree.clear();

	ghost_spawn_limit = 1;
	set_ghost_spawn_cd = 3000;
	ghost_spawn_cd = set_ghost_spawn_cd;

	float cloud_rightEdge = cloud_width / 2.f;
	while (cloud_rightEdge - cloud_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_cloud.push_back(createBackgroundCloud(
			renderer, { cloud_rightEdge - cloud_width / 2.0f, cloud_height / 2.f }));
		cloud_rightEdge += cloud_width;
	}
	float tree_rightEdge = tree_width / 2.f;
	while (tree_rightEdge - tree_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_tree.push_back(createBackgroundTrees(
			renderer, { tree_rightEdge - tree_width / 2.0f, window_height_px - tree_height / 2.f }));
		tree_rightEdge += tree_width;
	}

	// Next level box
	Entity nextLevel = createCustomTile(renderer, { 10290, 2530 }, { 70, 120 });
	registry.tiles.remove(nextLevel);
	registry.nextLevels.emplace(nextLevel);
	registry.colors.insert(nextLevel, { 0.3f, 0.3f, 0.3f });

	// A
	tiles.push_back(createCustomTile(renderer, { 640, 1000 }, { 1280, 300 })); // Floor all the way down
	for (int i = 0; i < 2; i++)
	{
		tiles.push_back(createVerticalTile(renderer, { -150, 500+1280*i }, { 300, 1280 }));// Left Wall all the way down
	}

	// B
	for (int i = 0; i < 8; i++)
	{
		tiles.push_back(createCustomTile(renderer, { 1620, 1000 + 300*i }, { 1280, 300 })); // Floor all the way down
	}

	// C
	// C2
	tiles.push_back(createCustomTile(renderer, { 3225, 1115 }, { 600, 40 })); // Top platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 2810, 1365 }, { 600, 40 })); // Second platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 3225, 1615 }, { 600, 40 })); // Third platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 2810, 1865 }, { 600, 40 })); // Fourth platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 3325, 2115 }, { 550, 40 })); // Last platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 2710, 2665 }, { 600, 720 })); // Tallest raised ground (DONE)
	tiles.push_back(createCustomTile(renderer, { 2860, 2765 }, { 600, 480 })); // Middle raised ground (DONE)
	tiles.push_back(createCustomTile(renderer, { 3050, 2865 }, { 600, 480 })); // Lowest raised ground (DONE)
	tiles.push_back(createVerticalLongTile(renderer, { 2410, 2130 }, { 300, 2560 }));// Left Wall all the way down

	// C3
	tiles.push_back(createCustomTile(renderer, { 3200, 3000 }, { 1300, 320 })); // Bottom floor (DONE)

	// D
	for (int i = 0; i < 4; i++)
	{
		tiles.push_back(createCustomTile(renderer, { 4300, 1000 + 300*i }, { 1280, 300 })); // Floor all the way down
	}
	tiles.push_back(createVerticalTile(renderer, { 3510, 1510 }, { 300, 1300 }));// Left Wall all the way down
	tiles.push_back(createCustomTile(renderer, { 4000, 1000 }, { 1280, 300 })); // Floor all the way down

	// D3
	tiles.push_back(createCustomTile(renderer, { 4170, 2960 }, { 650, 240 })); // Left bottom floor (DONE)
	tiles.push_back(createCustomTile(renderer, { 5210, 2960 }, { 650, 240 })); // Right bottom floor (DONE)

	// E
	tiles.push_back(createCustomTile(renderer, { 5280, 1000 }, { 1280, 300 }));
	tiles.push_back(createCustomTile(renderer, { 6560, 1000 }, { 1280, 300 }));
	for (int i = 0; i < 4; i++)
	{
		tiles.push_back(createCustomTile(renderer, { 6800, 950 + 200*i }, { 1280, 200 }));
	}
	tiles.push_back(createCustomTile(renderer, { 8350, 1115 }, { 600, 40 })); // First platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 7780, 1365 }, { 600, 40 })); // Second platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 8400, 1615 }, { 600, 40 })); // Third platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 7780, 1870 }, { 600, 40 })); // Fourth platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 8400, 2115 }, { 600, 40 })); // Fifth platform (DONE)
	tiles.push_back(createCustomTile(renderer, { 7780, 2365 }, { 600, 40 })); // Sixth platform (DONE)
	tiles.push_back(createVerticalTile(renderer, { 7590, 1380 }, { 300, 1060 }));// Left Wall all the way down

	// E3
	tiles.push_back(createCustomTile(renderer, { 5400, 2960 }, { 700, 240 })); // Left bottom floor (DONE)

	// F3
	tiles.push_back(createCustomTile(renderer, { 6800, 2960 }, { 1500, 240 })); // Left bottom floor (DONE)

	// G3
	tiles.push_back(createCustomTile(renderer, { 8440, 2725 }, { 1280, 300 })); // Platform

	// H
	for (int i = 0; i < 4; i++)
	{
		tiles.push_back(createCustomTile(renderer, { 9600, 1000 + 300*i }, { 1800, 300 })); // Floor all the way down
	}
	tiles.push_back(createVerticalTile(renderer, { 8550, 1505 }, { 300, 1310 }));// Left Wall all the way down
	tiles.push_back(createCustomTile(renderer, { 9140, 2725 }, { 1280, 300 })); // Floor from middle of H3 to bottom
	tiles.push_back(createCustomTile(renderer, { 10420, 2725 }, { 1280, 300 }));
	for (int i = 0; i < 3; i++)
	{
		tiles.push_back(createVerticalTile(renderer, { 10400, 500 + 1280 * i }, { 300, 1280 }));// Right Wall down to level exit
	}
	for (Entity tile : tiles) {
		registry.colors.insert(tile, { 0.3f, 0.3f, 0.3f });
	}

	createDoor(renderer, { 10250, 2525 }, { 70, 100 }); // Door to next level

	// Deathbox
	Entity deathbox = createCustomTile(renderer, { 5120, 5000 }, { 10240, 4000 });
	registry.tiles.remove(deathbox);
	registry.deathboxes.emplace(deathbox);
	registry.colors.insert(deathbox, { 0.f, 0.f, 0.f });

	// Enemy sign
	Entity sign1 = createStatue(renderer, { 400, 800 });
	registry.readables.get(sign1).lines.push_back("The path ahead rewards the righteous.");
	registry.readables.get(sign1).lines.push_back("Defeating foes reclaims thy honor.");
	registry.readables.get(sign1).lines.push_back("Honor glows a different shade with each milestone.");
	registry.readables.get(sign1).lines.push_back("Raise thy honor level to receive boons in the future.");

	//Player
	player = createPlayer(renderer, { 100,800 }, playerHealth);
	registry.colors.insert(player, { 1, 0.8f, 0.8f });

	// Player hearts
	update_hearts();

	// Potions
	createPotion(renderer, { 10200, 835 }, Heal);
	createPotion(renderer, { 5500, 2871 }, Heal);
	createPotion(renderer, { 3320, 2080 }, Heal);

	// Bats
	createBat(renderer, { 4500,2600 }, 150);
	createBat(renderer, { 5900,2600 }, 150);

	// Skeletons
	Entity skeleton1 = createSkeleton(renderer, { 800,775 }, 200, 500); //start position (x,y), attack range, roaming range
	Entity skeleton2 = createSkeleton(renderer, { 1500,775 }, 200, 500); //start position (x,y), attack range, roaming range
	Entity skeleton3 = createSkeleton(renderer, { 5500,775 }, 200, 600); //start position (x,y), attack range, roaming range
	Entity skeleton4 = createSkeleton(renderer, { 5300,2770 }, 200, 300); //start position (x,y), attack range, roaming range
	registry.skeletonEnemy.get(skeleton1).stateChange = true;
	registry.skeletonEnemy.get(skeleton2).stateChange = true;
	registry.skeletonEnemy.get(skeleton3).stateChange = true;
	registry.skeletonEnemy.get(skeleton4).stateChange = true;

	createWolf(renderer, { 7000, 800 }, 400, 2000);
	createWolf(renderer, { 9200, 800 }, 400, 2000);
	createWolf(renderer, { 6900, 2795 }, 400, 2000);

	// Mushrooms
	Entity rangedEnemy1 = createRangedEnemy(renderer, { 3800, 780 }, 500, 600, true); //start position (x,y), attack range, roaming range, STATIONARY?
	Entity rangedEnemy2 = createRangedEnemy(renderer, { 7780, 2275 }, 500, 600, true); //start position (x,y), attack range, roaming range, STATIONARY?
	registry.rangedEnemy.get(rangedEnemy1).stateChange = true;
	registry.rangedEnemy.get(rangedEnemy2).stateChange = true;

	Motion& temp = registry.motions.get(player);

	save_game();
}

void WorldSystem::level_two() {
	displayHonor = true;
	gameStarted = true;
	showBossHealth = false;
	// Create floors and walls
	// Divided into columns, then quadrants (A1 = top quadrant of leftmost column, H3 = bottom quadrant of rightmost column)
	// Quadrants are 1280 x 1000 in size
	background = createBackground(renderer, { window_width_px / 2.f, window_height_px / 2.f }, 3); // Background
	background_cloud.clear();
	background_tree.clear();

	ghost_spawn_limit = 3;
	set_ghost_spawn_cd = 6000;
	ghost_spawn_cd = set_ghost_spawn_cd;

	float cloud_rightEdge = cloud_width / 2.f;
	while (cloud_rightEdge - cloud_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_cloud.push_back(createBackgroundCloud(
			renderer, { cloud_rightEdge - cloud_width / 2.0f, cloud_height / 2.f }));
		cloud_rightEdge += cloud_width;
	}
	float tree_rightEdge = tree_width / 2.f;
	while (tree_rightEdge - tree_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_tree.push_back(createBackgroundTrees(
			renderer, { tree_rightEdge - tree_width / 2.0f, window_height_px - tree_height / 2.f }));
		tree_rightEdge += tree_width;
	}

	// Next level box
	Entity nextLevel = createCustomTile(renderer, { 7240, 1955 }, { 70, 120 });
	registry.tiles.remove(nextLevel);
	registry.nextLevels.emplace(nextLevel);
	registry.colors.insert(nextLevel, { 0.3f, 0.3f, 0.3f });

	// A
	tiles.push_back(createVerticalLongTile(renderer, { -300, 900 }, { 600, 2400 })); // Left Wall all the way down (DONE)
	// A1
	tiles.push_back(createCustomTile(renderer, { 640, 950 }, { 1280, 300 })); // Floor (DONE)
	// A2-A4
	tiles.push_back(createCustomTile(renderer, { 725, 1975 }, { 1450, 300 })); // Floor (DONE)
	
	// B platforms
	tiles.push_back(createCustomTile(renderer, { 2450, 1100 }, { 600, 40 })); // Top platform
	tiles.push_back(createCustomTile(renderer, { 1750, 1350 }, { 600, 40 })); // Second platform
	tiles.push_back(createCustomTile(renderer, { 2450, 1600 }, { 600, 40 })); // Third platform
	tiles.push_back(createCustomTile(renderer, { 1750, 1845 }, { 600, 40 })); // Fourth platform
	tiles.push_back(createCustomTile(renderer, { 2450, 2125 }, { 600, 40 })); // Fifth platform
	tiles.push_back(createCustomTile(renderer, { 1750, 2375 }, { 600, 40 })); // Sixth platform
	tiles.push_back(createCustomTile(renderer, { 2275, 2625 }, { 400, 40 })); // Seventh platform 
	tiles.push_back(createCustomTile(renderer, { 2650, 3070 }, { 600, 40 })); // Fifth platform

	// B1
	tiles.push_back(createCustomTile(renderer, { 2100, 600 }, { 600, 40 })); // Platform
	// B1/B2
	tiles.push_back(createVerticalTile(renderer, { 1480, 1095 }, { 400, 590 })); // Wall
	tiles.push_back(createCustomTile(renderer, { 1650, 1865 }, { 400, 80 })); // Fourth platform
	// B3 raised floors
	tiles.push_back(createCustomTile(renderer, { 2000, 3550 }, { 400, 560 })); // Taller raised floor
	tiles.push_back(createCustomTile(renderer, { 2255, 3700 }, { 400, 310 })); // Lower raised floor
	tiles.push_back(createVerticalLongTile(renderer, { 1600, 2905 }, { 500, 2000 })); // Left Wall all the way down (DONE)

	// C1/C2
	tiles.push_back(createVerticalTile(renderer, { 2700, 1480 }, { 400, 1360 })); // Wall

	// D
	// D2
	tiles.push_back(createCustomTile(renderer, { 5600, 2175 }, { 300, 40 })); // Level 2 platform 
	// D3
	tiles.push_back(createCustomTile(renderer, { 5560, 2725 }, { 150, 160 })); // Level 3 platform 

	// E
	// E2
	tiles.push_back(createCustomTile(renderer, { 6600, 2175 }, { 300, 40 })); // Level 2 platform 
	// E3
	tiles.push_back(createCustomTile(renderer, { 6475, 2725 }, { 150, 160 })); // Level 3 platform 

	// F4
	tiles.push_back(createVerticalTile(renderer, { 7800, 3445 }, { 300, 710 })); // Wall

	// G platforms
	tiles.push_back(createCustomTile(renderer, { 8650, 1100 }, { 600, 40 })); // Top platform
	tiles.push_back(createCustomTile(renderer, { 8200, 1350 }, { 300, 40 })); // Second platform
	tiles.push_back(createCustomTile(renderer, { 8650, 1600 }, { 600, 40 })); // Third platform
	tiles.push_back(createCustomTile(renderer, { 8200, 1865 }, { 300, 40 })); // Fourth platform
	tiles.push_back(createCustomTile(renderer, { 8750, 2125 }, { 600, 40 })); // Fifth platform
	tiles.push_back(createCustomTile(renderer, { 8200, 2360 }, { 300, 40 })); // Sixth platform
	tiles.push_back(createCustomTile(renderer, { 8750, 2725 }, { 600, 360 })); // Seventh platform

	// H
	// H3
	tiles.push_back(createVerticalLongTile(renderer, { 9000, 1920 }, { 400, 2200 })); // Wall/Floor
	tiles.push_back(createVerticalTile(renderer, { 9400,700 }, { 400, 1000 })); // Wall/Floor


	// C1-D1 floor
	tiles.push_back(createCustomTile(renderer, { 3400, 960 }, { 1200, 320 }));
	// D1-E1 floor
	tiles.push_back(createCustomTile(renderer, { 4600, 960 }, { 1200, 320 }));
	// E1-F1 floor
	tiles.push_back(createCustomTile(renderer, { 5800, 960 }, { 1200, 320 }));
	// E1-F1 floor
	tiles.push_back(createCustomTile(renderer, { 7000, 960 }, { 1200, 320 }));
	// C2-D2 floor
	tiles.push_back(createCustomTile(renderer, { 4860 - 1280 - 440, 2150 }, { 1280, 300 }));
	for (int i = 0; i < 2; i++)
	{
		tiles.push_back(createCustomTile(renderer, { 4860 - i*1280, 2150 }, { 1280, 300 }));
	}
	// E2-F2 floor
	tiles.push_back(createCustomTile(renderer, { 7175, 2080 }, { 850, 160 }));
	tiles.push_back(createCustomTile(renderer, { 7425, 1920 }, { 350, 160 }));

	// F
	// F1/F2
	tiles.push_back(createVerticalLongThickTile(renderer, { 7650, 1360 }, { 900, 1120 })); // Wall

	// B3-D3 floor
	for (int i = 0; i < 2; i++)
	{
		tiles.push_back(createCustomTile(renderer, { 3555 + i * 1280, 2935 }, { 1600, 300 }));
	}
	
	// D3-E3 platform
	tiles.push_back(createCustomTile(renderer, { 6100, 2425 }, { 300, 40 }));
	// E3-H3 floor
	for (int i = 0; i < 2; i++)
	{
		tiles.push_back(createCustomTile(renderer, { 7040 + i*1280, 2940 }, { 1280, 300 }));
	}
	
	// B4-D4 floor
	for (int i = 0; i < 3; i++)
	{
		tiles.push_back(createCustomTile(renderer, { 2575 + i * 1280, 3950 }, { 1450, 300 }));
	}
	// E4-F4 floor
	tiles.push_back(createCustomTile(renderer, { 7200, 3960 }, { 1500, 320 }));

	for (Entity tile : tiles) {
		registry.colors.insert(tile, { 0.3f, 0.3f, 0.3f });
	}

	// Deathbox
	Entity deathbox = createCustomTile(renderer, { 6200, 13000 }, { 10240, 18000 });
	registry.tiles.remove(deathbox);
	registry.deathboxes.emplace(deathbox);
	registry.colors.insert(deathbox, { 0.f, 0.f, 0.f });

	createDoor(renderer, { 7200, 1945 }, { 70, 110 }); // Door to next level

	//Player
	player = createPlayer(renderer, { 100, 800 }, 3);
	registry.colors.insert(player, { 1, 0.8f, 0.8f });

	// Player hearts
	update_hearts();

	// Saws
	createSaw(renderer, { 5450, 2750 });
	createSaw(renderer, { 6600, 2750 });

	// Potions
	createPotion(renderer, { 700, 1810 }, Heal);
	createPotion(renderer, { 2720, 3035 }, Heal);
	createPotion(renderer, { 3200, 1980 }, Heal);
	createPotion(renderer, { 5120, 780 }, Heal);
	createPotion(renderer, { 8760, 2530 }, Heal);
	createPotion(renderer, { 9060, 800 }, Heal);
	createPotion(renderer, { 7450, 3780 }, Heal);

	// Bats
	createBat(renderer, { 1400,1600 }, 150);
	createBat(renderer, { 7200,2600 }, 150);
	createBat(renderer, { 5700,3600 }, 150);

	// Wolves
	createWolf(renderer, { 7200, 2740 }, 400, 2000);
	createWolf(renderer, { 3800, 3750 }, 400, 2000);
	createWolf(renderer, { 5100, 3750 }, 400, 2000);
	createWolf(renderer, { 7100, 3750 }, 400, 2000);
	
	// Demon
	float left_b = 2600.f;
	float right_b = 7900.f;
	createDemon(renderer, { 5120, 680 }, left_b + DEMON_WIDTH / 2.0f, right_b - DEMON_WIDTH / 2.0f);
	
	// Skeletons
	Entity skeleton1 = createSkeleton(renderer, { 750,1750 }, 300, 200); //start position (x,y), attack range, roaming range
	Entity skeleton2 = createSkeleton(renderer, { 4500,2710 }, 300, 600); //start position (x,y), attack range, roaming range
	Entity skeleton3 = createSkeleton(renderer, { 3500,1925 }, 300, 600); //start position (x,y), attack range, roaming range
	registry.skeletonEnemy.get(skeleton1).stateChange = true;
	registry.skeletonEnemy.get(skeleton2).stateChange = true;
	registry.skeletonEnemy.get(skeleton3).stateChange = true;

	// Wizards
	Entity wizard1 = createWizard(renderer, { 9000,770 }, 500, 600, true); //start position (x,y), attack range, roaming range, STATIONARY?
	Entity wizard2 = createWizard(renderer, { 2100,530 }, 500, 600, true); //start position (x,y), attack range, roaming range, STATIONARY?
	registry.wizards.get(wizard1).stateChange = true;
	registry.wizards.get(wizard2).stateChange = true;

	// Mushrooms
	Entity rangedEnemy1 = createRangedEnemy(renderer, { 7450,3730 }, 500, 600, true); //start position (x,y), attack range, roaming range, STATIONARY?
	registry.rangedEnemy.get(rangedEnemy1).stateChange = true;

	Motion& temp = registry.motions.get(player);

	save_game();
}

void WorldSystem::buff_room() {
	gameStarted = true;
	displayHonor = false;
	showBossHealth = false;

	ghost_spawn_limit = 0;
	set_ghost_spawn_cd = 6000;
	ghost_spawn_cd = set_ghost_spawn_cd;

	// Create floors and walls
	// Divided into columns, then quadrants (A1 = top quadrant of leftmost column, H3 = bottom quadrant of rightmost column)
	// Quadrants are 1280 x 1000 in size
	background = createBackground(renderer, { window_width_px / 2.f, window_height_px / 2.f }, 1); // Background
	background_cloud.clear();
	background_tree.clear();
	float cloud_rightEdge = cloud_width / 2.f;
	while (cloud_rightEdge - cloud_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_cloud.push_back(createBackgroundCloud(
			renderer, { cloud_rightEdge - cloud_width / 2.0f, cloud_height / 2.f }));
		cloud_rightEdge += cloud_width;
	}
	float tree_rightEdge = tree_width / 2.f;
	while (tree_rightEdge - tree_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_tree.push_back(createBackgroundTrees(
			renderer, { tree_rightEdge - tree_width / 2.0f, window_height_px - tree_height / 2.f }));
		tree_rightEdge += tree_width;
	}

	// Next level box
	Entity nextLevel = createCustomTile(renderer, { 70, 370 }, { 60, 90 });
	registry.tiles.remove(nextLevel);
	registry.nextLevels.emplace(nextLevel);
	registry.colors.insert(nextLevel, { 0.3f, 0.3f, 0.3f });

	tiles.push_back(createCustomTile(renderer, { 640, 1050 }, { 1280, 300 })); // Floor all the way down
	tiles.push_back(createCustomTile(renderer, { 1640, 1050 }, { 1280, 300 })); // Floor all the way down
	tiles.push_back(createCustomTile(renderer, { 1100, 750 }, { 320, 80 })); // Lower platform
	tiles.push_back(createCustomTile(renderer, { 720, 500 }, { 320, 80 })); // Higher platform
	tiles.push_back(createCustomTile(renderer, { 1100, 250 }, { 320, 80 })); // Highest platform
	tiles.push_back(createCustomTile(renderer, { 250, 500 }, { 640, 80 })); // Higher floor
	tiles.push_back(createVerticalTile(renderer, { -50, 140 }, { 240, 640 })); // Higher floor wall
	for (int i = 0; i < 2; i++)
	{
		tiles.push_back(createVerticalTile(renderer, { -150, 500 + 1280 * i }, { 300, 1280 }));// Left Wall all the way down
	}
	for (int i = 0; i < 2; i++)
	{
		tiles.push_back(createVerticalTile(renderer, { 1350, 500 + 1280 * i }, { 300, 1280 }));// Left Wall all the way down
	}

	createDoor(renderer, { 70, 410 }, { 70, 100 }); // Door to next level

	createPedestal(renderer, { 1150, 875 });
	createPedestal(renderer, { 1150, 185 });

	// Attack buff sign
	Entity sign1 = createStatue(renderer, { 1000, 850 });
	if (honorGained < 12) {
		registry.readables.get(sign1).lines.push_back("Thou'rt short of honor.");
	}
	else {
		registry.readables.get(sign1).lines.push_back("Honor creates bastions against the dark.");
		registry.readables.get(sign1).lines.push_back("Your honor protects you against the coming evil.");
		registry.readables.get(sign1).lines.push_back("(Pick up this gem to gain +3 total health)");

		createDefenseBuff(renderer, { 1150, 840 });
	}
	// Defense sign
	Entity sign2 = createStatue(renderer, { 1000, 160 });
	if (honorGained < 20) {
		registry.readables.get(sign2).lines.push_back("Thou'rt short of honor.");
	}
	else {
		registry.readables.get(sign2).lines.push_back("Honor is a weapon against the unholy.");
		registry.readables.get(sign2).lines.push_back("Your honor vanquishes all foes who stand against you.");
		registry.readables.get(sign2).lines.push_back("(Pick up this gem to gain +2 attack)");

		createAttackBuff(renderer, { 1150, 150 });
	}

	for (Entity tile : tiles) {
		registry.colors.insert(tile, { 0.3f, 0.3f, 0.3f });
	}

	//Player
	player = createPlayer(renderer, { 100, 880 }, playerHealth);
	registry.colors.insert(player, { 1, 0.8f, 0.8f });

	// Player hearts
	update_hearts();

	save_game();
}

void WorldSystem::level_three() {
	// Create floors and walls
	// Divided into columns, then quadrants (A1 = top quadrant of leftmost column, H3 = bottom quadrant of rightmost column)
	// Quadrants are 1280 x 1000 in size
	showBossHealth = true;
	background = createBackground(renderer, { window_width_px / 2.f, window_height_px / 2.f }, 3); // Background
	background_cloud.clear();
	background_tree.clear();
	// music
	Mix_HaltChannel(0);
	Mix_PlayChannel(1, boss_music, -1);
	Mix_Volume(1, MIX_MAX_VOLUME / 10);

	ghost_spawn_limit = 0;
	set_ghost_spawn_cd = 10000;
	ghost_spawn_cd = set_ghost_spawn_cd;

	float cloud_rightEdge = cloud_width / 2.f;
	while (cloud_rightEdge - cloud_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_cloud.push_back(createBackgroundCloud(
			renderer, { cloud_rightEdge - cloud_width / 2.0f, cloud_height / 2.f }));
		cloud_rightEdge += cloud_width;
	}
	float tree_rightEdge = tree_width / 2.f;
	while (tree_rightEdge - tree_width * 2.f < window_width_px)
	{
		// fill the sky with clouds
		background_tree.push_back(createBackgroundTrees(
			renderer, { tree_rightEdge - tree_width / 2.0f, window_height_px - tree_height / 2.f }));
		tree_rightEdge += tree_width;
	}

	// Next level box
	Entity nextLevel = createCustomTile(renderer, { 7240, 1955 }, { 70, 120 });
	registry.tiles.remove(nextLevel);
	registry.nextLevels.emplace(nextLevel);
	registry.colors.insert(nextLevel, { 0.3f, 0.3f, 0.3f });

	// A
	tiles.push_back(createVerticalLongTile(renderer, { -265, 500 }, { 850, 2000 })); // Left Wall all the way down (DONE)
	// A1
	for (int i = 0; i < 2; i++)
	{
		tiles.push_back(createCustomTile(renderer, { 800+1280*i, 1000 }, { 1280, 400 })); // Floor (DONE)
	}
	tiles.push_back(createVerticalLongTile(renderer, { 3145, 500 }, { 850, 2000 })); // Left Wall all the way down (DONE)
	// platforms
	tiles.push_back(createCustomTile(renderer, { 1000, 600 }, { 500, 40 }));
	tiles.push_back(createCustomTile(renderer, { 2000, 600 }, { 500, 40 }));

	Entity Boss = createGolem(renderer, { 1500, 500 });
	bossHealth = registry.golem.get(Boss).health;

	player = createPlayer(renderer, { 300, 700 }, playerHealth);
	registry.colors.insert(player, { 1, 0.8f, 0.8f });

	Player& p = registry.players.get(player);

	if (attackBuffGained) {
		p.damage = 2;
	}
	if (defenseBuffGained) {
		p.shield = 3;
	}

	// Player hearts
	update_hearts();
	update_shields();


	save_game();
}

void WorldSystem::update_hearts() {
	int& health = registry.players.get(player).health;
	if (health != 0 && registry.player_health.entities.size() == 0) {
		createHeart(renderer, { window_width_px / 2.f, (window_height_px / 2.f) + 100.f }, { 510, -550 }, { 15.f, 20.f });
		createHeart(renderer, { window_width_px / 2.f, (window_height_px / 2.f) + 100.f }, { 560, -550 }, { 15.f, 20.f });
		createHeart(renderer, { window_width_px / 2.f, (window_height_px / 2.f) + 100.f }, { 610, -550 }, { 15.f, 20.f });
	}
	std::vector<Entity>& e_hearts = registry.player_health.entities;
	std::vector<Heart>& hearts = registry.player_health.components;
	for (int i = 0; i < e_hearts.size(); i++) {
		if (i >= health) {
			hearts[i].state_index = 3;
		}
		else {
			hearts[i].state_index = 1;
		}
	}
}

void WorldSystem::update_shields() {
	int& shield = registry.players.get(player).shield;

	// 
	if (shield == 0) {
		while (registry.shields.entities.size() > 0)
			registry.remove_all_components_of(registry.shields.entities.back());

		defenseBuffGained = false;
	}
	if (shield > 0 && registry.shields.entities.size() == 0) {
		createShield(renderer, { window_width_px / 2.f, (window_height_px / 2.f) + 150.f }, { 510, -550 }, { 15.f, 20.f });
		createShield(renderer, { window_width_px / 2.f, (window_height_px / 2.f) + 150.f }, { 560, -550 }, { 15.f, 20.f });
		createShield(renderer, { window_width_px / 2.f, (window_height_px / 2.f) + 150.f }, { 610, -550 }, { 15.f, 20.f });
	}

	std::vector<Entity>& e_shields = registry.shields.entities;
	std::vector<Shield>& shields = registry.shields.components;
	for (int i = 0; i < e_shields.size(); i++) {
		if (i >= shield) {
			shields[i].state_index = 3;
		}
		else {
			shields[i].state_index = 1;
		}
	}
}

void WorldSystem::update_health(int change) {
	int& health = registry.players.get(player).health;
	int& shield = registry.players.get(player).shield;

	// Potion healing
	if (change > 0 && health < 3) {
		health += change;
		if (health > 3) {
			health = 3;
		}
	}

	// Taking damage
	if (change < 0) {
		if (shield > 0 && registry.shields.size() == 3) {
			shield += change;
		} else if (shield < 0) {
			health += shield;
			shield = 0;
		}
		else {
			health += change;
		}
	}


	update_hearts();
	update_shields();
}

void WorldSystem::game_over() {
	showBossHealth = false;
	background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::GAMEOVER); // Background
}

void WorldSystem::game_complete() {
	showBossHealth = false;
	background = createScene(renderer, { window_width_px / 2.f, window_height_px / 2.f }, TEXTURE_ASSET_ID::COMPLETE); // Background
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	if (registry.players.has(player)) {
		// Loop over all collisions detected by the physics system
		auto& collisionsRegistry = registry.collisions;
		bool player_tile_land = false;
		for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
			// The entity and its collider
			Entity entity = collisionsRegistry.entities[i];
			Entity entity_other = collisionsRegistry.components[i].other;

			// attack collision
			if (registry.player_attack1.has(entity) || registry.player_attack2.has(entity))
			{

				// attack collision on golem
				if (registry.golem.has(entity_other))
				{
					if (registry.golem.get(entity_other).immunity_duration <= 0.f)
					{
						registry.golem.get(entity_other).health -= registry.players.get(registry.players.entities[0]).damage;
						bossHealth -= registry.players.get(registry.players.entities[0]).damage;
						Mix_PlayChannel(-1, enemy_take_damage, 0);
					}

					// kill enemy
					if (registry.golem.get(entity_other).health <= 0) {
						if (registry.golem.get(entity_other).alive) {
							registry.golem.get(entity_other).changeState("dead", true);
							createPotion(renderer, { 5120, 500 }, Heal);
						}
						registry.golem.get(entity_other).alive = false;
					}
					// enemy still alive
					else {
						registry.golem.get(entity_other).immunity_duration = registry.golem.get(entity_other).immunity_reset_duration;
					}
				}
				// attack collision on arm projectile
				if (registry.armProjectile.has(entity_other))
				{
					registry.remove_all_components_of(entity_other);
				}

				// attack collision on boss demon
				if (registry.demonBoss.has(entity_other))
				{
					if (registry.demonBoss.get(entity_other).health > 0)
					{
						if (registry.demonBoss.get(entity_other).immunity_duration <= 0.f)
						{
							registry.demonBoss.get(entity_other).health -= registry.players.get(registry.players.entities[0]).damage;
							Mix_PlayChannel(-1, enemy_take_damage, 0);
						}
						// kill enemy
						if (registry.demonBoss.get(entity_other).health <= 0) {
							Mix_PlayChannel(-1, boss_death_sound, 0);
							registry.demonBoss.get(entity_other).changeState("dead", true);
							honorGained += registry.demonBoss.get(entity_other).honor;
						}
						// enemy still alive
						else {
							registry.demonBoss.get(entity_other).immunity_duration = registry.demonBoss.get(entity_other).immunity_reset_duration;
						}
					}
				}

				// attack collision on ghost
				if (registry.ghostEnemy.has(entity_other))
				{
					Mix_PlayChannel(-1, enemy_take_damage, 0);
					registry.remove_all_components_of(entity_other);
				}

				// attack collision on wolf

				if (registry.wolfEnemy.has(entity_other)) {
					if (registry.wolfEnemy.get(entity_other).immunity_duration_ms <= 0) {
						registry.wolfEnemy.get(entity_other).health -= registry.players.get(registry.players.entities[0]).damage;
						Mix_PlayChannel(-1, enemy_take_damage, 0);

						// kill enemy
						if (registry.wolfEnemy.get(entity_other).health <= 0) {
							honorGained += registry.wolfEnemy.get(entity_other).honor;
							registry.remove_all_components_of(entity_other);
						}
						// enemy still alive
						else {
							registry.wolfEnemy.get(entity_other).immunity_duration_ms = 500.f;
							if (registry.motions.get(registry.players.entities[0]).position.x < registry.motions.get(entity_other).position.x) {
								registry.motions.get(entity_other).position.x += 75;
							}
							if (registry.motions.get(registry.players.entities[0]).position.x > registry.motions.get(entity_other).position.x) {
								registry.motions.get(entity_other).position.x -= 75;
							}
						}
					}
				}

				// ATTACK COLLISION ON RANGED ENEMY
				if (registry.rangedEnemy.has(entity_other)) {
					Mix_PlayChannel(-1, enemy_take_damage, 0);
					honorGained += registry.rangedEnemy.get(entity_other).honor;
					registry.remove_all_components_of(entity_other);
				}

				// ATTACK COLLISION ON SKELETON
				if (registry.skeletonEnemy.has(entity_other))
				{
					if (registry.skeletonEnemy.get(entity_other).immunity_duration_ms <= 0) {
						registry.skeletonEnemy.get(entity_other).health -= registry.players.get(registry.players.entities[0]).damage;
						Mix_PlayChannel(-1, enemy_take_damage, 0);

						// kill enemy
						if (registry.skeletonEnemy.get(entity_other).health <= 0) {
							honorGained += registry.skeletonEnemy.get(entity_other).honor;
							registry.remove_all_components_of(entity_other);
						}
						// enemy still alive
						else {
							registry.skeletonEnemy.get(entity_other).immunity_duration_ms = 500.f;
							// KnockBack enemy
							if (registry.motions.get(registry.players.entities[0]).position.x < registry.motions.get(entity_other).position.x) {
								registry.motions.get(entity_other).position.x += 75;
							}
							if (registry.motions.get(registry.players.entities[0]).position.x > registry.motions.get(entity_other).position.x) {
								registry.motions.get(entity_other).position.x -= 75;
							}
						}
					}
				}

				// Wizard Collision
				if (registry.wizards.has(entity_other))
				{
					if (registry.wizards.get(entity_other).immunity_duration_ms <= 0.f &&
						registry.wizards.get(entity_other).getCurrState() != "dead" &&
						registry.wizards.get(entity_other).getCurrState() != "hurt")
					{
						registry.wizards.get(entity_other).immunity_duration_ms = 500.f;
						registry.wizards.get(entity_other).changeState("hurt", true);
						registry.wizards.get(entity_other).health -= registry.players.get(registry.players.entities[0]).damage;
            
						if (registry.wizards.get(entity_other).attack_duration_ms > 0) {
							registry.wizards.get(entity_other).attack_duration_ms = 0;
							registry.wizards.get(entity_other).changeState("attack1", false);
							registry.wizards.get(entity_other).changeState("attack2", false);
						}
						Mix_PlayChannel(-1, enemy_take_damage, 0);
					}
					// kill enemy
					if (registry.wizards.get(entity_other).health <= 0 && registry.wizards.get(entity_other).getCurrState() != "dead") {
						registry.wizards.get(entity_other).changeState("hurt1", false);
						registry.wizards.get(entity_other).changeState("attack2", false);
						registry.wizards.get(entity_other).changeState("attack1", false);
						registry.wizards.get(entity_other).changeState("attack2", false);
						registry.wizards.get(entity_other).changeState("dead", true);
						registry.wizards.get(entity_other).attack_duration_ms = 0;
						registry.wizards.get(entity_other).hurt_duration_ms = 0;
						honorGained += registry.wizards.get(entity_other).honor;
					}
				}

				// ATTACK COLLISION ON BAT
				if (registry.batEnemy.has(entity_other))
				{
					if (registry.batEnemy.get(entity_other).immunity_duration_ms <= 0) {
						registry.batEnemy.get(entity_other).health -= registry.players.get(registry.players.entities[0]).damage;
						Mix_PlayChannel(-1, enemy_take_damage, 0);

						// kill enemy
						if (registry.batEnemy.get(entity_other).health <= 0) {
							honorGained += registry.batEnemy.get(entity_other).honor;
							registry.remove_all_components_of(entity_other);
						}
						// enemy still alive
						else {
							registry.batEnemy.get(entity_other).immunity_duration_ms = 500.f;
						}
					}
				}
			}

			if (registry.players.has(entity)) {
				// Checking Player - health went to 0
				if (registry.players.get(entity).health <= 0) {
					// Death sound, reset timer, and fade to black
					if (!registry.deathTimers.has(entity)) {
						registry.deathTimers.emplace(entity);
						registry.deathTimers.get(entity).counter_ms = 4000;
						registry.motions.get(entity).velocity.x = 0; // Fixes issue that makes camera keep moving after player death
						registry.motions.get(entity).velocity.y = 0;
						registry.players.get(player).changeState("death", true);
						movingLeft = false;
						movingRight = false;
						Mix_PlayChannel(-1, player_death_sound, 0);
					}
				}

				// Checking Player - Fall into pit
				if (registry.deathboxes.has(entity_other)) {
					if (!registry.deathTimers.has(entity)) {
						// Death sound, reset timer, and fade to black
						registry.deathTimers.emplace(entity);
						registry.deathTimers.get(entity).counter_ms = 4000;
						registry.motions.get(entity).velocity.x = 0; // Fixes issue that makes camera keep moving after player death
						movingLeft = false;
						movingRight = false;
						Mix_PlayChannel(-1, player_death_sound, 0);
					}
				}

				// Checking if player can read sign
				if (registry.readables.has(entity_other)) {
					playerCanRead = true;
					registry.readables.get(entity_other).beingRead = true;
					while (registry.ghostEnemy.entities.size() > 0) {
						registry.remove_all_components_of(registry.ghostEnemy.entities.back());
					}
					ghost_spawn_cd = 0;
				}
				else {
					playerCanRead = false;
					for (Entity readable : registry.readables.entities) {
						registry.readables.get(readable).beingRead = false;
					}
				}

				// Checking Player - Enter next area
				if (registry.nextLevels.has(entity_other)) {
					if (!registry.nextLevelTimers.has(entity)) {
						// Next area sound, stop player, fade to black
						while (registry.ghostEnemy.entities.size() > 0) {
							registry.remove_all_components_of(registry.ghostEnemy.entities.back());
						}
						ghost_spawn_cd = 0;
						registry.nextLevelTimers.emplace(entity);
						registry.nextLevelTimers.get(entity).counter_ms = 2000;
						Mix_PlayChannel(-1, next_area_sound, 0);
					}
				}
				// Check potion collisions
				else if (registry.potions.has(entity_other)) {
					Mix_PlayChannel(-1, player_heal_sound, 0);

					auto& player = registry.players.get(entity);
					if (player.health < 3 || player.shield < 3) {
						update_health(1);
						playerHealth++;
					}

					registry.remove_all_components_of(entity_other);
				}

				//check armProjectile - player collision
				else if (registry.armProjectile.has(entity_other) && !registry.rollTimers.has(player)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0 && !registry.rollTimers.has(entity)) {
						player.immunity_duration_ms = 1000.f;
						update_health(-registry.armProjectile.get(entity_other).damage);
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}

				//check energyProjectile - player collision
				else if (registry.energyProjectile.has(entity_other) && !registry.rollTimers.has(player)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0 && !registry.rollTimers.has(entity)) {
						player.immunity_duration_ms = 1000.f;
						update_health(-registry.energyProjectile.get(entity_other).damage);
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}

				//check magicBalls1 - player collision
				else if (registry.magicBalls1.has(entity_other) && !registry.rollTimers.has(player)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0 && !registry.rollTimers.has(entity)) {
						player.immunity_duration_ms = 1000.f;
						update_health(-registry.magicBalls1.get(entity_other).damage);
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}

				//check magicBalls2 - player collision
				else if (registry.magicBalls2.has(entity_other) && !registry.rollTimers.has(player)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0 && !registry.rollTimers.has(entity)) {
						player.immunity_duration_ms = 1000.f;
						update_health(-registry.magicBalls2.get(entity_other).damage);
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}
				
				//check wolf - player collision
				else if (registry.wolfEnemy.has(entity_other) && !registry.rollTimers.has(player)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0 && !registry.rollTimers.has(entity)) {
						player.immunity_duration_ms = 1000.f;
						update_health(-registry.wolfEnemy.get(entity_other).damage);
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}

				// checking ghost - player collision
				else if (registry.ghostEnemy.has(entity_other) && !registry.rollTimers.has(player)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0 && !registry.rollTimers.has(entity)) {
						player.immunity_duration_ms = 1000.f;
						update_health(-registry.ghostEnemy.get(entity_other).damage);
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}

				else if (registry.buffs.has(entity_other)) {
					Mix_PlayChannel(-1, player_buff_sound, 0);
					if (registry.buffs.get(entity_other).buff_type == "attack") {
						registry.players.get(player).damage = 2;
						attackBuffGained = true;
					}
					else if (registry.buffs.get(entity_other).buff_type == "defense") {
						registry.players.get(player).shield = 3;
						defenseBuffGained = true;
						update_shields();
					}
					registry.remove_all_components_of(entity_other);
				}

				// checking bat - player collision
				else if (registry.batEnemy.has(entity_other) && !registry.rollTimers.has(player)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0 && !registry.rollTimers.has(entity)) {
						update_health(-registry.batEnemy.get(entity_other).damage);
						player.immunity_duration_ms = 1000.f;
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}
				// checking fireball - player collision
				else if (registry.fireBalls.has(entity_other) && !registry.rollTimers.has(player)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0 && !registry.rollTimers.has(entity)) {
						update_health(-registry.fireBalls.get(entity_other).damage);
						player.immunity_duration_ms = 1000.f;
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}
				// checking skeleton - player collision
				else if (registry.skeletonEnemy.has(entity_other) && !registry.rollTimers.has(entity)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0) {
						update_health(-registry.skeletonEnemy.get(entity_other).damage);
						player.immunity_duration_ms = 1000.f;
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}
				else if (registry.saws.has(entity_other)) {
					auto& player = registry.players.get(entity);
					if (player.immunity_duration_ms <= 0) {
						update_health(-1);
						player.immunity_duration_ms = 1000.f;
						if (player.health >= 0)
							Mix_PlayChannel(-1, player_take_damage, 0);
					}
				}
				// Checking Player - Tile collisions
				else if (registry.tiles.has(entity_other)) {
					// prevent falling through

				float player_height_half = abs(registry.motions.get(entity).scale.y) / 2.0f - 15.f;
				float player_width_half = abs(registry.motions.get(entity).scale.x) / 2.0f;

					// check left/right collision first
					// approaching from left, (p->t), requires 3 check to be identified as left side collision
					// 1. check if player right side boundary is >= tile left side boundary
					// 2. also check if player center.y is lower than tile upper boundary (could still be buggy, but less likely to happen)
					// 3. if there's still velocity to the right 

					// check bottom collision
					// 1. if tile thickness < 100, player can jump through it
					// 2. else, player upper boundary must be less than tile bottom boundary

					// check fall from up collision
					// 1. player lower boundery lower than (or equal to) tile upper boundary
					// 2. player right boundary > tile left boundary OR
					// 3. player left boundary < tile right boundary
					// 4. has downward velocity

				if (registry.motions.get(entity).position.x - player_width_half <=
					(registry.motions.get(entity_other).position.x + abs(registry.motions.get(entity_other).scale.x) / 2.0f)
					&& registry.motions.get(entity).position.y >
					(registry.motions.get(entity_other).position.y - abs(registry.motions.get(entity_other).scale.y) / 2.0f)
					&& registry.motions.get(entity).velocity.x < 0)
				{
					// side collision - object << player
					if (registry.players.get(player).getCurrState() != "roll") {
						registry.players.get(player).changeState("run", false);
					}
					registry.motions.get(player).touchingWall = true;
					if (registry.motions.get(entity_other).scale.y >= 80) {
						registry.motions.get(entity).position.x =
							registry.motions.get(entity_other).position.x + (abs(registry.motions.get(entity_other).scale.x) / 2.0f) + player_width_half + 1.0f;
					}
					registry.motions.get(entity).velocity.x = 0.0f;
				}
				else if (registry.motions.get(entity).position.x + player_width_half >=
					(registry.motions.get(entity_other).position.x - abs(registry.motions.get(entity_other).scale.x) / 2.0f)
					&& registry.motions.get(entity).position.y >
					(registry.motions.get(entity_other).position.y - abs(registry.motions.get(entity_other).scale.y) / 2.0f)
					&& registry.motions.get(entity).velocity.x > 0)
				{
					// side collision - player >> object
					if (registry.players.get(player).getCurrState() != "roll") {
						registry.players.get(player).changeState("run", false);
					}
					registry.motions.get(player).touchingWall = true;
					if (registry.motions.get(entity_other).scale.y >= 80) {
						registry.motions.get(entity).position.x =
							registry.motions.get(entity_other).position.x - (abs(registry.motions.get(entity_other).scale.x) / 2.0f) - player_width_half - 1.0f;
					}
					registry.motions.get(entity).velocity.x = 0.0f;
				}
				else if (registry.motions.get(entity).position.y + player_height_half >= (registry.motions.get(entity_other).position.y - (abs(registry.motions.get(entity_other).scale.y) / 2.0f))
					&& (registry.motions.get(entity).position.x + player_width_half > registry.motions.get(entity_other).position.x - (abs(registry.motions.get(entity_other).scale.x) / 2.0f) ||
						registry.motions.get(entity).position.x - player_width_half < registry.motions.get(entity_other).position.x + (abs(registry.motions.get(entity_other).scale.x) / 2.0f))
					&& registry.motions.get(entity).velocity.y > 0)
				{
					// land collision, e.g land on the tile
					player_tile_land = true;
					registry.motions.get(player).isJumping = false;
					registry.motions.get(player).touchingWall = false;
					registry.players.get(player).changeState("jump", false, false);
					registry.players.get(player).changeState("fall", false, false);
					registry.motions.get(entity).velocity.y = -0.0f;
					registry.motions.get(entity).position.y =
						registry.motions.get(entity_other).position.y - player_height_half - (abs(registry.motions.get(entity_other).scale.y) / 2.0f);
				}
			}
		}
		
		// Drop gravity on potions
		if (registry.potions.has(entity)) {
			float potion_height_half = abs(registry.motions.get(entity).scale.y) / 2.0f;
			float potion_width_half = abs(registry.motions.get(entity).scale.x) / 2.0f;

				if (registry.tiles.has(entity_other) && registry.motions.get(entity).velocity.y > 0) {
					registry.motions.get(entity).velocity.y = 0;
					registry.motions.get(entity).position.y =
						registry.motions.get(entity_other).position.y - potion_height_half - (abs(registry.motions.get(entity_other).scale.y) / 2.0f);
				}
				else if (!(registry.tiles.has(entity_other))) {
					registry.motions.get(entity).velocity.y += gravity;
				}
			}

		if (registry.fireBalls.has(entity)) {
				if (registry.tiles.has(entity_other)) {
				registry.remove_all_components_of(entity);
				}
			}

		}
		if (!player_tile_land)
		{
			// gravity kicks in
			if (registry.motions.get(player).velocity.y > 0.f)
			{
				registry.players.get(player).changeState("fall", true, false);
			}
			registry.motions.get(player).velocity.y += gravity;
		}

	}
	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// Move player
	if (registry.players.has(player) && registry.players.get(player).alive)
	{
		char attackDirection = registry.motions.get(player).attackDirection;
		float angle = registry.motions.get(player).angle;
		float speed = 700.0f;

		//angle not needed for speed
		float velX = speed;
		float velY = speed;

		if (key == GLFW_KEY_ENTER && !registry.rollTimers.has(player) && !playerIsReading) {
			if (action == GLFW_PRESS)
			{
				// attack
				if (player_attack_curr_cd <= 0.f)
				{
					// ready to attack
					if (rand() % 2)
					{
						createAttack1(renderer, registry.motions.get(player).position);
						registry.players.get(player).changeState("attack1", true);
					}
					else
					{
						createAttack2(renderer, registry.motions.get(player).position);
						registry.players.get(player).changeState("attack2", true);
					}
					player_attack_curr_cd = player_attack_cd;
				}
			}
		}

		if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.rollTimers.has(player) && !playerIsReading)
		{
			if (movingLeft && registry.motions.get(player).velocity.x != 0) {
				previousKeyA = true;
			}

			if (action == GLFW_PRESS) registry.players.get(player).changeState("run", true);
			//registry.motions.get(player).velocity.y = -velY;
			//right screen border check
			registry.motions.get(player).attackDirection = 'd';
			registry.motions.get(player).angle = 0;
			registry.motions.get(player).velocity.x = velX;
			movingLeft = false;
			movingRight = true;
		}

		else if (key == GLFW_KEY_D && action == GLFW_RELEASE && !playerIsReading) {
			 // Keep attack direction smooth by ensuring if other keys are held they wont be affected
			 if (attackDirection == 'd') {
				 //registry.motions.get(player).attackDirection = 'n';
			 }
			 // keep movement smooth to ensure if A is also held, velocity wont changed
			 if (registry.motions.get(player).velocity.x > 0) {
				 if (!registry.rollTimers.has(player))
				 {
					 registry.motions.get(player).velocity.x = 0.0f;
					 registry.players.get(player).changeState("run", false);
				 }
				 else
				 {
					 registry.players.get(player).changeState("run", false, false);
				 }
				 movingRight = false;
			 }

			 previousKeyD = false;

			 if (previousKeyA) {
				 key = GLFW_KEY_A;
				 action = GLFW_PRESS;
				 movingRight = false;
			 }
		}

		if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.rollTimers.has(player) && !playerIsReading)
		{
			if (movingRight && registry.motions.get(player).velocity.x != 0) {
				previousKeyD = true;
			}

			if (action == GLFW_PRESS) registry.players.get(player).changeState("run", true);
			registry.motions.get(player).attackDirection = 'a';
			registry.motions.get(player).angle = 135;
			registry.motions.get(player).velocity.x = -velX;
			movingRight = false;
			movingLeft = true;
		} 
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE && !playerIsReading) {
			if (attackDirection == 'a') {
			}
			if (registry.motions.get(player).velocity.x < 0) {
				if (!registry.rollTimers.has(player))
				{
					registry.motions.get(player).velocity.x = 0.0f;
					registry.players.get(player).changeState("run", false);
				}
				else
				{
					registry.players.get(player).changeState("run", false, false);
				}
				movingLeft = false;
			}

			previousKeyA = false;

			if (previousKeyD) {
				key = GLFW_KEY_D;
				action = GLFW_PRESS;
				movingLeft = false;
			}
		}

		if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.rollTimers.has(player) && !playerIsReading)
		{
			if (movingLeft && registry.motions.get(player).velocity.x != 0) {
				previousKeyA = true;
			}

			if (action == GLFW_PRESS) registry.players.get(player).changeState("run", true);
			registry.motions.get(player).attackDirection = 'd';
			registry.motions.get(player).angle = 0;
			registry.motions.get(player).velocity.x = velX;
			movingLeft = false;
			movingRight = true;
		}

		else if (key == GLFW_KEY_D && action == GLFW_RELEASE && !registry.rollTimers.has(player) && !playerIsReading) {
			// Keep attack direction smooth by ensuring if other keys are held they wont be affected
			if (attackDirection == 'd') {
			}
			// keep movement smooth to ensure if A is also held, velocity wont changed
			if (registry.motions.get(player).velocity.x > 0) {
				registry.motions.get(player).velocity.x = 0.0f;
				registry.players.get(player).changeState("run", false);
				movingRight = false;
			}

			previousKeyD = false;

			

			if (previousKeyA) {
				key = GLFW_KEY_A;
				action = GLFW_PRESS;
				movingLeft = false;
			}
		}

		 // isJumping set so player can't jump in midair
		 if (action == GLFW_PRESS && key == GLFW_KEY_W && registry.motions.get(player).isJumping != true && 
			 registry.motions.get(player).velocity.y >= 0 && !registry.rollTimers.has(player) && !playerIsReading) {
			 Mix_PlayChannel(-1, player_jump_sound, 0);
			 registry.motions.get(player).isJumping = true;
			 registry.players.get(player).changeState("jump", true, false);
			 registry.motions.get(player).velocity.y = -16.0f;
		 }

		 if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !registry.rollTimers.has(player) && (registry.motions.get(player).velocity.y <= 0.5 && 
			 registry.motions.get(player).velocity.y >= -0.5) && !playerIsReading) {
			 Mix_PlayChannel(-1, player_roll_sound, 0);
			 registry.rollTimers.emplace(player);
			 registry.players.get(player).changeState("roll", true);
			 if (previousKeyA) {
				 key = GLFW_KEY_A;
				 action = GLFW_PRESS;

				 //movingRight = false;
				 //BUG ======================================
				 //IF RELEASE A OR D DURING ROLLING PLAYER WILL CONTINUE IN THAT DIRECTION AFTER ROLL
			 }
			 if (previousKeyD) {
				 key = GLFW_KEY_D;
				 action = GLFW_PRESS;

				 //movingLeft = false;
				 //BUG ======================================
				 //IF RELEASE A OR D DURING ROLLING PLAYER WILL CONTINUE IN THAT DIRECTION AFTER ROLL
			 }
		 }
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_G && playerCanRead) {
		playerIsReading = !playerIsReading;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS && inCutscene) {
		currentSlide++;
		if (whichCutscene == 1) {
			firstCutscene();
		} else if (whichCutscene == 2) {
			secondCutscene();
		} else if (whichCutscene == 3) {
			thirdCutscene();
		}
	}

	// Can be done at any moment
	// Exit game
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		save_game();

		cleanup();

		// Remove all screenStates
		while (registry.screenStates.entities.size() > 0)
			registry.remove_all_components_of(registry.screenStates.entities.back());

		// Debugging for memory/component leaks
		registry.list_all_components();

		exit(0);
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		honorGained = 0;
		playerHealth = 2;
		defenseBuffGained = false;
		attackBuffGained = false;
		playerIsReading = false;
		glfwGetWindowSize(window, &w, &h);
		whichCutscene = 1;
		currentSlide = 0;

		restart_game();
	}

	// Loading game
	if (action == GLFW_RELEASE && key == GLFW_KEY_L) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		load_game_save();
	}

	// Restart level
	if (action == GLFW_RELEASE && key == GLFW_KEY_C) {
		int w, h;

		glfwGetWindowSize(window, &w, &h);
		switch_state(game_state);
		honorGained = level_start_honor;
		playerIsReading = false;
		currentSlide = 0;
	}

	// Debugging
	if (key == GLFW_KEY_T) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_F) {
		renderer->fps_bool = !renderer->fps_bool;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_H) {
		renderer->help_bool = !renderer->help_bool;
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	(vec2)mouse_position; // dummy to avoid compiler warning
}

void WorldSystem::cleanup() {

	// Remove all deathTimers
	while (registry.deathTimers.entities.size() > 0)
		registry.remove_all_components_of(registry.deathTimers.entities.back());
	printf("Cleaned Deathtimers\n");

	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());
	printf("Cleaned Motions\n");

	// Remove all collisions
	while (registry.collisions.entities.size() > 0)
		registry.remove_all_components_of(registry.collisions.entities.back());
	printf("Cleaned Collisions\n");

	// Remove all players
	while (registry.players.entities.size() > 0)
		registry.remove_all_components_of(registry.players.entities.back());
	printf("Cleaned Players\n");

	// Remove all meshPtrs
	while (registry.meshPtrs.entities.size() > 0)
		registry.remove_all_components_of(registry.meshPtrs.entities.back());
	printf("Cleaned meshPtrs\n");

	// Remove all renderRequests
	while (registry.renderRequests.entities.size() > 0)
		registry.remove_all_components_of(registry.renderRequests.entities.back());
	printf("Cleaned renderRequests\n");

	// Remove all eatables
	while (registry.eatables.entities.size() > 0)
		registry.remove_all_components_of(registry.eatables.entities.back());
	printf("Cleaned eatables\n");

	// Remove all deadlys
	while (registry.deadlys.entities.size() > 0)
		registry.remove_all_components_of(registry.deadlys.entities.back());
	printf("Cleaned deadlys\n");

	// Remove all debugComponents
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());
	printf("Cleaned debugComponents\n");

	// Remove all colors
	while (registry.colors.entities.size() > 0)
		registry.remove_all_components_of(registry.colors.entities.back());
	printf("Cleaned colors\n");

	// Remove all background
	while (registry.background.entities.size() > 0)
		registry.remove_all_components_of(registry.background.entities.back());
	printf("Cleaned background\n");

	// Remove all tiles
	while (registry.tiles.entities.size() > 0)
		registry.remove_all_components_of(registry.tiles.entities.back());
	printf("Cleaned tiles\n");

	// Remove all deathboxes
	while (registry.deathboxes.entities.size() > 0)
		registry.remove_all_components_of(registry.deathboxes.entities.back());
	printf("Cleaned deathboxes\n");
	
	// Remove all potions
	while (registry.potions.entities.size() > 0)
		registry.remove_all_components_of(registry.potions.entities.back());
	printf("Cleaned potions\n");

	// Remove all batEnemy
	while (registry.batEnemy.entities.size() > 0)
		registry.remove_all_components_of(registry.batEnemy.entities.back());
	printf("Cleaned Bat Enemy\n");

	// Remove all skelentonEnemy
	while (registry.skeletonEnemy.entities.size() > 0)
		registry.remove_all_components_of(registry.skeletonEnemy.entities.back());
	printf("Cleaned Skelenton Enemy\n");

	// Remove all demon
	while (registry.demonBoss.entities.size() > 0)
		registry.remove_all_components_of(registry.demonBoss.entities.back());
	printf("Cleaned Demon Enemy\n");

	// Remove all wolfEnemy
	while (registry.wolfEnemy.entities.size() > 0)
		registry.remove_all_components_of(registry.wolfEnemy.entities.back());
	printf("Cleaned Wolf Enemy\n");

	// Remove all ghostEnemy
	while (registry.ghostEnemy.entities.size() > 0)
		registry.remove_all_components_of(registry.ghostEnemy.entities.back());
	printf("Cleaned ghostEnemy \n");

	// Remove all golem
	while (registry.golem.entities.size() > 0)
		registry.remove_all_components_of(registry.golem.entities.back());
	printf("Cleaned golem \n");

	// Remove all arm projectiles
	while (registry.armProjectile.entities.size() > 0)
		registry.remove_all_components_of(registry.armProjectile.entities.back());
	printf("Cleaned armProjectile \n");

	// Remove all energy projectiles
	while (registry.energyProjectile.entities.size() > 0)
		registry.remove_all_components_of(registry.energyProjectile.entities.back());
	printf("Cleaned energyProjectile \n");

	// Remove all player_attack1
	while (registry.player_attack1.entities.size() > 0)
		registry.remove_all_components_of(registry.player_attack1.entities.back());
	printf("Cleaned Player Attack1\n");

	// Remove all player_attack2
	while (registry.player_attack2.entities.size() > 0)
		registry.remove_all_components_of(registry.player_attack2.entities.back());
	printf("Cleaned Player Attack2\n");

	// Remove all player_health
	while (registry.player_health.entities.size() > 0)
		registry.remove_all_components_of(registry.player_health.entities.back());
	printf("Cleaned Player Health\n");
		
	// Remove all readables
	while (registry.readables.entities.size() > 0)
		registry.remove_all_components_of(registry.readables.entities.back());
	printf("Cleaned readables\n");

	// Remove all doors
	while (registry.doors.entities.size() > 0)
		registry.remove_all_components_of(registry.doors.entities.back());
	printf("Cleaned doors\n");

	// Remove all doors
	while (registry.saws.entities.size() > 0)
		registry.remove_all_components_of(registry.saws.entities.back());
	printf("Cleaned saws\n");

	// Remove all texts
	while (registry.texts.entities.size() > 0)
		registry.remove_all_components_of(registry.texts.entities.back());
	printf("Cleaned Texts\n");

	// Remove all roll timers
	while (registry.rollTimers.entities.size() > 0)
		registry.remove_all_components_of(registry.rollTimers.entities.back());
	printf("Cleaned roll timers\n");

	// Remove all ranged enemies
	while (registry.rangedEnemy.entities.size() > 0)
		registry.remove_all_components_of(registry.rangedEnemy.entities.back());
	printf("Cleaned ranged enemy\n");

	// Remove all fireballs
	while (registry.fireBalls.entities.size() > 0)
		registry.remove_all_components_of(registry.fireBalls.entities.back());
	printf("Cleaned fireballs\n");

	// Remove all fireballs
	while (registry.shields.entities.size() > 0)
		registry.remove_all_components_of(registry.shields.entities.back());
	printf("Cleaned shields\n");

	// Remove all attackPathTimers
	while (registry.attackPathTimers.entities.size() > 0)
		registry.remove_all_components_of(registry.attackPathTimers.entities.back());
	printf("Cleaned fireballs\n");

	while (registry.buffs.entities.size() > 0)
		registry.remove_all_components_of(registry.buffs.entities.back());
	printf("Cleaned buffs\n");
	

	// Remove all Wizards
	while (registry.wizards.entities.size() > 0)
		registry.remove_all_components_of(registry.wizards.entities.back());
	printf("Cleaned wizards\n");

	// Remove all magicBalls1
	while (registry.magicBalls1.entities.size() > 0)
		registry.remove_all_components_of(registry.magicBalls1.entities.back());
	printf("Cleaned magicBalls1\n");

	// Remove all magicBalls1
	while (registry.magicBalls2.entities.size() > 0)
		registry.remove_all_components_of(registry.magicBalls2.entities.back());
	printf("Cleaned magicBalls1\n");
	
	// Debugging for memory/component leaks
	registry.list_all_components();
}
