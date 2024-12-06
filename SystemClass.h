#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "InputClass.h"
#include "ApplicationClass.h"


class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool initialize();
	void shutdown();
	void run();

	// windows shenanigans
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool frame();
	void initializeWindows(int&, int&);
	void shutdownWindows();

	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	
	InputClass* m_input;
	ApplicationClass* m_application;
};

// redirectiong windows messaging into MessageHandler (whatever that means)
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* ApplicationHandle = 0;