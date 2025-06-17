#include "Audio.h"

std::map<std::string, Audio*> Audio::sLoadedAudios;

Audio::Audio(){}

Audio::~Audio() {

	BASS_SampleFree(sample);
}


Audio* Audio::Get(const char* filename,bool loop) {

	assert(filename);
	std::map<std::string, Audio*>::iterator it = sLoadedAudios.find(filename);
	if (it != sLoadedAudios.end())
		return it->second;




	
	
	Audio* a = new Audio();
	std::string name = filename;

	int loop_int = 0;

	if (loop) { loop_int = BASS_SAMPLE_LOOP; }
	

	a->sample = BASS_SampleLoad(false, filename, 0, 0, 3, loop_int);

	if (a->sample == 0)
	{
		//file not found
	}


	//this->name = name;
	a->registerAudio(name);
	return a;

}

HCHANNEL Audio::play(float volume) {

	

	HCHANNEL hSampleChannel = BASS_SampleGetChannel(sample, false);

	
	BASS_ChannelSetAttribute(hSampleChannel, BASS_ATTRIB_VOL,volume);

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);

	return hSampleChannel;

}

void Audio::registerAudio(std::string name)
{
	this->name = name;
	sLoadedAudios[name] = this;
}

void Audio::stop() {

	HCHANNEL hSampleChannel = BASS_SampleGetChannel(sample, false);

	BASS_ChannelStop(hSampleChannel);

}

void Audio::Stop(HCHANNEL channel) {

	BASS_ChannelStop(channel);

}

void Audio::changeVolume(float volume) {

	HCHANNEL hSampleChannel = BASS_SampleGetChannel(sample, false);

	BASS_ChannelSetAttribute(hSampleChannel, BASS_ATTRIB_VOL, volume);

}

void Audio::ChangeVolume(HCHANNEL channel,float volume) {


	BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

}