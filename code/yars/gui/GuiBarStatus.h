#ifndef GUIBARSTATUS_H
#define GUIBARSTATUS_H

#include "GuiBar.h"

#include "../Sandbox.h"

#include <sstream>
#include <vector>
#include <memory>

namespace GuiBarStatus_ns {
	
}


class GuiBarStatus : public GuiBar {
public:
	GuiBarStatus() : GuiBar("Status", 2) {} ;
	virtual ~GuiBarStatus() {};

	void init() {
		//status = "loading bla bla bla";
		TwRemoveAllVars(m_barHandle);

		TwAddVarRO(m_barHandle, ":", TW_TYPE_STDSTRING, &status, "" ); 
	}	

	void setStatusString(const std::string& newstatus) {
		status = newstatus;
	}
private:
	std::string status;
};

#endif