
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "EntityManager.h"
#include "EntityMesh.h"
#include "GameStageManager.h"
#include "PlayStage.h"
#include "EditorStage.h"
#include "TileMap.h"
#include "ZombieManager.h"
#include <bass.h>
#include "Audio.h"
#include "StaticObjectRenderer.h"
#include "MainMenuStage.h"
#include "PauseStage.h"
#include "DeadStage.h"
#include "SelectLevelStage.h"
#include "WinStage.h"
#include "TutorialStage.h"

Game* Game::instance = nullptr;
GameStageManager* GameStageManager::instance = nullptr;
ZombieManager* ZombieManager::instance = nullptr;
StaticObjectRenderer* StaticObjectRenderer::instance = nullptr;

Mesh Game::woodQuad;


TileMap* Game::GetMapAt(const Vector3& pos)const
{
	for (auto* map : Maps)
	{
		const auto mapPosition = map->GetMapEntity()->GetWorldPosition();
		const auto mapWidth = map->GetMapWidth();
		const auto mapHeight = map->GetMapHeight();

		if (pos.x >= mapPosition.x && pos.x <= mapPosition.x + mapWidth
			&& pos.z >= mapPosition.z && pos.z <= mapPosition.z + mapHeight)
		{
			return map;
		}
	}
	return nullptr;
}



Game::Game(int window_width, int window_height, SDL_Window* window)
	:Maps({ nullptr })
	,CurrentLevel(0)
	,CurrentDayTime(0.f)
	,CurrentFrame(0)
	,ShowRays(false)
	,WoodCounter(0)
	,Score(0)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	time_pause = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;
	pause = true;
	pause_clean = false;

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	EntityManager::instance = new EntityManager();
	ZombieManager::instance = new ZombieManager();
	GameStageManager::instance = new GameStageManager();
	StaticObjectRenderer::instance = new StaticObjectRenderer();
	StaticObjectRenderer::instance->SetObjectShader(Shader::Get("data/shaders/basic_instanced.vs", "data/shaders/flat.fs"));

	GameStageManager::instance->RegisterStage(new MainMenuStage());
	GameStageManager::instance->RegisterStage(new PlayStage());
	GameStageManager::instance->RegisterStage(new EditorStage());
	GameStageManager::instance->RegisterStage(new PauseStage());
	GameStageManager::instance->RegisterStage(new DeadStage());
	GameStageManager::instance->RegisterStage(new SelectLevelStage());
	GameStageManager::instance->RegisterStage(new WinStage());
	GameStageManager::instance->RegisterStage(new TutorialStage());

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f, 10.f, 10.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f, window_width / (float)window_height, 0.1f, 100000.f); //set the projection, we want to be perspective

	GameStageManager::instance->ChangeStage("MainMenuStage");

	woodQuad.createQuad(-0.85f, -0.85f, 0.25f * ((float)window_width / (float)window_height), 0.25f, false);

	
	RayMesh.vertices.resize(2);
	Vector4 rayColor(1.f, 1.f, 0.f, 1.f);
	RayMesh.colors.push_back(rayColor);
	RayMesh.colors.push_back(rayColor);
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse

	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		//error abriendo la tarjeta de sonido...
		std::cout << "error abriendo la tarjeta de sonido..." << std::endl;
	}

	

}

void RenderGeneralMesh(const Matrix44& m, Mesh* mesh, Shader* shader, const Vector4& color, Texture* texture,
	int submesh_id, int primitive)
{
	if (shader == nullptr)
		shader = Shader::getDefaultShader("flat");

	shader->enable();

	shader->setUniform("u_color", color);
	shader->setUniform("u_viewprojection", Camera::current->viewprojection_matrix);
	if (texture != nullptr)
		shader->setUniform("u_texture", texture);
	shader->setUniform("u_model", m);
	mesh->render(primitive, submesh_id);
	shader->disable();
}

//what to do when the image has to be draw
void Game::render(void)
{

	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();


	//create model matrix for cube
	Matrix44 m;

	
	/* Do frustum culling with the AABB of the map */
	for (auto* map : Maps)
		if(map)
			map->Render();

	EntityManager::instance->Render();
	GameStageManager::instance->Render();
	StaticObjectRenderer::instance->Render(this->time_pause);

	//drawText(2.f, 2.f, getGPUStats(), Vector3(1.f, 1.f, 1.f), 2.f);

	GameStageManager::instance->RenderGUI();

	if (ShowRays)
	{
		for (auto& ray : Rays)
		{
			if (ray.Origin == ray.Collision)
				continue;

			RayMesh.vertices[0] = ray.Origin;
			RayMesh.vertices[1] = ray.Collision;
			RenderGeneralMesh(Matrix44(), &RayMesh, nullptr, Vector4(1.f, 1.f, 0.f, 0.75f), nullptr, 0, GL_LINES);
		}
	}

	

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

bool Game::IsNight() const
{
	return CurrentDayTime > 1.f;
}

bool Game::IsDay() const
{
	return CurrentDayTime <= 1.f;
}

void Game::update(double seconds_elapsed)
{
	StaticObjectRenderer::instance->StartFrame();
	CurrentDayTime = fmod(time_pause, DayDuration) / (DayDuration * 0.5f);
	++CurrentFrame;
	

	if (Input::wasKeyPressed(SDL_SCANCODE_2))
		EntityMesh::ShowAABB = !EntityMesh::ShowAABB;

	if (Input::wasKeyPressed(SDL_SCANCODE_3))
		ShowRays = !ShowRays;

	if (Input::wasKeyPressed(SDL_SCANCODE_4))
		mouse_locked = !mouse_locked;

	if (Input::wasKeyPressed(SDL_SCANCODE_5))
		EntityMesh::ShowFrontVector = !EntityMesh::ShowFrontVector;


	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();

	//***COMENTADO POR DAR ERROR EN MENUINSTROSTAGE ***

	for (auto* map : Maps)
		if(map)
			map->Update(seconds_elapsed);
	
	GameStageManager::instance->Update(seconds_elapsed);
	EntityManager::instance->Update(seconds_elapsed);
	ZombieManager::instance->Update(seconds_elapsed);

	
}

//Keyboard event handler (sync input)
void Game::onKeyDown(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
	//case SDLK_TAB: free_cam = !free_cam; break;
	case SDLK_F1: Shader::ReloadAll(); break;
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
}

void Game::onResize(int width, int height)
{
	std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport(0, 0, width, height);
	camera->aspect = width / (float)height;
	window_width = width;
	window_height = height;
}
