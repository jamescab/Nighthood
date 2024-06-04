#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>
#include <map>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};

// window size
const int window_width_px = 1280;
const int window_height_px = 1000;

// how far would camera go beyond the player's pos relative to player's velocity
const float x_camera_multiplier = 0.08f;
const float y_camera_multiplier = 3.f;

const float gravity = 0.50f;

extern bool playerCanRead;
extern bool playerIsReading;
extern int honorGained;
extern int bossHealth;
extern bool defenseBuffGained;
extern bool attackBuffGained;
extern bool displayHonor;
extern bool showBossHealth;
extern bool gameStarted;
extern bool inCutscene;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
	void mirrorYAxis(float radians);
	void rotateProjectile(float radians, Entity entity);
};

bool gl_has_errors();

enum GameState {
	Intro,
	FirstCutscene,
	Tutorial,
	LevelOne,
	SecondCutscene,
	LevelTwo,
	ThirdCutscene,
	BuffRoom,
	LevelThree,
	Complete,
	GameOver,
	CutScene
};
