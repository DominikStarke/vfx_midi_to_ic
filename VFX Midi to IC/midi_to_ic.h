#pragma once

#include "midi_to_ic_ui.h"
#include <windows.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <thread>
#include "fp_plugclass.h"
#include "fp_cplug.h"
#include "generictransport.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class midi_to_ic_ui;

enum MIDIEventTypes {
	MIDI_NOTE_ON = 0x09,
	MIDI_NOTE_OFF = 0x08,
	MIDI_CC = 0x0B,
	MIDI_PITCH_BEND = 0x0E
	// FIXME: Expand
};

typedef struct {
	unsigned int channel : 4;
	unsigned int type : 4;
	int cc;
	int value;
	int port;
} MidiMessage;


class MidiIC : public TCPPFruityPlug
{
public:
	MidiIC(int SetHostTag, TFruityPlugHost* SetPlugHost, HINSTANCE SetInstance);
	~MidiIC();
	intptr_t _stdcall Dispatcher(intptr_t ID, intptr_t Index, intptr_t Value);
	intptr_t dispatch(intptr_t ID, intptr_t Index, intptr_t Value);
	void _stdcall Idle_Public();
	void _stdcall Idle();
	void _stdcall SaveRestoreState(IStream* Stream, BOOL Save);
	void _stdcall GetName(int Section, int Index, int Value, char* Name);
	int _stdcall ProcessEvent(int EventID, int EventValue, int Flags); 
	int _stdcall ProcessParam(int Index, int Value, int RECFlags); 
	void _stdcall Eff_Render(PWAV32FS SourceBuffer, PWAV32FS DestBuffer, int Length); 
	void _stdcall Gen_Render(PWAV32FS DestBuffer, int& Length);  
	TVoiceHandle _stdcall TriggerVoice(PVoiceParams VoiceParams, intptr_t SetTag); 
	void _stdcall Voice_Release(TVoiceHandle Handle); 
	void _stdcall Voice_Kill(TVoiceHandle Handle); 
	int _stdcall Voice_ProcessEvent(TVoiceHandle Handle, int EventID, int EventValue, int Flags); 
	int _stdcall Voice_Render(TVoiceHandle Handle, PWAV32FS DestBuffer, int& Length); 
	void _stdcall NewTick();  
	void _stdcall MIDITick();  
	void _stdcall MIDIIn(int& Msg); 
	void _stdcall MsgIn(intptr_t Msg); 
	int _stdcall OutputVoice_ProcessEvent(TOutVoiceHandle Handle, int EventID, int EventValue, int Flags); 
	void _stdcall OutputVoice_Kill(TVoiceHandle Handle);

	int param_limit_channel_num = 0;
	int param_limit_port_num = 0;

	bool update_host_param = false;

	std::map<std::string, std::pair<int, long long int>> state;
	std::map<int, std::pair<std::string, int>> _state;
	std::vector<std::string> paths;
	std::map<std::string, std::pair<int, long long int>> changes;

protected:
	TFruityPlugHost* _host;
	midi_to_ic_ui* _editor;
};
