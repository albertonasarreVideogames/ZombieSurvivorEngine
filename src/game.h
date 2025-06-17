
#pragma once

#ifndef GAME_H
#define GAME_H

#include "includes.h"
#include "camera.h"
#include "utils.h"
#include "TileMap.h"

class Game
{
	Mesh RayMesh;
public:
	static Game* instance;

	static constexpr auto DayDuration = 40.f;
	static constexpr int PlayerMaxHealth = 100;
	static constexpr size_t MapAmount = 9;
	static Mesh woodQuad;

	int CurrentLevel;
	float CurrentDayTime;
	size_t CurrentFrame;
	bool ShowRays;
	int32 WoodCounter;
	std::vector<CollidedRay> Rays;

	bool IsNight()const;
	bool IsDay()const;

	struct Splayer
	{
		Vector3 position;
		float angle;
		float pitch;
		Matrix44 model;
		int Health;
		Splayer()
			:position(0.f, 0.f, 0.f)
			,angle(0.f)
			,pitch(0.f)
			,Health(PlayerMaxHealth)
		{

		}
	};

	Splayer player;

	std::array<TileMap*, MapAmount> Maps;
	TileMap* GetMapAt(const Vector3& position)const;

	//window
	SDL_Window* window;
	int window_width;
	int window_height;

	//some globals
	long frame;
    float time;
	float time_pause;
	bool pause;
	bool pause_clean;
	float elapsed_time;
	int fps;
	bool must_exit;
	int Score;

	//some vars
	Camera* camera; //our global camera
	bool mouse_locked; //tells if the mouse is locked (not seen)

	Game( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void update( double dt );
	
	//events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onGamepadButtonDown(SDL_JoyButtonEvent event);
	void onGamepadButtonUp(SDL_JoyButtonEvent event);
	void onResize(int width, int height);
};

void RenderGeneralMesh(const Matrix44& m, class Mesh* mesh, class Shader* shader,
	const Vector4& color = Vector4(1.f,1.f,1.f,1.f), class Texture* texture = nullptr,
	int submesh_id = 0, int primitive = GL_TRIANGLES);

#endif 