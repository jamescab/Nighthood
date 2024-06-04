// internal
#include "render_system.hpp"
#include <SDL.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "tiny_ecs_registry.hpp"

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	
	transform.translate(motion.position);
	transform.scale(motion.scale);
	// Rotation to the chain of transformations, mind the order
	// of transformations
	//transform.rotate(registry.motions.get(entity).angle);
	// Mirror across Y axis for moving left and right
	transform.mirrorYAxis(registry.motions.get(entity).angle);
	transform.rotateProjectile(motion.angle, entity);

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);
		
		glEnableVertexAttribArray(in_position_loc);
		gl_has_errors();
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void *)0);

		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::TEXTUREDFIXED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		gl_has_errors();
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void *)0);

		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
		if (render_request.used_texture == TEXTURE_ASSET_ID::HEART_SHEET &&
			render_request.used_effect == EFFECT_ASSET_ID::TEXTUREDFIXED &&
			render_request.used_geometry == GEOMETRY_BUFFER_ID::PLAYER_HEART) {
			registry.player_health.get(entity).stateChange = false;
			std::string curr_state = registry.player_health.get(entity).getCurrState();
			glm::uint& sIndex = registry.player_health.get(entity).state_index;
			glm::uint col = registry.player_health.get(entity).state_map.at(curr_state).first;
			glm::uint& RowMax = registry.player_health.get(entity).rowMax;
			glm::uint& ColMax = registry.player_health.get(entity).colMax;
			float& width = registry.player_health.get(entity).sheetsize.x;
			float& height = registry.player_health.get(entity).sheetsize.y;

			std::vector<TexturedVertex> textured_vertices(4);
			// if the texture is off, change it here
			textured_vertices[0].position = { -1.2f, 0.4f, 0.f };
			textured_vertices[1].position = { +1.2f, 0.4f, 0.f };
			textured_vertices[2].position = { +1.2f, -1.6f , 0.f };
			textured_vertices[3].position = { -1.2f, -1.6f , 0.f };
			// if the specific texture not loaded correctly, change it here
			textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * (col + 1)) / height };
			textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * (col + 1)) / height };
			textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * col) / height };
			textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * col) / height };
			const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
			bindVBOandIBO(GEOMETRY_BUFFER_ID::PLAYER_HEART, textured_vertices, textured_indices);
		}

		if (render_request.used_texture == TEXTURE_ASSET_ID::SHIELD_SHEET &&
			render_request.used_effect == EFFECT_ASSET_ID::TEXTUREDFIXED &&
			render_request.used_geometry == GEOMETRY_BUFFER_ID::PLAYER_SHIELD) {
			registry.shields.get(entity).stateChange = false;
			std::string curr_state = registry.shields.get(entity).getCurrState();
			glm::uint& sIndex = registry.shields.get(entity).state_index;
			glm::uint col = registry.shields.get(entity).state_map.at(curr_state).first;
			glm::uint& RowMax = registry.shields.get(entity).rowMax;
			glm::uint& ColMax = registry.shields.get(entity).colMax;
			float& width = registry.shields.get(entity).sheetsize.x;
			float& height = registry.shields.get(entity).sheetsize.y;

			std::vector<TexturedVertex> textured_vertices(4);
			// if the texture is off, change it here
			textured_vertices[0].position = { -1.2f, 0.4f, 0.f };
			textured_vertices[1].position = { +1.2f, 0.4f, 0.f };
			textured_vertices[2].position = { +1.2f, -1.6f , 0.f };
			textured_vertices[3].position = { -1.2f, -1.6f , 0.f };
			// if the specific texture not loaded correctly, change it here
			textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * (col + 1)) / height };
			textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * (col + 1)) / height };
			textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * col) / height };
			textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * col) / height };
			const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
			bindVBOandIBO(GEOMETRY_BUFFER_ID::PLAYER_SHIELD, textured_vertices, textured_indices);
		}
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::PLAYER || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();

		if (render_request.used_effect == EFFECT_ASSET_ID::PLAYER)
		{
			// Status glow
			GLint status_glow_uloc = glGetUniformLocation(program, "status_glow");
			assert(status_glow_uloc >= 0);

			// Set the light_up shader variable using glUniform1i,
			// similar to the glUniform1f call below. The 1f or 1i specified the type, here a single int.
			glUniform1i(status_glow_uloc, 0);
			gl_has_errors();
		}
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);

	if (render_request.used_texture == TEXTURE_ASSET_ID::SKELETON_IDLE &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::SKELETON_ENEMY)
	{
		if (registry.skeletonEnemy.has(entity))
		{
			if (registry.skeletonEnemy.get(entity).stateChange)
			{
				registry.skeletonEnemy.get(entity).stateChange = false;
				std::string curr_state = registry.skeletonEnemy.get(entity).getCurrState();
				glm::uint& sIndex = registry.skeletonEnemy.get(entity).state_index;
				glm::uint col = registry.skeletonEnemy.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.skeletonEnemy.get(entity).rowMax;
				glm::uint& ColMax = registry.skeletonEnemy.get(entity).colMax;
				float& width = registry.skeletonEnemy.get(entity).sheetsize.x;
				float& height = registry.skeletonEnemy.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -1.5f, 1.4f, 0.f };
				textured_vertices[1].position = { +1.5f, 1.4f, 0.f };
				textured_vertices[2].position = { +1.5f, -1.4f , 0.f };
				textured_vertices[3].position = { -1.5f, -1.4f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::SKELETON_ENEMY, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::BAT &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::BAT_ENEMY)
	{
		if (registry.batEnemy.has(entity))
		{
			if (registry.batEnemy.get(entity).stateChange)
			{
				registry.batEnemy.get(entity).stateChange = false;
				std::string curr_state = registry.batEnemy.get(entity).getCurrState();
				glm::uint& sIndex = registry.batEnemy.get(entity).state_index;
				glm::uint col = registry.batEnemy.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.batEnemy.get(entity).rowMax;
				glm::uint& ColMax = registry.batEnemy.get(entity).colMax;
				float& width = registry.batEnemy.get(entity).sheetsize.x;
				float& height = registry.batEnemy.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -1.5f, 1.4f, 0.f };
				textured_vertices[1].position = { +1.5f, 1.4f, 0.f };
				textured_vertices[2].position = { +1.5f, -1.4f , 0.f };
				textured_vertices[3].position = { -1.5f, -1.4f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::BAT_ENEMY, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::MUSHROOM &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::MUSHROOM_ENEMY)
	{
		if (registry.rangedEnemy.has(entity))
		{
			if (registry.rangedEnemy.get(entity).stateChange)
			{
				registry.rangedEnemy.get(entity).stateChange = false;
				std::string curr_state = registry.rangedEnemy.get(entity).getCurrState();
				glm::uint& sIndex = registry.rangedEnemy.get(entity).state_index;
				glm::uint col = registry.rangedEnemy.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.rangedEnemy.get(entity).rowMax;
				glm::uint& ColMax = registry.rangedEnemy.get(entity).colMax;
				float& width = registry.rangedEnemy.get(entity).sheetsize.x;
				float& height = registry.rangedEnemy.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -2.0f, 1.8f, 0.f };
				textured_vertices[1].position = { +2.0f, 1.8f, 0.f };
				textured_vertices[2].position = { +2.0f, -2.2f , 0.f };
				textured_vertices[3].position = { -2.0f, -2.2f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::MUSHROOM_ENEMY, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::WIZARD &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::WIZARD_ENEMY)
	{
		if (registry.wizards.has(entity))
		{
			if (registry.wizards.get(entity).stateChange)
			{
				registry.wizards.get(entity).stateChange = false;
				std::string curr_state = registry.wizards.get(entity).getCurrState();
				glm::uint& sIndex = registry.wizards.get(entity).state_index;
				glm::uint col = registry.wizards.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.wizards.get(entity).rowMax;
				glm::uint& ColMax = registry.wizards.get(entity).colMax;
				float& width = registry.wizards.get(entity).sheetsize.x;
				float& height = registry.wizards.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -2.0f, 0.5f, 0.f };
				textured_vertices[1].position = { +2.4f, 0.5f, 0.f };
				textured_vertices[2].position = { +2.4f, -1.5f , 0.f };
				textured_vertices[3].position = { -2.0f, -1.5f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::WIZARD_ENEMY, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::MAGICBALL1 &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::MAGICBALL1)
	{
		if (registry.magicBalls1.has(entity))
		{
			if (registry.magicBalls1.get(entity).stateChange)
			{
				registry.magicBalls1.get(entity).stateChange = false;
				std::string curr_state = registry.magicBalls1.get(entity).getCurrState();
				glm::uint& sIndex = registry.magicBalls1.get(entity).state_index;
				glm::uint col = registry.magicBalls1.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.magicBalls1.get(entity).rowMax;
				glm::uint& ColMax = registry.magicBalls1.get(entity).colMax;
				float& width = registry.magicBalls1.get(entity).sheetsize.x;
				float& height = registry.magicBalls1.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -4.1f, 3.9f, 0.f };
				textured_vertices[1].position = { +1.3f, 3.9f, 0.f };
				textured_vertices[2].position = { +1.3f, -3.9f , 0.f };
				textured_vertices[3].position = { -4.1f, -3.9f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::MAGICBALL1, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::MAGICBALL2 &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::MAGICBALL2)
	{
		if (registry.magicBalls2.has(entity))
		{
			if (registry.magicBalls2.get(entity).stateChange)
			{

				registry.magicBalls2.get(entity).stateChange = false;
				std::string curr_state = registry.magicBalls2.get(entity).getCurrState();
				glm::uint& sIndex = registry.magicBalls2.get(entity).state_index;
				glm::uint col = registry.magicBalls2.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.magicBalls2.get(entity).rowMax;
				glm::uint& ColMax = registry.magicBalls2.get(entity).colMax;
				float& width = registry.magicBalls2.get(entity).sheetsize.x;
				float& height = registry.magicBalls2.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -5.7f, 3.9f, 0.f };
				textured_vertices[1].position = { +3.0f, 3.9f, 0.f };
				textured_vertices[2].position = { +3.0f, -3.9f , 0.f };
				textured_vertices[3].position = { -5.7f, -3.9f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::MAGICBALL2, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::SAW &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::SAW)
	{
		if (registry.saws.has(entity))
		{
			if (registry.saws.get(entity).stateChange)
			{
				registry.saws.get(entity).stateChange = false;
				std::string curr_state = registry.saws.get(entity).getCurrState();
				glm::uint& sIndex = registry.saws.get(entity).state_index;
				glm::uint col = registry.saws.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.saws.get(entity).rowMax;
				glm::uint& ColMax = registry.saws.get(entity).colMax;
				float& width = registry.saws.get(entity).sheetsize.x;
				float& height = registry.saws.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -1.0f, 1.5f, 0.f };
				textured_vertices[1].position = { +1.0f, 1.5f, 0.f };
				textured_vertices[2].position = { +1.0f, -0.5f , 0.f };
				textured_vertices[3].position = { -1.0f, -0.5f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::SAW, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::WOLF &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::WOLF_ENEMY)
	{
		if (registry.wolfEnemy.has(entity))
		{
			if (registry.wolfEnemy.get(entity).stateChange)
			{
				registry.wolfEnemy.get(entity).stateChange = false;
				std::string curr_state = registry.wolfEnemy.get(entity).getCurrState();
				glm::uint& sIndex = registry.wolfEnemy.get(entity).state_index;
				glm::uint col = registry.wolfEnemy.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.wolfEnemy.get(entity).rowMax;
				glm::uint& ColMax = registry.wolfEnemy.get(entity).colMax;
				float& width = registry.wolfEnemy.get(entity).sheetsize.x;
				float& height = registry.wolfEnemy.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -1.0f, 0.5f, 0.f };
				textured_vertices[1].position = { +1.0f, 0.5f, 0.f };
				textured_vertices[2].position = { +1.0f, -0.5f , 0.f };
				textured_vertices[3].position = { -1.0f, -0.5f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::WOLF_ENEMY, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::GOLEM &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::GOLEM_ENEMY)
	{
		if (registry.golem.has(entity))
		{
			if (registry.golem.get(entity).stateChange)
			{
				registry.golem.get(entity).stateChange = false;
				std::string curr_state = registry.golem.get(entity).getCurrState();
				glm::uint& sIndex = registry.golem.get(entity).state_index;
				glm::uint col = registry.golem.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.golem.get(entity).rowMax;
				glm::uint& ColMax = registry.golem.get(entity).colMax;
				float& width = registry.golem.get(entity).sheetsize.x;
				float& height = registry.golem.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -1.1f, 1.2f, 0.f };
				textured_vertices[1].position = { +1.1f, 1.2f, 0.f };
				textured_vertices[2].position = { +1.1f, -0.8f , 0.f };
				textured_vertices[3].position = { -1.1f, -0.8f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::GOLEM_ENEMY, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::ARMPROJECTILE &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::ARMPROJECTILE_ENEMY)
	{
		if (registry.armProjectile.has(entity))
		{
			if (registry.armProjectile.get(entity).stateChange)
			{
				registry.armProjectile.get(entity).stateChange = false;
				std::string curr_state = registry.armProjectile.get(entity).getCurrState();
				glm::uint& sIndex = registry.armProjectile.get(entity).state_index;
				glm::uint col = registry.armProjectile.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.armProjectile.get(entity).rowMax;
				glm::uint& ColMax = registry.armProjectile.get(entity).colMax;
				float& width = registry.armProjectile.get(entity).sheetsize.x;
				float& height = registry.armProjectile.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -0.5f, 1.5f, 0.f };
				textured_vertices[1].position = { +0.5f, 1.5f, 0.f };
				textured_vertices[2].position = { +0.5f, -1.5f , 0.f };
				textured_vertices[3].position = { -0.5f, -1.5f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::ARMPROJECTILE_ENEMY, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::ENERGYPROJECTILE &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::ENERGYPROJECTILE_ENEMY)
	{
		if (registry.energyProjectile.has(entity))
		{
			if (registry.energyProjectile.get(entity).stateChange)
			{
				registry.energyProjectile.get(entity).stateChange = false;
				std::string curr_state = registry.energyProjectile.get(entity).getCurrState();
				glm::uint& sIndex = registry.energyProjectile.get(entity).state_index;
				glm::uint col = registry.energyProjectile.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.energyProjectile.get(entity).rowMax;
				glm::uint& ColMax = registry.energyProjectile.get(entity).colMax;
				float& width = registry.energyProjectile.get(entity).sheetsize.x;
				float& height = registry.energyProjectile.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -1.1f, 0.9f, 0.f };
				textured_vertices[1].position = { +0.9f, 0.9f, 0.f };
				textured_vertices[2].position = { +0.9f, -0.9f , 0.f };
				textured_vertices[3].position = { -1.1f, -0.9f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax) * col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax) * col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::ENERGYPROJECTILE_ENEMY, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::DEMON &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::DEMON_ENEMY)
	{
		if (registry.demonBoss.has(entity))
		{
			if (registry.demonBoss.get(entity).stateChange)
			{
				registry.demonBoss.get(entity).stateChange = false;
				std::string curr_state = registry.demonBoss.get(entity).getCurrState();
				glm::uint& sIndex = registry.demonBoss.get(entity).state_index;
				glm::uint col = registry.demonBoss.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.demonBoss.get(entity).rowMax;
				glm::uint& ColMax = registry.demonBoss.get(entity).colMax;
				float& width = registry.demonBoss.get(entity).sheetsize.x;
				float& height = registry.demonBoss.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -2.1f, 0.5f, 0.f };
				textured_vertices[1].position = { +2.1f, 0.5f, 0.f };
				textured_vertices[2].position = { +2.1f, -1.5f , 0.f };
				textured_vertices[3].position = { -2.1f, -1.5f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::DEMON_ENEMY, textured_vertices, textured_indices);
			}
		}
	}

	if (render_request.used_texture == TEXTURE_ASSET_ID::PLAYER_SHEET &&
		render_request.used_effect == EFFECT_ASSET_ID::TEXTURED &&
		render_request.used_geometry == GEOMETRY_BUFFER_ID::PLAYER)
	{
		if (registry.players.has(entity))
		{
			if (registry.players.get(entity).stateChange)
			{
				registry.players.get(entity).stateChange = false;
				std::string curr_state = registry.players.get(entity).getCurrState();
				glm::uint& sIndex = registry.players.get(entity).state_index;
				glm::uint col = registry.players.get(entity).state_map.at(curr_state).first;
				glm::uint& RowMax = registry.players.get(entity).rowMax;
				glm::uint& ColMax = registry.players.get(entity).colMax;
				float& width = registry.players.get(entity).sheetsize.x;
				float& height = registry.players.get(entity).sheetsize.y;

				std::vector<TexturedVertex> textured_vertices(4);
				// if the texture is off, change it here
				textured_vertices[0].position = { -2.0f, 0.4f, 0.f };
				textured_vertices[1].position = { +2.3f, 0.4f, 0.f };
				textured_vertices[2].position = { +2.3f, -1.6f , 0.f };
				textured_vertices[3].position = { -2.0f, -1.6f , 0.f };
				// if the specific texture not loaded correctly, change it here
				textured_vertices[0].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[1].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* (col + 1)) / height };
				textured_vertices[2].texcoord = { ((width / RowMax) * sIndex) / width, ((height / ColMax)* col) / height };
				textured_vertices[3].texcoord = { ((width / RowMax) * (sIndex - 1)) / width, ((height / ColMax)* col) / height };
				const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
				bindVBOandIBO(GEOMETRY_BUFFER_ID::PLAYER, textured_vertices, textured_indices);
			}
		}

		// camera
		float x = registry.players.get(registry.players.entities[0]).camera_x;
		float y = registry.players.get(registry.players.entities[0]).camera_y;
		glm::vec3 playerPos = glm::vec3(
			(2.0f * x - window_width_px) / window_width_px, ((window_height_px - y - 400.0f) * 2.0f) / window_height_px,
			0.0f);

		glm::mat4 viewMatrix = glm::lookAt(
			playerPos + glm::vec3(0.0f, 0.0f, 1.0f),
			playerPos,
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		GLuint viewMatrixLocation = glGetUniformLocation(currProgram, "viewMatrix");
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, (float *)&viewMatrix);
	}

	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawText(std::string text, vec2 pos, vec2 scale, const glm::vec3& color, const glm::mat4& trans)
{
	// activate corresponding render state
	glUseProgram(m_font_shaderProgram);
	GLint textColor_location = glGetUniformLocation(m_font_shaderProgram, "textColor");
	assert(textColor_location > -1);

	glUniform3f(textColor_location, color.x, color.y, color.z);
	auto transformLoc = glGetUniformLocation(m_font_shaderProgram, "transform");
	assert(transformLoc > -1);

	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

	glBindVertexArray(m_font_VAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = m_ftCharacters[*c];
		float xpos = pos.x + ch.Bearing.x * scale.x;
		float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * scale.y;
		float w = ch.Size.x * scale.x;
		float h = ch.Size.y * scale.y;

		// update VBO for each character
		float vertices[6][4] = {
		{ xpos, ypos + h, 0.0f, 0.0f },
		{ xpos, ypos, 0.0f, 1.0f },
		{ xpos + w, ypos, 1.0f, 1.0f },
		{ xpos, ypos + h, 0.0f, 0.0f },
		{ xpos + w, ypos, 1.0f, 1.0f },
		{ xpos + w, ypos + h, 1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // ERROR HERE INVALID VALUE

		gl_has_errors();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		pos.x += (ch.Advance >> 6) * scale.x; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	// Unbind text VAO
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	resetVAO();
}

// draw the intermediate texture to the screen, with some distortion to simulate
// wind
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the wind texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WIND]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint wind_program = effects[(GLuint)EFFECT_ASSET_ID::WIND];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(wind_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(wind_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(wind_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

void RenderSystem::resetVAO() {
	// Rebind Dummy VAO
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	
	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);

	glClearColor(0.674, 0.847, 0.0 , 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
	// Draw all textured meshes that have a position and size component
	for (Entity entity : registry.renderRequests.entities)
	{
		if (!registry.motions.has(entity))
			continue;
		// Note, its not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize

		// do not render attack obj
		if (registry.player_attack1.has(entity) || registry.player_attack2.has(entity))
			continue;

		drawTexturedMesh(entity, projection_2D);
	}

	mat4 trans = mat4(1.0f);

	mat4 honor_trans = mat4(1.0f);
	honor_trans = glm::scale(honor_trans, vec3(0.5, 0.5, 1));
	if (gameStarted && !inCutscene && displayHonor) {
		if (honorGained > 20) {
			drawText("Honor level: " + std::to_string(honorGained), { 5.f, window_height_px * 2 - 45 }, { 1.25f, 1.25f }, glm::vec3(1.0f, 0.0f, 0.0f), honor_trans);
		}
		else if (honorGained > 10) {
			drawText("Honor level: " + std::to_string(honorGained), { 5.f, window_height_px * 2 - 45 }, { 1.25f, 1.25f }, glm::vec3(0.0f, 0.0f, 1.0f), honor_trans);
		}
		else {
			drawText("Honor level: " + std::to_string(honorGained), { 5.f, window_height_px * 2 - 45 }, { 1.25f, 1.25f }, glm::vec3(1.0f, 1.0f, 1.0f), honor_trans);
		}
	}

	// Render FPS text if toggled
	if (fps_bool) {
		mat4 fps_trans = mat4(1.0f);
		fps_trans = glm::scale(fps_trans, vec3(0.5, 0.5, 1));
		drawText("FPS: " + std::to_string(m_fps), { 5.f, window_height_px * 2 - 90 }, { 1.25f, 1.25f }, glm::vec3(1.0f, 1.0f, 1.0f), fps_trans);
	}

	// Tutorial text
	if (!inCutscene) {
		if (help_bool) {
			trans = glm::scale(trans, vec3(0.5, 0.5, 1));
			drawText("'A' key to move left", { 5.f, window_height_px - 740 }, { 1.0f, 1.0f }, glm::vec3(0.5f, 0.5f, 0.5f), trans);
			drawText("'D' key to move right", { 5.f, window_height_px - 780 }, { 1.0f, 1.0f }, glm::vec3(0.5f, 0.5f, 0.5f), trans);
			drawText("'W' key to jump", { 5.f, window_height_px - 820 }, { 1.0f, 1.0f }, glm::vec3(0.5f, 0.5f, 0.5f), trans);
			drawText("'SPACE' key to roll", { 5.f, window_height_px - 860 }, { 1.0f, 1.0f }, glm::vec3(0.5f, 0.5f, 0.5f), trans);
			drawText("'ENTER' key to attack", { 5.f, window_height_px - 900 }, { 1.0f, 1.0f }, glm::vec3(0.5f, 0.5f, 0.5f), trans);
		}
		else {
			trans = glm::scale(trans, vec3(0.5, 0.5, 1));
			drawText("'H' key to toggle help", { 5.f, window_height_px - 900 }, { 1.0f, 1.0f }, glm::vec3(0.5f, 0.5f, 0.5f), trans);
		}
	}

	// Read sign prompt
	// If prompted, prints out sign's message to screen
	if (playerCanRead) {
		trans = glm::scale(trans, vec3(0.5, 0.5, 1));
		if (playerIsReading) {
			for (Entity readable : registry.readables.entities) {
				if (registry.readables.get(readable).beingRead) {
					for (int i = 0; i < registry.readables.get(readable).lines.size(); i++) {
						drawText(registry.readables.get(readable).lines[i], { 2550 - (24 * registry.readables.get(readable).lines[i].length()), 3000 - (120 * i)}, {3.0f, 3.0f}, glm::vec3(0.5f, 0.5f, 0.5f), trans);

						// Last line of every message
						if (i == registry.readables.get(readable).lines.size() - 1) {
							drawText("Press 'G' to stop reading", { 2550 - (24 * 25), 3000 - (120 * i) - 330 }, { 3.0f, 3.0f }, glm::vec3(0.5f, 0.5f, 0.5f), trans);
						}
					}
				}
			}
		}
		else {
			drawText("Press 'G' to read", { 2550 - (24 * 17), 2600}, {3.0f, 3.0f}, glm::vec3(0.5f, 0.5f, 0.5f), trans);
		}
	}

	if (showBossHealth) {
		std::string healthBar = "";
		for (int i = 0; i < bossHealth; i++) {
			healthBar += "[]";
		}
		drawText("Sentinel Golem", { 1170, 260 }, { 1.5f, 1.5f }, glm::vec3(1.0f, 0.851f, 0.4f), trans);
		drawText(healthBar, { 800, 200 }, { 3.0f, 1.0f }, glm::vec3(1.0f, 0.267f, 0.267f), trans);
	}
	
	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float) window_width_px;
	float bottom = (float) window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

void RenderSystem::setFPS(int fps) {
	m_fps = fps;
}
