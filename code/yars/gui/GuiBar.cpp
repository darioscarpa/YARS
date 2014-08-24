#include "GuiBar.h"

GuiBar::GuiBar(const std::string& name, int visLevel) : m_barName(name), m_hidden(false), m_visibilityLevel(visLevel) {
	m_barHandle = TwNewBar(m_barName.c_str());
	setUpdateInterval(1.0f);	
}

GuiBar::~GuiBar(){
	TwDeleteBar(m_barHandle);
}

void GuiBar::setAlpha(int alpha) {
	TwSetParam(m_barHandle, nullptr, "alpha", TW_PARAM_INT32, 1, &alpha);
}

void GuiBar::setColor(int r, int g, int b) {
	int color[] = {r, g, b};
	TwSetParam(m_barHandle, nullptr, "color", TW_PARAM_INT32, 3, &color);
}

void GuiBar::setValuesWidth(int w) {
	TwSetParam(m_barHandle, nullptr, "valueswidth", TW_PARAM_INT32, 1, &w);
}

void GuiBar::setUpdateInterval(const float updateInterval) {
	m_updateInterval = updateInterval;
	TwSetParam(m_barHandle, nullptr, "refresh", TW_PARAM_FLOAT, 1, &m_updateInterval);
}

void GuiBar::setVisible(bool visible) {
//	m_hidden = visible ? 0 : 1;
//	TwSetParam(m_barHandle,  nullptr, "iconified", TW_PARAM_INT32, 1, &m_hidden);
	m_hidden = visible ? 1 : 0;
	TwSetParam(m_barHandle,  nullptr, "visible", TW_PARAM_INT32, 1, &m_hidden);
}


void GuiBar::setPosition(int x, int y) {
	int pos[2] = { x, y } ;	
	TwSetParam(m_barHandle, nullptr, "position", TW_PARAM_INT32, 2, pos);
}

void GuiBar::getPosition(int *x, int *y)  const {
	int pos[2];
	TwGetParam(m_barHandle, nullptr, "position", TW_PARAM_INT32, 2, pos);
	*x = pos[0];
	*y = pos[1];
}

void GuiBar::getSize(int *width, int *height) const {
	int size[2];
	TwGetParam(m_barHandle, nullptr, "size", TW_PARAM_INT32, 2, size);
	*width  = size[0];
	*height = size[1];
}

void GuiBar::setSize(int width, int height) {
	int size[2] = { width, height } ;	
	TwSetParam(m_barHandle, nullptr, "size", TW_PARAM_INT32, 2, size);
}