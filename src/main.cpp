
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;

	// Initializing window
	GLFWwindow* window = world.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	renderer.init(window);
	world.init(&renderer);

	// World steps
	const float step_value = 1000.f / 60.f;
	float ms_count = 0.0f;

	// frames per second
	int frame_count = 0;
	float fps_count = 0.0f;

	// variable timestep loop
	auto t = Clock::now();
	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// Update world and physics 60 times per second
		ms_count += elapsed_ms;
		if (ms_count >= step_value) {
			world.step(ms_count);
			physics.step(ms_count);
			world.handle_collisions();
			ms_count = 0;
		}

		// Calculate FPS
		frame_count++;
		fps_count += elapsed_ms;
		if (fps_count >= 1000.0f) {
			renderer.setFPS(frame_count);
			frame_count = 0;
			fps_count = 0;
		}

		renderer.draw();
	}

	return EXIT_SUCCESS;
}
