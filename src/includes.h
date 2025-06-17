#pragma once

#ifndef INCLUDES_H
#define INCLUDES_H

#if defined(_DEBUG) || defined(DEBUG)
#define GAME_DEBUG 1
#else
#define GAME_DEBUG 0
#endif

//under windows we need this file to make opengl work
#ifdef WIN32 
#define _CRT_SECURE_NO_WARNINGS 1
#define WIN32_LEAN_AND_MEAN 1
	#include <windows.h>
#endif

#ifndef APIENTRY
    #define APIENTRY
#endif

#ifdef WIN32
	#define USE_GLEW
	#define GLEW_STATIC
	#include <GL/glew.h>
	#pragma comment(lib, "glew32s.lib")
#endif


//SDL
//#pragma comment(lib, "SDL2.lib")
//#pragma comment(lib, "SDL2main.lib")


#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>



//GLUT
#ifdef WIN32
    #include <GL/glext.h>
    #include "GL/GLU.h"
#endif

#ifdef __APPLE__
    #include <OpenGL/glext.h>
    #include "OpenGL/glu.h"
#endif

#include <iostream>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cmath>
#include <cassert>
#include <cctype>
#include <locale>
#include <algorithm> 
#include <functional> 
#include <limits>
#include <sys/stat.h>


//remove warnings

//used to access opengl extensions
#define REGISTER_GLEXT(RET, FUNCNAME, ...) typedef RET ( * FUNCNAME ## _func)(__VA_ARGS__); FUNCNAME ## _func FUNCNAME = NULL; 
#define IMPORT_GLEXT(FUNCNAME) FUNCNAME = (FUNCNAME ## _func) SDL_GL_GetProcAddress(#FUNCNAME); if (FUNCNAME == NULL) { std::cout << "ERROR: This Graphics card doesnt support " << #FUNCNAME << std::endl; }


//OPENGL EXTENSIONS



#define INSTANCED_ZOMBIES 0


#endif
