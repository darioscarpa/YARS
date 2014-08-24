#include "Input.h"

#include "RenderWindow.h"

Input::Input() {
	mouseDownBuf[BTN_LEFT] = mouseDownBuf[BTN_MIDDLE] = mouseDownBuf[BTN_RIGHT] = false;
	mouseWheelLevel = 0;
	for (auto b : keyDownBuf) b = false;	 

	mouseX = mouseY = lastKeyPressed = 0;
}

void Input::keyCB( int key, int action ) {
	if (action == RenderWindow::keyAction::KEY_PRESS) {
		keyDownBuf[key] = true;
	} else if (action == RenderWindow::keyAction::KEY_RELEASE) {
		keyDownBuf[key] = false;
		lastKeyPressed = key;
	}
}

void Input::mousePosCB(int mX, int mY) {
	mouseX = mX;
	mouseY = mY;
}

void Input::mouseWheelCB(int pos) {
	mouseWheelLevel = pos;
}

void Input::mouseButtonCB(int btn, int action) {
	switch(btn) {
		case RenderWindow::mouseBtn::LEFT:
		case RenderWindow::mouseBtn::RIGHT:
		case RenderWindow::mouseBtn::MIDDLE:
			mouseDownBuf[btn] = action == RenderWindow::mouseBtnAction::MBTN_PRESS;
	}	
}

