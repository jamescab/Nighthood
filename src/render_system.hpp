#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <map>

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  // specify meshes of other assets here
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER_ATTACK1, mesh_path("attack1.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER_ATTACK2, mesh_path("attack2.obj"))
	};

	// Make sure these paths remain in sync with the associated enumerators.
	// TODO replace textures here
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("bat_sheet.png"),
			textures_path("background.png"),
			textures_path("cloud.png"),
			textures_path("trees.png"),
			textures_path("stone.png"),
			textures_path("fireball.png"),
			textures_path("player_sheet.png"),
			textures_path("red_potion_32.png"),
			textures_path("heart_sheet.png"),
			textures_path("shield_sheet.png"),
			textures_path("saw.png"),
			textures_path("magicball1.png"),
			textures_path("magicball2.png"),
			textures_path("SkeletonIdle.png"),
			textures_path("mushroom_sheet.png"),
			textures_path("wizard_sheet.png"),
			textures_path("demon.png"),
			textures_path("level_door.png"),
			textures_path("readable_statue.png"),
			textures_path("attack_buff.png"),
			textures_path("defense_buff.png"),
			textures_path("pedestal.png"),
			textures_path("stone_verticle.png"),
			textures_path("stone_verticle_long.png"),
			textures_path("stone_verticle_long_thick.png"),
			textures_path("Intro.png"),
			textures_path("Complete.png"),
			textures_path("GameOver.png"),
			textures_path("ghost.png"),
			textures_path("WolfEnemy.png"),
			textures_path("Cutscene1a.png"),
			textures_path("Cutscene1b.png"),
			textures_path("Cutscene1c.png"),
			textures_path("Cutscene1d.png"),
			textures_path("Cutscene1e.png"),
			textures_path("Cutscene2a.png"),
			textures_path("Cutscene2b.png"),
			textures_path("Cutscene2c.png"),
			textures_path("Cutscene3a.png"),
			textures_path("Cutscene3b.png"),
			textures_path("Cutscene3c.png"),
			textures_path("Cutscene3d.png"),
			textures_path("background2.png"),
			textures_path("background3.png"),
			textures_path("golem.png"),
			textures_path("arm_projectile.png"),
			textures_path("energyProjectile.png")
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("player"),
		shader_path("textured"),
		shader_path("texturedfixed"),
		shader_path("wind")};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

	// Map of character fonts
	std::map<char, Character> m_ftCharacters;

	// Font handles
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();
	
	bool initFont(const std::string& font_filename, unsigned int font_default_size);

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	// Draw text
	void RenderSystem::drawText(std::string text, vec2 pos, vec2 scale, const vec3& color, const mat4& trans);

	mat3 createProjectionMatrix();

	void setFPS(int fps);
	bool fps_bool;
	bool help_bool;

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();
	void resetVAO();

	// Window handle
	GLFWwindow* window;

	// Dummy VAO
	GLuint m_vao;

	// FPS
	int m_fps;


	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
