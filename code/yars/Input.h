#ifndef INPUT_H
#define INPUT_H

class Input {
	public:
		
		enum mouseBtn {BTN_LEFT, BTN_MIDDLE, BTN_RIGHT};
		
		Input();
		
		void keyCB( int key, int action );
		void mousePosCB(int mX, int mY);
		void mouseWheelCB(int pos);
		void mouseButtonCB(int btn, int action);
						
		inline int getLastKeyPressed() { int ret = lastKeyPressed; lastKeyPressed = 0; return ret; }
		
		inline bool keyDown(int k)        { return keyDownBuf[k];   }
		inline bool mouseDown(mouseBtn k) { return mouseDownBuf[k]; }
		inline int  mouseWheelScroll()    { return mouseWheelLevel; }

		inline int getMouseX()         { return mouseX; }
		inline int getMouseY()         { return mouseY; }		

	private:
		int  mouseX;
		int  mouseY;
		
		int  lastKeyPressed;		
		bool mouseDownBuf[3];
		int  mouseWheelLevel;						

		bool keyDownBuf[512];
};

#endif