#ifndef GUIBAR_H
#define GUIBAR_H

#include <AntTweakBar.h>

#include <string>

class GuiAction;

class GuiBar {

public:
	GuiBar(const std::string& name, int visLevel = 0);
	virtual ~GuiBar();
	virtual void init() = 0;

	void setAlpha(int alpha); //0=trasparent 255=opaque

	void setColor(int r, int g, int b); //0-255

	void setValuesWidth(int width);

	void setUpdateInterval(float sec);
	void setVisible(bool visible);

	void setPosition(int x, int y);
	void getPosition(int *x, int *y) const;
	
	void setSize(int width, int height);
	void getSize(int *width, int *height) const;
		
	void addButton(const std::string& btnLabel, TwButtonCallback callback, const std::string& options);

	int getVisibilityLevel() const { return m_visibilityLevel; }
	/*void addInt();
	void addBool();
	void addVec3();
	void addOrientation();
	void addColor();*/

protected:
	TwBar       *m_barHandle;
private:	
	std::string  m_barName;
	float        m_updateInterval;
	int          m_hidden;
	int          m_visibilityLevel;
};

#endif