﻿//plane.cpp
// DirectX11 : https://www.microsoft.com/en-us/download/details.aspx?id=6812

// Include DirectX
#include "Win32_DirectXAppUtil.h"

// Need DirectX install
#include "D3DX11async.h"

// Include the Oculus SDK
//#include "OVR_CAPI_D3D.h"

#pragma comment(lib,"d3dx11.lib")

//Shader
LPCSTR g_planeShader =
	"Texture2D g_texDecal : register(t0);\n"
	"SamplerState g_samLinear : register(s0);\n"
	"\n"
	"   struct VS_OUTPUT\n"
	"   {\n"
	"      float4 Pos : SV_POSITION;\n"
	"      float2 Tex : TEXCOORD;\n"
	"   };\n"
	"\n"
	"   VS_OUTPUT VSFunc(float4 Pos : POSITION, float2 Tex : TEXCOORD)\n"
	"   {\n"
	"      VS_OUTPUT output = (VS_OUTPUT)0;\n"
	"      output.Pos = Pos;\n"
	"      output.Tex = Tex;\n"
	"      return output;\n"
	"   }\n"
	"\n"
	"   float4 PSFunc(VS_OUTPUT input) : SV_Target\n"
	"   {\n"
	"      return g_texDecal.Sample(g_samLinear, input.Tex);\n"
	"	}\n"
;

//Vector class
class VECTOR2
{
public:
	VECTOR2(float, float);
	float u;
	float v;
};
VECTOR2::VECTOR2(float a, float b)
{
	u = a; v = b;
}
class VECTOR3
{
public:
	VECTOR3(float, float, float);
	float x;
	float y;
	float z;
};
VECTOR3::VECTOR3(float a, float b, float c)
{
	x = a; y = b; z = c;
}

//Vertex struct
struct SimpleVertex
{
	VECTOR3 Pos;
	VECTOR2 Uv;
};

static ID3D11InputLayout* VertexLayout = NULL;
static ID3D11Buffer* VertexBuffer = NULL;
static ID3D11VertexShader* VertexShader = NULL;
static ID3D11PixelShader* PixelShader = NULL;
static ID3D11SamplerState* SampleLinear = NULL;
static ID3D11Texture2D* Texture = NULL;
static ID3D11ShaderResourceView* ShaderRC = NULL;

int InitializeCamPlane(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, int w, int h, float zsize)
{
	//初期化
	ID3DBlob *pCompiledShader = NULL;
	ID3DBlob *pErrors = NULL;

	//頂点シェーダー作成
	if (FAILED(D3DX11CompileFromMemory(g_planeShader, strlen(g_planeShader), NULL, NULL, NULL, "VSFunc", "vs_4_0", 0, 0, NULL, &pCompiledShader, &pErrors, NULL)))
	{
		MessageBox(0, L"Vertex Shader Read Error!", NULL, MB_OK);
		return E_FAIL;
	}
	Release(pErrors);

	if (FAILED(Device->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &VertexShader)))
	{
		Release(pCompiledShader);
		MessageBox(0, L"Vertex Shader Create Error!", NULL, MB_OK);
		return E_FAIL;
	}
	//頂点インプットレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//頂点インプットレイアウトを作成
	if (FAILED(Device->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &VertexLayout)))
		return FALSE;
	//頂点インプットレイアウトをセット
	DeviceContext->IASetInputLayout(VertexLayout);

	//ピクセルシェーダー作成
	if (FAILED(D3DX11CompileFromMemory(g_planeShader, strlen(g_planeShader), NULL, NULL, NULL, "PSFunc", "ps_4_0", 0, 0, NULL, &pCompiledShader, &pErrors, NULL)))
	{
		MessageBox(0, L"Pixel Shader Read Error!", NULL, MB_OK);
		return E_FAIL;
	}
	Release(pErrors);
	if (FAILED(Device->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &PixelShader)))
	{
		Release(pCompiledShader);
		MessageBox(0, L"Pixel Shader Create Error!", NULL, MB_OK);
		return E_FAIL;
	}
	Release(pCompiledShader);

	float aspect = (float)h / (float)w * 0.82f;

	//四角形ポリゴン
	SimpleVertex vertices[] =
	{
		VECTOR3(-zsize, -zsize*aspect, 0.5f), VECTOR2(0, 1),
		VECTOR3(-zsize, zsize*aspect, 0.5f), VECTOR2(0, 0),
		VECTOR3(zsize, -zsize*aspect, 0.5f), VECTOR2(1, 1),
		VECTOR3(zsize, zsize*aspect, 0.5f), VECTOR2(1, 0),
	};
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;

	if (FAILED(Device->CreateBuffer(&bd, &InitData, &VertexBuffer)))
		return FALSE;

	//バーテックスバッファーをセット
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

	//プリミティブ・トポロジーをセット
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャー用サンプラー作成
	D3D11_SAMPLER_DESC SamDesc;
	ZeroMemory(&SamDesc, sizeof(D3D11_SAMPLER_DESC));

	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	Device->CreateSamplerState(&SamDesc, &SampleLinear);

	//テクスチャー作成
	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(texDesc));

	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.Width = w;
	texDesc.Height = h;
	texDesc.CPUAccessFlags = 0;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	if (FAILED(Device->CreateTexture2D(&texDesc, NULL, &Texture)))
	{
		MessageBox(0, L"テクスチャバッファ作成に失敗", NULL, MB_OK);
		return E_FAIL;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	memset(&srvDesc, 0, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	if (FAILED(Device->CreateShaderResourceView(Texture, &srvDesc, &ShaderRC))){
		MessageBox(0, L"リソースバッファ作成に失敗", NULL, MB_OK);
		return E_FAIL;
	}

	return S_OK;
}

int CleanCamPlane()
{
	Release(SampleLinear);
	Release(Texture);
	Release(ShaderRC);
	Release(VertexShader);
	Release(PixelShader);
	Release(VertexBuffer);
	Release(VertexLayout);

	return S_OK;
}

int SetCamImage(ID3D11DeviceContext* DeviceContext, unsigned char* camImage, unsigned int imageRowsize)
{
	if (Texture == NULL)
		return S_FALSE;

	D3D11_TEXTURE2D_DESC desc;
	Texture->GetDesc(&desc);
	DeviceContext->UpdateSubresource(Texture, 0, NULL, camImage, imageRowsize, 0);

	return S_OK;
}

int RendererCamPlane(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext)
{
	if (Texture == NULL)
		return S_FALSE;

	//使用するシェーダーの登録
	DeviceContext->VSSetShader(VertexShader, NULL, 0);
	DeviceContext->PSSetShader(PixelShader, NULL, 0);

	//テクスチャーをシェーダーに渡す
	DeviceContext->PSSetSamplers(0, 1, &SampleLinear);
	DeviceContext->PSSetShaderResources(0, 1, &ShaderRC);

	//プリミティブをレンダリング
	DeviceContext->Draw(4, 0);

	return S_OK;
}