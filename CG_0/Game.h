#pragma once
#include <iostream>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <chrono>
#include <vector>
#include <windows.h>
#include <WinUser.h>
#include <wrl.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

class Component;
class DisplayWin32;
class InputDevice;

class Game
{
public:
	
	DisplayWin32* DW32;
	static InputDevice* inputDevice;
	static Game* game;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* backTex;
	ID3D11RenderTargetView* rtv;
	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float totalTime;
	unsigned int frameCount;
	//UINT pointsCount;

	std::vector<Component*> comps = {};

	HRESULT res;

	int scoreL = 0;
	int scoreR = 0;
	int dead = 0;

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

	void PrepareResources(UINT windowHeight, UINT windowWidth);

	void Initialize(Component* comp);

	void Draw();

	void Run();

	void PushRackets(int key);

	void Exit();

	Game() {
		game = this;
	};
};