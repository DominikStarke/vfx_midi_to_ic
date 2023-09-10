#pragma once
#ifndef MIDI_TO_IC_UI_H
#define MIDI_TO_IC_UI_H

#include "plugin-bindings/plugguieditor.h"
#include "lib/platform/platform_win32.h"
#include "lib/platform/iplatformfont.h"
#include "lib/controls/cparamdisplay.h"
#include "lib/ccolor.h"
#include "lib/cfont.h"
#include "midi_to_ic.h"
#include "linear_knob.h"

// using namespace VSTGUI;

class MidiIC;

class midi_to_ic_ui : public VSTGUIEditorInterface, public IControlListener
{
public:
	CControl* noop = nullptr;
	struct {
		CColor colorlightBlue = CColor(188, 203, 210, 255);
		CColor colorDarkGrey = CColor(46, 50, 52, 255);
		CColor colorLightGrey = CColor(216, 216, 216, 255);
		CColor colorFrameBackground = CColor(49, 55, 59, 255);
		CColor colorContainerBackground = CColor(54, 60, 64, 255);
		CColor colorContainerBorder = CColor(39, 45, 49, 255);
		CColor colorTransparent = VSTGUI::kTransparentCColor;
		CFontDesc* defaultFontBold;
		CFontDesc* defaultFont;
		CBitmap* background = new CBitmap("background.png");
		CBitmap* radio_button = new CBitmap("radio_button_green_s.png");
	} theme;

	struct TextEditOptions {
		int tag;
		int x;
		int y;
		int width;
		int height;
		char* text;
	};

	struct RadioButtonOptions {
		int tag;
		int x;
		int y;
	};

	struct TextLabelOptions {
		int x;
		int y;
		int width;
		int height;
		char* text;
	};

	enum UITags {
		EDIT_CONTROL_LIMIT_PORT_ENABLE,
		EDIT_CONTROL_LIMIT_CHANNEL_ENABLE,
		EDIT_CONTROL_CHANNEL_NUM,
		EDIT_CONTROL_PORT_NUM,
		EDIT_CONTROL_STRIP_NOTES,
		EDIT_CONTROL_STRIP_CC,
		EDIT_CONTROL_RECORD_NOTES
	};

	midi_to_ic_ui(MidiIC* effect, void* ptr);
	virtual ~midi_to_ic_ui();
	virtual void doIdleStuff();
	void setParameter(int32_t index, float value);
	void* getHWND();
	virtual void valueChanged(CControl* pControl);

private:
    virtual CFontDesc* Font(char* name, int size, int style);
    virtual CTextLabel* TextLabel(TextLabelOptions options);
protected:
	MidiIC* plugin;
};

#endif