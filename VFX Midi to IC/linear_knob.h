#include "plugin-bindings/plugguieditor.h"
#include "lib/platform/iplatformfont.h"
#include "lib/controls/cparamdisplay.h"

using VSTGUI::CColor;
using VSTGUI::CTextEdit;
using VSTGUI::CFontDesc;
using VSTGUI::CTxtFace;
using VSTGUI::CRect;
using VSTGUI::CBitmap;
using VSTGUI::CFrame;
using VSTGUI::CControl;
using VSTGUI::VSTGUIEditorInterface;
using VSTGUI::IControlListener;
using VSTGUI::COnOffButton;
using VSTGUI::CTextLabel;
using VSTGUI::CViewContainer;
using VSTGUI::CView;
using VSTGUI::CDrawContext;
using VSTGUI::CKnobBase;
using VSTGUI::IMultiBitmapControl;
using VSTGUI::IMultiBitmapControl;
using VSTGUI::CAnimKnob;
using VSTGUI::CButtonState;

using VSTGUI::CPoint;
using VSTGUI::CCoord;
using VSTGUI::CKnobMode;

using VSTGUI::CMouseEventResult;

#if TARGET_OS_IPHONE
	static const float kCKnobRange = 300.f;
#else
	static const float kCKnobRange = 200.f;
#endif

static constexpr VSTGUI::CViewAttributeID kCKnobMouseStateAttribute = 'knms';

//-----------------------------------------------------------------------------
// CAnimKnob Declaration
//! @brief a bitmap knob control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CLinearKnob : public CKnobBase, public IMultiBitmapControl
{
public:
	enum DrawStyle {
		kLegacyHandleLineDrawing = 0,
		kHandleCircleDrawing = 1 << 0,
		kCoronaDrawing = 1 << 1,
		kCoronaFromCenter = 1 << 2,
		kCoronaInverted = 1 << 3,
		kCoronaLineDashDot = 1 << 4,
		kCoronaOutline = 1 << 5,
		kCoronaLineCapButt = 1 << 6,
		kSkipHandleDrawing = 1 << 7,
	};
	CLinearKnob(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint(0, 0));
	CLinearKnob(const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint(0, 0));
	CLinearKnob(const CLinearKnob& knob);

	//-----------------------------------------------------------------------------
	/// @name CAnimKnob Methods
	//-----------------------------------------------------------------------------
	//@{
	void setInverseBitmap(bool val) { bInverseBitmap = val; }
	bool getInverseBitmap() const { return bInverseBitmap; }
	//@}

	// overrides
	void draw(CDrawContext* pContext) override;
	bool sizeToFit() override;
	void setHeightOfOneImage(const CCoord& height) override;
	void setBackground(CBitmap* background) override;
	void setNumSubPixmaps(int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps(numSubPixmaps); invalid(); }

	CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
	bool onWheel(const CPoint& where, const VSTGUI::CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) override;
	//CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseEntered(CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited(CPoint& where, const CButtonState& buttons) override;

	CLASS_METHODS(CLinearKnob, CKnobBase)
private:
	struct MouseEditingState;

	MouseEditingState& getMouseEditingState();
protected:
	~CLinearKnob() noexcept override = default;
	bool	bInverseBitmap;
};
