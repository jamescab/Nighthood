#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;

	// draw debug bounding box
	void drawDebugBoundingBox(const vec2& pos, const vec2& scale, vec3 color, float lineThickns);

	// draw dotted outline for mesh
	void drawDebugDot(Entity& e, vec3 color, float lineThickns);

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);

	// restart level
	void restart_game();

	// update player health bar
	void update_hearts();
	void update_shields();
	void update_health(int dmg);
	// OpenGL window handle
	GLFWwindow* window;

	// Leftover from A1
	unsigned int points;

	// Game state
	void load_game_save();
	void save_game();
	GameState game_state;
	RenderSystem* renderer;
	Entity player;
	Entity background;
	std::vector<Entity> background_cloud;
	std::vector<Entity> background_tree;
	Entity background_trees;
	std::vector<Entity> tiles;

	void intro();
	void firstCutscene();
	void tutorial();
	void level_one();
	void secondCutscene();
	void level_two();
	void thirdCutscene();
	void buff_room();
	void level_three();
	void game_over();
	void game_complete();

	void switch_state(GameState gs);
	void cleanup();

	// Render, player update frame
	const float player_update_frame = 50.f;
	float player_curr_frame = player_update_frame;
	const float skeleton_update_frame = 100.f;
	const float bat_update_frame = 100.f;
	const float wolf_update_frame = 100.f;
	const float wizard_update_frame = 100.f;
	const float player_attack_cd = 300.f;
	float player_attack_curr_cd = 0.f;

	// control how fast camera data is updated
	const float camera_update_frame = 15.f;
	float camera_curr_frame = camera_update_frame;

	// Parallex background value
	// larger the factor, slower the movement
	const float cloud_width = 576.f, cloud_height = 324.f;
	const float cloud_relative_speed_factor = 4.f; // relative to player's velocity
	const float tree_width = 800.f, tree_height = 600.f;
	const float tree_relative_speed_factor = 2.5f; // relative to player's velocity
	float player_prev_position = -1000.f;
	float player_position_derivative = 0.f; // real velocity

	float ghost_spawn_cd = 0;
	int ghost_spawn_limit = 0;
	float set_ghost_spawn_cd = 0;

	// music references
	Mix_Chunk* player_death_sound;
	Mix_Chunk* player_heal_sound;
	Mix_Chunk* player_buff_sound;
	Mix_Chunk* player_jump_sound;
	Mix_Chunk* player_roll_sound;
	Mix_Chunk* player_take_damage;
	Mix_Chunk* enemy_take_damage;
	Mix_Chunk* potion_disappear_sound;
	Mix_Chunk* next_area_sound;
	Mix_Chunk* boss_death_sound;
	Mix_Chunk* background_music;
	Mix_Chunk* boss_music;
	Mix_Chunk* intro_music;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
