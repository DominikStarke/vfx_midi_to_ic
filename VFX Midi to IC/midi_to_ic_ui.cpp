
#include "midi_to_ic_ui.h"
#include "fp_plugclass.h"


CTextLabel* midi_to_ic_ui::TextLabel(TextLabelOptions options) {
	CRect pos(0, 0, options.width, options.height);
	pos.offset(options.x, options.y);
	CTextLabel* cTextLabel = new CTextLabel(
		pos,
		options.text, nullptr, 0 | CTextLabel::kNoFrame);
	cTextLabel->setHoriAlign(VSTGUI::kRightText);
	cTextLabel->setBackColor(theme.colorTransparent);
	cTextLabel->setFont(theme.defaultFont);
	cTextLabel->setFontColor(theme.colorLightGrey);
	return cTextLabel;
}


CFontDesc* midi_to_ic_ui::Font(char* name, int size, int style) {
	CFontDesc* themeFont = new CFontDesc(name, size);

	if (themeFont) {
		auto f = themeFont->getPlatformFont();
		themeFont->setStyle(style);
	}
	return themeFont;
}

//----------------
// constructor
//----------------
midi_to_ic_ui::midi_to_ic_ui(MidiIC* effect, void* ptr)
	:plugin(effect)
{
	CRect frameSize(0, 0, 234, 50);
	this->frame = new CFrame(frameSize, this);
	this->frame->setFocusColor(theme.colorTransparent);
	this->frame->open(ptr);
	this->frame->setBackgroundColor(theme.colorFrameBackground);
	theme.defaultFontBold = Font("Arial", 11, 0 | CTxtFace::kBoldFace);
	theme.defaultFont = Font("Arial", 11, 0);

	frame->addView(TextLabel({
		10, 17, 25, 16, "Port"
	}));

	CRect portBounds(0, 0, 64, 30);
	portBounds.offset(40, 10);
	CBitmap* portBackground = new CBitmap("numknob_0_255.png");
	CLinearKnob* port = new CLinearKnob(
		portBounds,
		this,
		EDIT_CONTROL_PORT_NUM,
		257,
		30,
		portBackground,
		CPoint(0, 0)
	);
	port->setValue((1.0f / 256)* plugin->param_limit_port_num);
	frame->addView(port);


	frame->addView(TextLabel({
		110, 17, 45, 16, "Channel"
	}));

	CRect channelBounds(0, 0, 64, 30);
	channelBounds.offset(160, 10);
	CBitmap* background = new CBitmap("numknob_1_16.png");
	CLinearKnob* channel = new CLinearKnob(
		channelBounds,
		this,
		EDIT_CONTROL_CHANNEL_NUM,
		17,
		30,
		background,
		CPoint(0, 0)
	);
	channel->setValue((1.0f / 16)* plugin->param_limit_channel_num);


	frame->addView(channel);
}

midi_to_ic_ui::~midi_to_ic_ui()
{
	if (frame != nullptr)
	{
		frame->forget();
	}
}

void midi_to_ic_ui::valueChanged(CControl* pControl)
{
	if (pControl == nullptr) return;

	if (pControl->getTag() == EDIT_CONTROL_CHANNEL_NUM) {
		plugin->param_limit_channel_num = static_cast<int>(floor(16 * pControl->getValue()));

		std::string debug = std::to_string(floor(16 * pControl->getValue())) + "\n";
		OutputDebugStringA(debug.c_str());
		return;
	}
	else if (pControl->getTag() == EDIT_CONTROL_PORT_NUM) {
		plugin->param_limit_port_num = static_cast<int>(floor(256 * pControl->getValue()));

		std::string debug = std::to_string(floor(256 * pControl->getValue())) + "\n";
		OutputDebugStringA(debug.c_str());
		return;
	}
}

void midi_to_ic_ui::doIdleStuff()
{
	if (frame != nullptr)
	{
		frame->idle();
	}
}

void midi_to_ic_ui::setParameter(int32_t index, float value)
{
	return;
}

void* midi_to_ic_ui::getHWND()
{
	return frame->getPlatformFrame()->getPlatformRepresentation();
}
