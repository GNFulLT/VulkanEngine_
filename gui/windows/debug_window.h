#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H


#include "../window.h"

class DebugWindow : public Window
{
	OBJECT_DEF(DebugWindow,Window)
public:
	DebugWindow(const String& name) : Window(name)
	{}

	virtual void render() override;

private:
};

#endif // DEBUG_WINDOW_H