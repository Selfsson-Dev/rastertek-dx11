#include "ApplicationClass.h"

ApplicationClass::ApplicationClass()
{
}

ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::initialize(int screenWidth, int screenHeight, HWND hwnd)
{

	return true;
}

void ApplicationClass::shutdown()
{

	return;
}

bool ApplicationClass::frame()
{

	return true;
}

bool ApplicationClass::render()
{

	return true;
}
