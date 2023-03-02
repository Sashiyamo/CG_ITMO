#include "Component.h"
#include "Game.h"
#include <cmath>
#include "InputDevice.h"

int Component::Initialize(Game* newGame, LPCWSTR shadersLoc, bool b, bool l)
{
	if (!b) {
		if (l) {
			points = {
				DirectX::XMFLOAT4(-1.0f, -0.08f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, .5f),
				DirectX::XMFLOAT4(-1.0f, 0.08f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, .5f),
				DirectX::XMFLOAT4(-0.96f, 0.08f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				DirectX::XMFLOAT4(-0.96f, -0.08f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
			};
		}
		else {
			left = false;

			points = {
				DirectX::XMFLOAT4(0.96f, -0.08f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, .5f),
				DirectX::XMFLOAT4(0.96f, 0.08f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, .5f),
				DirectX::XMFLOAT4(1.0f, 0.08f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				DirectX::XMFLOAT4(1.0f, -0.08f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
			};
		}
		
		ball = false;
		xVelocity = 0.0;
		yVelocity = 0.008;
	}
	else {
		std::vector<DirectX::XMFLOAT4> c_points;
		std::vector<int> c_indeces;

		float radius = 0.015;
		int tesselation = 48;
		int last = 0;

		for (int i = 0; i < tesselation; i++)
		{
			float angle = float(i) / float(tesselation)	* 3.14159265358979323846 * 2;
			c_points.push_back(DirectX::XMFLOAT4(radius * cos(angle), radius * sin(angle), 0.0f, 1.0f));
			c_points.push_back(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			if (i != 0) {
				if (i == 47) {
					c_indeces.push_back(0);
					c_indeces.push_back(i);
					c_indeces.push_back(1);
				}
				else {
					c_indeces.push_back(0);
					c_indeces.push_back(i);
					c_indeces.push_back(i + 1);
				}
			}
		}

		indeces = c_indeces;
		points = c_points;
	}


	game = newGame;

	game->res = D3DCompileFromFile(shadersLoc,
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexBC,
		&errorVertexCode);

	if (FAILED(game->res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorVertexCode) {
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			//MessageBox(game->DW32->hWnd, shadersLoc, L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	//D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(1.0f, 1.0f, 1.0f, 1.0f)", nullptr, nullptr };

	game->res = D3DCompileFromFile(shadersLoc, nullptr /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelBC, &errorPixelCode);

	game->device->CreateVertexShader(
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		nullptr, &vertexShader);

	game->device->CreatePixelShader(
		pixelBC->GetBufferPointer(),
		pixelBC->GetBufferSize(),
		nullptr, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	game->device->CreateInputLayout(
		inputElements,
		2,
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		&layout);



	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * points.size();

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	game->device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);



	pointsCount = indeces.size();

	//int indeces[] = { 0,1,2, 1,0,3 };
	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * indeces.size();

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indeces.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	game->device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	UINT strides[] = { 32 };
	UINT offsets[] = { 0 };



	//std::vector<DirectX::XMFLOAT4> cPoints;
	for (int i = 0; i < pointsCount / 2; i++)
	{
		offset.push_back(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
		//cPoints.push_back(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	}

	D3D11_BUFFER_DESC constBufDesc = {};
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = 0;
	constBufDesc.MiscFlags = 0;
	constBufDesc.StructureByteStride = 0;
	constBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * offset.size();

	D3D11_SUBRESOURCE_DATA constData = {};
	constData.pSysMem = offset.data();
	constData.SysMemPitch = 0;
	constData.SysMemSlicePitch = 0;

	game->device->CreateBuffer(&constBufDesc, &constData, &cb);


	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	game->res = game->device->CreateRasterizerState(&rastDesc, &rastState);
}

void Component::Draw()
{
	bool bounceX = bounce;
	bool bounceY = false;
	bool loose = false;
	for (int i = 0; i < offset.size(); i++)
	{
		//std::cout << offset[i].x;
		/*offset[i].x = offset[i].x + xVelocity;
		offset[i].y = offset[i].y + yVelocity;*/

		if (ball) {
			offset[i].x = offset[i].x + xVelocity;
			offset[i].y = offset[i].y + yVelocity;

			if (offset[i].x >= 0.98) {
				if (game->dead == 0) {
					loose = true;
					game->scoreL++;
					std::cout << "Left: " << game->scoreL << "  ###  " << "Right: " << game->scoreR << std::endl;
				}
				game->dead++;
			}
			if (offset[i].x <= -0.98) {
				if (game->dead == 0) {
					loose = true;
					game->scoreR++;
					std::cout << "Left: " << game->scoreL << "  ###  " << "Right: " << game->scoreR << std::endl;
				}
				game->dead++;
			}
			if (offset[i].y >= 0.98 || offset[i].y <= -0.98) {
				bounceY = true;
			}
		} 
		else {
			if (game->inputDevice->IsKeyDown(Keys::Up) && !left && offset[i].y < 0.92) {
				offset[i].y = offset[i].y + yVelocity;
			}
			else if (game->inputDevice->IsKeyDown(Keys::Down) && !left && offset[i].y > -0.92) {
				offset[i].y = offset[i].y - yVelocity;
			}
			else if (game->inputDevice->IsKeyDown(Keys::W) && left && offset[i].y < 0.92) {
				offset[i].y = offset[i].y + yVelocity;
			}
			else if (game->inputDevice->IsKeyDown(Keys::S) && left && offset[i].y > -0.92) {
				offset[i].y = offset[i].y - yVelocity;
			}
			else if (game->inputDevice->IsKeyDown(Keys::Escape)) {
				PostQuitMessage(0);
			}
		}
		//else if (!ball && offset[i].y >= 0.96 || offset[i].y <= -0.96) yVelocity = -yVelocity;
	}

	if (ball) {
		if (bounceX) {
			xVelocity = -(xVelocity * 1.05);
		}
		else if (bounceY) {
			yVelocity = -(yVelocity * 1.05);
		}
		else if (loose) {
			for (int i = 0; i < offset.size(); i++)
			{
				offset[i] = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
				if (xVelocity >= 0) xVelocity = -0.007;
				else  xVelocity = 0.007;
				
				yVelocity = 0.004;

				/*game->scoreL = 0;
				game->scoreR = 0;*/

				if (game->dead >= 2) {
					game->dead = 0;
				}
			}
		}
	}


	game->context->UpdateSubresource(cb, 0, nullptr, offset.data(), 0, 0);

	UINT strides[] = { 32 };
	UINT offsets[] = { 0 };

	game->context->RSSetState(rastState);

	game->context->IASetInputLayout(layout);
	game->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game->context->VSSetConstantBuffers(0, 1, &cb);

	game->context->VSSetShader(vertexShader, nullptr, 0);
	game->context->PSSetShader(pixelShader, nullptr, 0);

	game->context->DrawIndexed(pointsCount, 0, 0);
}
