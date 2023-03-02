#pragma once
#include <iostream>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <vector>
class Game;

class Component
{

public: 
	Game* game;

	ID3DBlob* vertexBC;
	ID3DBlob* errorVertexCode;

	ID3DBlob* pixelBC;
	ID3DBlob* errorPixelCode;

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;

	ID3D11InputLayout* layout;

	ID3D11Buffer* vb;
	ID3D11Buffer* ib;
	ID3D11Buffer* cb;

	ID3D11RasterizerState* rastState;

	std::vector<DirectX::XMFLOAT4> points = {
		DirectX::XMFLOAT4(-0.02f, -0.02f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, .5f),
		DirectX::XMFLOAT4(-0.02f, 0.02f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, .5f),
		DirectX::XMFLOAT4(0.02f, 0.02f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0.02f, -0.02f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
	};

	//std::vector<int> indeces = { 0,1,2, 1,0,3 };
	std::vector<int> indeces = { 0,1,2, 2,0,3 };

	UINT pointsCount;
	std::vector<DirectX::XMFLOAT4> offset;

	bool ball = true;
	float xVelocity = 0.007;
	float yVelocity = 0.004;
	//float yVelocity = 0.0;
	bool bounce = false;

	bool left = true;

	//int Initialize(Game* newGame, LPCWSTR shadersLoc, std::vector<DirectX::XMFLOAT4>& points, std::vector<int>& indeces);
	int Initialize(Game* newGame, LPCWSTR shadersLoc, bool b = true, bool l = true);

	void Draw();

};

