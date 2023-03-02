#include "Game.h"
#include "Component.h"
#include "DisplayWin32.h"
#include "InputDevice.h"

Game* Game::game;
InputDevice* Game::inputDevice = nullptr;

LRESULT CALLBACK Game::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	//case WM_KEYDOWN:
	//{
	//	// If a key is pressed send it to the input object so it can record that state.
	//	std::cout << "Key: " << static_cast<unsigned int>(wparam) << std::endl;

	//	if (static_cast<unsigned int>(wparam) == 27) {
	//		PostQuitMessage(0);
	//	}
	//	/*else if ((static_cast<unsigned int>(wparam) == 38)) {
	//		PushRackets(38);
	//	}
	//	else if ((static_cast<unsigned int>(wparam) == 40)) {
	//		PushRackets(40);
	//	}*/
	//	return 0;
	//}
	//default:
	//{
	//	return DefWindowProc(hwnd, umessage, wparam, lparam);
	//}

	case WM_INPUT:
	{
		UINT dwSize = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == nullptr) {
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb);

		if (raw->header.dwType == RIM_TYPEKEYBOARD)
		{
			//printf(" Kbd: make=%04i Flags:%04i Reserved:%04i ExtraInformation:%08i, msg=%04i VK=%i \n",
			//	raw->data.keyboard.MakeCode,
			//	raw->data.keyboard.Flags,
			//	raw->data.keyboard.Reserved,
			//	raw->data.keyboard.ExtraInformation,
			//	raw->data.keyboard.Message,
			//	raw->data.keyboard.VKey);

			inputDevice->OnKeyDown({
				raw->data.keyboard.MakeCode,
				raw->data.keyboard.Flags,
				raw->data.keyboard.VKey,
				raw->data.keyboard.Message
				});
		}
		else if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			//printf(" Mouse: X=%04d Y:%04d \n", raw->data.mouse.lLastX, raw->data.mouse.lLastY);
			inputDevice->OnMouseMove({
				raw->data.mouse.usFlags,
				raw->data.mouse.usButtonFlags,
				static_cast<int>(raw->data.mouse.ulExtraInformation),
				static_cast<int>(raw->data.mouse.ulRawButtons),
				static_cast<short>(raw->data.mouse.usButtonData),
				raw->data.mouse.lLastX,
				raw->data.mouse.lLastY
				});
		}

		delete[] lpb;
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
	
	default:
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
}

void Game::PrepareResources(UINT windowHeight, UINT windowWidth)
{
	
	DW32 = new DisplayWin32(windowHeight, windowWidth, this);
	inputDevice = new InputDevice(this);

	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Width = DW32->ClientWidth;
	swapDesc.BufferDesc.Height = DW32->ClientHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = DW32->hWnd;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	res = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&swapChain,
		&device,
		nullptr,
		&context);

	if (FAILED(res))
	{
		//lol
		std::cout << "Failed to generate SwapChain and Device";
	}

	res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);	// __uuidof(ID3D11Texture2D)
	res = device->CreateRenderTargetView(backTex, nullptr, &rtv);
}

void Game::Initialize(Component* comp)
{
	comps.push_back(comp);
}

void Game::Draw()
{
	int ballIdx = -1;
	for (int i = 0; i < comps.size(); i++) {
		if (comps[i]->ball) ballIdx = i;
	}

	float ballUp, ballDown, racketUp, racketDown;
	ballUp = comps[ballIdx]->points[2].y + comps[ballIdx]->offset[1].y;
	ballDown = comps[ballIdx]->points[0].y + comps[ballIdx]->offset[0].y;

	comps[ballIdx]->bounce = false;

	for (int i = 0; i < comps.size(); i++) {
		racketUp = comps[i]->points[2].y + comps[i]->offset[1].y;
		racketDown = comps[i]->points[0].y + comps[i]->offset[0].y;

		//std::cout << "racket: " << racketUp << " " << racketDown << std::endl;

		if (i != ballIdx && !comps[i]->left && comps[ballIdx]->offset[2].x + comps[ballIdx]->xVelocity >= 0.94 && ballUp >= racketDown && ballUp <= racketUp ||
			i != ballIdx && !comps[i]->left && comps[ballIdx]->offset[2].x + comps[ballIdx]->xVelocity >= 0.94 && ballDown >= racketDown && ballDown <= racketUp ||
			i != ballIdx && comps[i]->left && comps[ballIdx]->offset[0].x + comps[ballIdx]->xVelocity <= -0.94 && ballUp >= racketDown && ballUp <= racketUp ||
			i != ballIdx && comps[i]->left && comps[ballIdx]->offset[0].x + comps[ballIdx]->xVelocity <= -0.94 && ballDown >= racketDown && ballDown <= racketUp)
		{
			float ballCenter = (ballUp + ballDown) / 2;
			float racketCenter = (racketUp + racketDown) / 2;

			if (ballCenter > racketCenter) {
				comps[ballIdx]->yVelocity += 0.002; // +comps[ballIdx]->yVelocity * 0.85;
			}
			else {
				comps[ballIdx]->yVelocity -= 0.002; // +comps[ballIdx]->yVelocity * 0.85;
			}
			comps[ballIdx]->bounce = true;
		}
	}


	context->OMSetRenderTargets(1, &rtv, nullptr);

	// float color[] = { totalTime, 0.1f, 0.1f, 1.0f };
	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(rtv, color);

	for (int i = 0; i < comps.size(); i++) {
		comps[i]->Draw();
	}

	//context->DrawIndexed(pointsCount, 0, 0);
	context->OMSetRenderTargets(0, nullptr, nullptr);
	swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
}

void Game::Run()
{
	PrevTime = std::chrono::steady_clock::now();
	totalTime = 0;
	frameCount = 0;

	MSG msg = {};
	bool isExitRequested = false;
	while (!isExitRequested) {
		// Handle the windows messages.
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		/*if (static_cast<unsigned int>(msg.wParam) == 38) {
			PushRackets(38);
		}
		else if (static_cast<unsigned int>(msg.wParam) == 40) {
			PushRackets(40);
		}
		else if (static_cast<unsigned int>(msg.wParam) == 87) {
			PushRackets(87);
		}
		else if (static_cast<unsigned int>(msg.wParam) == 83) {
			PushRackets(83);
		}*/

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT) {
			isExitRequested = true;
		}

		context->ClearState();

		//context->RSSetState(rastState);

		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(DW32->ClientWidth);
		viewport.Height = static_cast<float>(DW32->ClientHeight);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0f;

		context->RSSetViewports(1, &viewport);

		/*context->IASetInputLayout(layout);
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
		context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
		context->VSSetShader(vertexShader, nullptr, 0);
		context->PSSetShader(pixelShader, nullptr, 0);*/


		auto	curTime = std::chrono::steady_clock::now();
		float	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;

		totalTime += deltaTime;
		frameCount++;

		if (totalTime > 1.0f) {
			float fps = frameCount / totalTime;

			totalTime -= 1.0f;

			WCHAR text[256];
			swprintf_s(text, TEXT("FPS: %f"), fps);
			SetWindowText(DW32->hWnd, text);

			frameCount = 0;
		}

		Draw();
	}
}

void Game::PushRackets(int key)
{
	if (key == 87) {
		for (int i = 0; i < comps.size(); i++) {
			if (!comps[i]->ball && comps[i]->left) {
				for (int j = 0; j < comps[i]->offset.size(); j++) {
					if (comps[i]->offset[j].y < 0.92) {
						comps[i]->offset[j].y = comps[i]->offset[j].y + comps[i]->yVelocity;
					}
				}
			}
		}
	}
	else if (key == 83) {
		for (int i = 0; i < comps.size(); i++) {
			if (!comps[i]->ball && comps[i]->left) {
				for (int j = 0; j < comps[i]->offset.size(); j++) {
					if (comps[i]->offset[j].y > -0.92) {
						comps[i]->offset[j].y = comps[i]->offset[j].y - comps[i]->yVelocity;
					}
				}
			}
		}
	}
	else if (key == 38) {
		for (int i = 0; i < comps.size(); i++) {
			if (!comps[i]->ball && !comps[i]->left) {
				for (int j = 0; j < comps[i]->offset.size(); j++) {
					if (comps[i]->offset[j].y < 0.92) {
						comps[i]->offset[j].y = comps[i]->offset[j].y + comps[i]->yVelocity;
					}
				}
			}
		}
	}
	else if (key == 40) {
		for (int i = 0; i < comps.size(); i++) {
			if (!comps[i]->ball && !comps[i]->left) {
				for (int j = 0; j < comps[i]->offset.size(); j++) {
					if (comps[i]->offset[j].y > -0.92) {
						comps[i]->offset[j].y = comps[i]->offset[j].y - comps[i]->yVelocity;
					}
				}
			}
		}
	}
	
}

void Game::Exit()
{
	PostQuitMessage(0);
}
