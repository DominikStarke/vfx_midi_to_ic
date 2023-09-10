#pragma once
#include "linear_knob.h"


//------------------------------------------------------------------------
// CLinearKnob
//------------------------------------------------------------------------
/*! @class CLinearKnob
Such as a CKnob control object, but there is a unique bitmap which contains different views (subbitmaps) of this knob.
According to the value, a specific subbitmap is displayed. The different subbitmaps are stacked in the bitmap object.
*/
//------------------------------------------------------------------------
/**
 * CLinearKnob constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the background bitmap
 * @param offset unused
 */
 //------------------------------------------------------------------------
CLinearKnob::CLinearKnob(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset)
	: CKnobBase(size, listener, tag, background)
	, bInverseBitmap(false)
{
	heightOfOneImage = size.getHeight();
	setNumSubPixmaps(background ? (int32_t)(background->getHeight() / heightOfOneImage) : 0);
	inset = 0;
}

//------------------------------------------------------------------------
/**
 * CLinearKnob constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage the height of one sub bitmap
 * @param background the background bitmap
 * @param offset unused
 */
 //------------------------------------------------------------------------
CLinearKnob::CLinearKnob(const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset)
	: CKnobBase(size, listener, tag, background)
	, bInverseBitmap(false)
{
	setNumSubPixmaps(subPixmaps);
	setHeightOfOneImage(heightOfOneImage);
	inset = 0;
}

//------------------------------------------------------------------------
CLinearKnob::CLinearKnob(const CLinearKnob& v)
	: CKnobBase(v)
	, bInverseBitmap(v.bInverseBitmap)
{
	setNumSubPixmaps(v.subPixmaps);
	setHeightOfOneImage(v.heightOfOneImage);
}

//-----------------------------------------------------------------------------------------------
bool CLinearKnob::sizeToFit()
{
	if (getDrawBackground())
	{
		CRect vs(getViewSize());
		vs.setWidth(getDrawBackground()->getWidth());
		vs.setHeight(getHeightOfOneImage());
		setViewSize(vs);
		setMouseableArea(vs);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------
void CLinearKnob::setHeightOfOneImage(const CCoord& height)
{
	IMultiBitmapControl::setHeightOfOneImage(height);
	if (getDrawBackground() && heightOfOneImage > 0)
		setNumSubPixmaps((int32_t)(getDrawBackground()->getHeight() / heightOfOneImage));
}

//-----------------------------------------------------------------------------------------------
void CLinearKnob::setBackground(CBitmap* background)
{
	CKnobBase::setBackground(background);
	if (heightOfOneImage == 0)
		heightOfOneImage = getViewSize().getHeight();
	if (background && heightOfOneImage > 0)
		setNumSubPixmaps((int32_t)(background->getHeight() / heightOfOneImage));
}

//------------------------------------------------------------------------
void CLinearKnob::draw(CDrawContext* pContext)
{
	if (getDrawBackground())
	{
		CPoint where(0, 0);
		float val = getValueNormalized();
		if (val >= 0.f && heightOfOneImage > 0.)
		{
			CCoord tmp = heightOfOneImage * (getNumSubPixmaps() - 1);
			if (bInverseBitmap)
				where.y = floor((1. - val) * tmp);
			else
				where.y = floor(val * tmp);
			where.y -= (int32_t)where.y % (int32_t)heightOfOneImage;
		}

		getDrawBackground()->draw(pContext, getViewSize(), where);
	}
	setDirty(false);
}

struct CLinearKnob::MouseEditingState
{
	CPoint firstPoint;
	CPoint lastPoint;
	float startValue;
	float entryState;
	float range;
	float coef;
	CButtonState oldButton;
	bool modeLinear;
};

auto CLinearKnob::getMouseEditingState() -> MouseEditingState&
{
	MouseEditingState* state = nullptr;
	if (!getAttribute(::kCKnobMouseStateAttribute, state))
	{
		state = new MouseEditingState;
		setAttribute(kCKnobMouseStateAttribute, state);
	}
	return *state;
}

CMouseEventResult CLinearKnob::onMouseDown(CPoint& where, const CButtonState& buttons)
{
	if (!buttons.isLeftButton())
		return CMouseEventResult::kMouseEventNotHandled;

	invalidMouseWheelEditTimer(this);
	beginEdit();

	/// FIXME: VSTGUI 4.10 -> VSTGUI 4.11
	/// CControl::checkDefaultValue(CButtonState) was removed and replaced by a generic method which uses the static function CControl::CheckDefaultValueEventFunc to reset a control to its default value
	if (checkDefaultValue(buttons))
	{
		endEdit();
		return CMouseEventResult::kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	auto& mouseState = getMouseEditingState();
	mouseState.firstPoint = where;
	mouseState.startValue = getOldValue();
	mouseState.entryState = value;
	mouseState.range = getNumSubPixmaps() * 2.0f;
	mouseState.oldButton = buttons;
	if (buttons & kZoomModifier)
		mouseState.range *= zoomFactor;
	mouseState.lastPoint = where;
	mouseState.modeLinear = true;
	mouseState.coef = (getMax() - getMin()) / mouseState.range;

	auto v = getNumSubPixmaps();

	return onMouseMoved(where, buttons);
}

CMouseEventResult CLinearKnob::onMouseEntered(CPoint& where, const CButtonState& buttons) {
	getFrame()->setCursor(VSTGUI::CCursorType::kCursorVSize);
	return CMouseEventResult::kMouseEventHandled;
}

CMouseEventResult CLinearKnob::onMouseExited(CPoint& where, const CButtonState& buttons) {
	getFrame()->setCursor(VSTGUI::CCursorType::kCursorDefault);
	return CMouseEventResult::kMouseEventHandled;
}

CMouseEventResult CLinearKnob::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton() && isEditing())
	{
		auto& mouseState = getMouseEditingState();

		float middle = (getMax() - getMin()) * 0.5f;

		if (where != mouseState.lastPoint)
		{
			mouseState.lastPoint = where;

			CCoord diff = (mouseState.firstPoint.y - where.y);
			if (buttons != mouseState.oldButton)
			{
				mouseState.range = kCKnobRange;
				if (buttons & kZoomModifier)
					mouseState.range *= zoomFactor;

				float coef2 = (getMax() - getMin()) / mouseState.range;
				mouseState.entryState += (float)(diff * (mouseState.coef - coef2));
				mouseState.coef = coef2;
				mouseState.oldButton = buttons;
			}
			value = (float)(mouseState.entryState + diff * mouseState.coef);
			bounceValue();
			

			if (value != getOldValue())
				valueChanged();
			if (isDirty())
				invalid();
		}
		return CMouseEventResult::kMouseEventHandled;
	}
	return CMouseEventResult::kMouseEventNotHandled;
}

//------------------------------------------------------------------------
bool CLinearKnob::onWheel(const CPoint& where, const VSTGUI::CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{
	int numValues = getNumSubPixmaps();

	if (!getMouseEnabled())
		return false;

	onMouseWheelEditing(this);

	float v = getValueNormalized();
	float current = floor(numValues * v);
	float step = 1.0f / numValues;

	current += distance;
	if (current < 0) current = 0;

	v = step * current;
	//if (buttons & kZoomModifier)
	//	v += 0.1f * distance * wheelInc;
	//else
	//	v += distance * wheelInc;
	setValueNormalized(v);

	if (isDirty())
	{
		invalid();
		valueChanged();
	}
	return true;
}