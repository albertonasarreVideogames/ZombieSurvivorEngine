#pragma once

#include "includes.h"
#include <bass.h>

class Audio
{
public:

	std::string name;

	static std::map<std::string, Audio*> sLoadedAudios; //para nuestro manager
	HSAMPLE sample; //aqui guardamos el handler del sample que retorna BASS_SampleLoad

	Audio(); //importante poner sample a cero aqui
	~Audio(); //aqui deberiamos liberar el sample con BASS_SampleFree

	HCHANNEL play(float volume); //lanza el audio y retorna el channel donde suena
	void stop();
	void changeVolume(float volume);

	static void ChangeVolume(HCHANNEL channel, float volume);
	static void Stop(HCHANNEL channel); //para parar un audio necesitamos su channel
	static Audio* Get(const char* filename,bool loop); //manager de audios 
	static HCHANNEL* Play(const char* filename); //version estática para ir mas rapido

	void registerAudio(std::string name);
};
