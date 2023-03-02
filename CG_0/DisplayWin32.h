#pragma once
#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <iostream>

class Game;

class DisplayWin32
{

public:
	Game* game;

	UINT ClientHeight;
	UINT ClientWidth;
	HINSTANCE hInstance;
	WNDCLASSEX wc;
	HWND hWnd;
	LPCWSTR applicationName;

	//static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

	DisplayWin32(UINT height, UINT width, Game* newgame);

	//DisplayWin32();
};

