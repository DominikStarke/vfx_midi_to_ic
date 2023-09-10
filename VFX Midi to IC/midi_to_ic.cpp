
#include "midi_to_ic.h"

// Plug-in information
TFruityPlugInfo PlugInfo =
{
	CurrentSDKVersion,
	"VFX Midi to IC",
	"VFX Midi to IC",
	FPF_Type_Visual | FPF_WantNewTick | FPF_CantSmartDisable,
	4096,
	0,
	4096 // the amount of Out Controls
};

// DLL entry
void* hInstance; // used by VSTGUI

extern "C" BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hInst;
	return TRUE;
}

// External init call
extern "C" TCPPFruityPlug* _stdcall CreatePlugInstance(TFruityPlugHost *Host, int Tag)
{
	return new MidiIC(Tag, Host, reinterpret_cast<HINSTANCE>(hInstance));
};

// constructor
MidiIC::MidiIC(int SetHostTag, TFruityPlugHost* SetPlugHost, HINSTANCE SetInstance)
	: TCPPFruityPlug(SetHostTag, SetPlugHost, SetInstance)
{
	Info = &PlugInfo;
	HostTag = SetHostTag;
	EditorHandle = 0;
	_host = SetPlugHost;
	_editor = nullptr;

	//SetPlugHost->Dispatcher(HostTag, FHD_ActivateMIDI, 1, 1);
	SetPlugHost->Dispatcher(HostTag, FHD_WantMIDIInput, 1, 1);
}

// destructor
MidiIC::~MidiIC()
{
	delete _editor;
}

// Save or Load; nlohmann json
void _stdcall MidiIC::SaveRestoreState(IStream* Stream, BOOL Save)
{
	if (Save)
	{
		json data = {
			{"port", param_limit_port_num},
			{"channel", param_limit_channel_num},
			{"parameters", json::array()},
		};

		for (auto& it : state) {
			data["parameters"][it.second.first] = {
				{"path", it.first},
				{"index", it.second.first},
				{"value", it.second.second == NULL ? 0 : it.second.second}
			};
		}

		std::string jString = data.dump();

		long len = (long)jString.length();
		Stream->Write(&len, 4, NULL);
		Stream->Write(jString.c_str(), len, NULL);
	}
	else
	{
		json data;
		long length = 0xFFFFFFFF;

		Stream->Read(&length, 4, NULL);
		char* buffer = (char*)malloc(length);

		state.clear();
		paths.clear();

		if (buffer != NULL)
		{
			Stream->Read(buffer, length, NULL);
			state.empty();

			std::string jsonData(buffer, length);
			data = json::parse(jsonData);
			param_limit_port_num = data["port"];
			param_limit_channel_num = data["channel"];
			for (auto& x : data["parameters"].items())
			{
				auto val = x.value();
				paths.push_back(val["path"]);
				state[val["path"]] = std::make_pair(val["index"], val["value"]);
			}
			free(buffer);
			_host->Dispatcher(HostTag, FHD_NamesChanged, 1, FPN_OutCtrl);
			_host->Dispatcher(HostTag, FHD_NamesChanged, 1, FPN_Param);
			//evt_changes = true;
		}
	}
}

// 
intptr_t _stdcall MidiIC::Dispatcher(intptr_t ID, intptr_t Index, intptr_t Value)
{
	int val = 0;
	if (ID == FPD_ShowEditor)
	{
		if (Value == 0)
		{
			// close editor
			delete _editor;
			_editor = nullptr;
			EditorHandle = 0;
		}
		else if (EditorHandle == 0)
		{
			if (_editor == nullptr)
			{
				_editor = new midi_to_ic_ui(this, reinterpret_cast<HWND>(Value));
			}

			// open editor
			EditorHandle = reinterpret_cast<HWND>(_editor->getHWND());
			return Value;
		}
		else
		{
			// change parent window ?
			::SetParent(EditorHandle, reinterpret_cast<HWND>(Value));
			return Value;
		}
	}
	else if (ID == FPD_SetFocus && _editor != nullptr)
	{
		InvalidateRect(EditorHandle, NULL, TRUE); // If we don't do this the editor window will turn black if it is overlayed
	}
	return val;

}

// Get the parameters name, called from FLS
void _stdcall MidiIC::GetName(int Section, int Index, int Value, char *Name)
{
	if(Section == FPN_OutCtrl)
	{
		if (Index < state.size())
		{
			for (auto& it : state) {
				std::string path = it.first;
				std::pair<int, long long int> val = it.second;
				if (it.second.first == Index) {
					strcpy_s(Name, 256, path.c_str());
				}
			}
		}
	}
	else if (Section == FPN_Param) {
		if (Index < state.size())
		{
			for (auto& it : state) {
				std::string path = it.first;
				std::pair<int, long long int> val = it.second;
				if (it.second.first == Index) {
					strcpy_s(Name, 256, path.c_str());
				}
			}
		}
	}
}


// Not sure if necessary
int _stdcall MidiIC::ProcessParam(int Index, int Value, int RECFlags)
{
	// update the value in the ParamValue array
	if (RECFlags & REC_UpdateValue)
	{
		state[paths[Index]].second = Value;
		// _host->OnControllerChanged(HostTag, Index, Value);
	}
	// retrieve the value from the ParamValue array
	else if (RECFlags & REC_GetValue) {
		Value = state[paths[Index]].second;
	}

	return Value;
}

// Called on midi message
void _stdcall MidiIC::MIDIIn(int& Msg)
{
	short channel = Msg & 0x0F;
	int type      = (Msg >> 4) & 0x0F;
	short cc      = (Msg >> 8) & 0xFF;
	int value     = (Msg >> 16) & 0xFF;
	int port      = (Msg >> 24) & 0xFF;
	
	if (param_limit_channel_num != 0 && channel + 1 != param_limit_channel_num) return;
	if (param_limit_port_num != 0 && port != param_limit_port_num - 1) return;

	std::ostringstream _path;
	// Ignore Notes
	if (type == MIDI_NOTE_ON || type == MIDI_NOTE_OFF)
	{
		return;
	}
	// Ignore Pitch bend
	else if (type == MIDI_PITCH_BEND)
	{
		// Fixme: Reimplement
		// _path << "/midi/" << channel << "/pitch";
		return;
	}
	// CC 123, 6, 7, 100 and 101 are ignored because they are spammed
	else if (cc != 123 && cc != 6 && cc != 7 && cc != 100 && cc != 101)
	{
		_path << "/midi/" << channel << "/cc/" << cc;
	}
	else {
		return;
	}

	std::string path = _path.str();

	// Parameter doesn't exist yet
	if (state.find(path) == state.end()) {
		paths.push_back(path);
		state[path] = std::make_pair(state.size(), 0);
	}

	// FIXME:
	// Pitch bend needs to be handled differently, implement nrpn
	if (type == MIDI_PITCH_BEND)
	{
		//value = ((value & 0x7F) << 7 ) | (cc & 0x7F);
		//state[path].second = 0x40010004 * value;
		//changes[path].second = static_cast<int>(0x1FF * value);
		//_host->OnParamChanged(HostTag, state[path].first, value * 4);
	}
	else {
		state[path].second = static_cast<long long int>(value);
		update_host_param = true;
	}
}

void _stdcall MidiIC::NewTick()
{
	if (update_host_param && !state.empty()) {
		for (auto& it : state) {
			// _host->OnControllerChanged(HostTag, it.second.first, it.second.second * 0x1FF); /// For Mixer
			_host->OnControllerChanged(HostTag, it.second.first, it.second.second * 0x2040810204); /// For Patcher
		}
		update_host_param = false;
	}
	return;
}

// idle
void _stdcall MidiIC::Idle_Public()
{
	if (_editor) _editor->doIdleStuff();
}

void _stdcall MidiIC::Idle()
{
	if (_editor) _editor->doIdleStuff();
}

// Not so interesting functions below
int _stdcall MidiIC::ProcessEvent(int EventID, int EventValue, int Flags)
{
	return 0;
}


void _stdcall MidiIC::MsgIn(intptr_t Msg)
{
	return;
} 

int _stdcall MidiIC::OutputVoice_ProcessEvent(TOutVoiceHandle Handle, int EventID, int EventValue, int Flags)
{
	return 0;
}

void _stdcall MidiIC::OutputVoice_Kill(TVoiceHandle Handle)
{
	return;
}

intptr_t MidiIC::dispatch(intptr_t ID, intptr_t Index, intptr_t Value) {
	return _host->Dispatcher(HostTag, ID, Index, Value);
}

void _stdcall MidiIC::Eff_Render(PWAV32FS SourceBuffer, PWAV32FS DestBuffer, int Length)
{
	return;
}


void _stdcall MidiIC::Gen_Render(PWAV32FS DestBuffer, int& Length)
{
	return;
}

TVoiceHandle _stdcall MidiIC::TriggerVoice(PVoiceParams VoiceParams, intptr_t SetTag)
{
	return 0;
}

void _stdcall MidiIC::Voice_Release(TVoiceHandle Handle)
{
	return;
}

void _stdcall MidiIC::Voice_Kill(TVoiceHandle Handle)
{
	return;
}

int _stdcall MidiIC::Voice_ProcessEvent(TVoiceHandle Handle, int EventID, int EventValue, int Flags)
{
	return 0;
}

int _stdcall MidiIC::Voice_Render(TVoiceHandle Handle, PWAV32FS DestBuffer, int& Length)
{
	return 0;
}

void _stdcall MidiIC::MIDITick()
{
	return;
}