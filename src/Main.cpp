#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#define MG_IMPL
#define MG_USE_WINDOWS
#include <Mg.h>
#define STB_IMAGE_IMPLEMENTATION
#include  <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include  "stb_image_write.h"

#define SCR_WIDTH   1024
#define SCR_HEIGHT  768

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

int
main(void)
{
    ///////////////////////////////////////////////////////////////////////////
    // Win32 Setup

    WNDCLASSEX      WindowClass = {0};
    MSG             Message;
    HWND            Window;


    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = &WndProc;
    WindowClass.hInstance = GetModuleHandle(NULL);
    WindowClass.lpszClassName = "Direct3DWindowClass";

    if (!RegisterClassEx(&WindowClass))
    {
        MessageBox(NULL, "Failed to register class!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    Window = CreateWindowEx(NULL, WindowClass.lpszClassName, "Direct3D 11 Window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, SCR_WIDTH, SCR_HEIGHT,
        NULL, NULL, WindowClass.hInstance, NULL);

    if (!Window)
    {
        MessageBox(NULL, "Failed to create window!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    ShowWindow(Window, SW_SHOW);
    UpdateWindow(Window);

    ///////////////////////////////////////////////////////////////////////////
    // D3D11 SETUP

    HRESULT                     Hr;
    ID3D11Device                *Device;
    ID3D11DeviceContext         *Context;
    ID3D11Texture2D             *Backbuffer,
                                *DepthStencilBuffer;
    ID3D11RenderTargetView      *RenderTargetView;
    ID3D11DepthStencilView      *DepthStencilView;
    ID3D11RasterizerState       *RasterState;
    IDXGISwapChain              *SwapChain;
    DXGI_MODE_DESC              BufferDesc;
    DXGI_SWAP_CHAIN_DESC        SwapChainDesc;
    D3D11_TEXTURE2D_DESC        DepthStencilDesc;
    D3D11_VIEWPORT              Viewport;
    D3D11_RASTERIZER_DESC       RasterStateDesc;


    BufferDesc.Width = SCR_WIDTH;
    BufferDesc.Height = SCR_HEIGHT;
    BufferDesc.RefreshRate.Denominator = 1;
    BufferDesc.RefreshRate.Numerator = 60;
    BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    SwapChainDesc.BufferDesc = BufferDesc;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SampleDesc.Quality = 0;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 1;
    SwapChainDesc.OutputWindow = Window;
    SwapChainDesc.Windowed = TRUE;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    SwapChainDesc.Flags = 0;

    DepthStencilDesc.Width = SCR_WIDTH;
    DepthStencilDesc.Height = SCR_HEIGHT;
    DepthStencilDesc.MipLevels = 1;
    DepthStencilDesc.ArraySize = 1;
    DepthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthStencilDesc.SampleDesc.Count = 1;
    DepthStencilDesc.SampleDesc.Quality = 0;
    DepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DepthStencilDesc.CPUAccessFlags = 0;
    DepthStencilDesc.MiscFlags = 0;

    Hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
            D3D11_CREATE_DEVICE_DEBUG, 0, 0, D3D11_SDK_VERSION, &SwapChainDesc,
            &SwapChain, &Device,NULL, &Context);
    Hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&Backbuffer);
    Hr = Device->CreateRenderTargetView(Backbuffer, NULL, &RenderTargetView);
    Hr = Device->CreateTexture2D(&DepthStencilDesc, 0, &DepthStencilBuffer);
    Hr = Device->CreateDepthStencilView(DepthStencilBuffer, NULL, &DepthStencilView);
    Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);

    Viewport.TopLeftX = 0;
    Viewport.TopLeftY = 0;
    Viewport.Width = SCR_WIDTH;
    Viewport.Height = SCR_HEIGHT;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    Context->RSSetViewports(1, &Viewport);

    ZeroMemory(&RasterStateDesc, sizeof(RasterStateDesc));
    RasterStateDesc.FillMode = D3D11_FILL_SOLID;
    RasterStateDesc.CullMode = D3D11_CULL_NONE;
    RasterStateDesc.DepthClipEnable = TRUE;
    Hr = Device->CreateRasterizerState(&RasterStateDesc, &RasterState);
    Context->RSSetState(RasterState);

    ///////////////////////////////////////////////////////////////////////////
    // Shader Setup

    ID3D11VertexShader      *VS;
    ID3D11PixelShader       *PS;
	ID3D11ComputeShader		*CS;
    ID3DBlob                *VSBlob,
                            *PSBlob,
							*CSBlob;


    Hr = D3DReadFileToBlob(L"Vertex.hlsl.cso", &VSBlob);
    Hr = D3DReadFileToBlob(L"Pixel.hlsl.cso", &PSBlob);
    Hr = D3DReadFileToBlob(L"Compute.hlsl.cso", &CSBlob);
    Hr = Device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), NULL, &VS);
    Hr = Device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), NULL, &PS);
    Hr = Device->CreateComputeShader(CSBlob->GetBufferPointer(), CSBlob->GetBufferSize(), NULL, &CS);
    Context->VSSetShader(VS, 0, 0);
    Context->PSSetShader(PS, 0, 0);
    Context->CSSetShader(CS, 0, 0);

    ///////////////////////////////////////////////////////////////////////////
    // Buffer Setup

    ID3D11Buffer               	*VertexBuffer,
								*IndexBuffer,
								*TexCoordBuffer;
    D3D11_BUFFER_DESC           VertexBufferDesc = {0},
								IndexBufferDesc = {0},
								TexCoordBufferDesc = {0};
    D3D11_SUBRESOURCE_DATA      VertexBufferData = {0},
								IndexBufferData = {0},
								TexCoordBufferData = {0};
    UINT                        Stride,
                                Offset;
    FLOAT                       Vertices[] =
    {
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
    };
	DWORD						Indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};
	FLOAT						TexCoords[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};


    VertexBufferDesc.ByteWidth = sizeof(Vertices);
    VertexBufferDesc.BindFlags =  D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    VertexBufferData.pSysMem = Vertices;

    IndexBufferDesc.ByteWidth = sizeof(Indices);
    IndexBufferDesc.BindFlags =  D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    IndexBufferData.pSysMem = Indices;

    TexCoordBufferDesc.ByteWidth = sizeof(TexCoords);
    TexCoordBufferDesc.BindFlags =  D3D11_BIND_VERTEX_BUFFER;
	TexCoordBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    TexCoordBufferData.pSysMem = TexCoords;

    Hr = Device->CreateBuffer(&VertexBufferDesc, &VertexBufferData, &VertexBuffer);
    Hr = Device->CreateBuffer(&IndexBufferDesc, &IndexBufferData, &IndexBuffer);
    Hr = Device->CreateBuffer(&TexCoordBufferDesc, &TexCoordBufferData, &TexCoordBuffer);

    Stride = 3 * sizeof(FLOAT);
    Offset = 0;
    Context->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
    Stride = 2 * sizeof(FLOAT);
    Context->IASetVertexBuffers(1, 1, &TexCoordBuffer, &Stride, &Offset);

	Context->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    ///////////////////////////////////////////////////////////////////////////
    // Input Layout Setup

    ID3D11InputLayout               *Layout;
    D3D11_INPUT_ELEMENT_DESC        LayoutDesc[2] = {0};


    LayoutDesc[0].SemanticName = "POSITION";
    LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    LayoutDesc[0].InputSlot = 0;
    LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    LayoutDesc[1].SemanticName = "TEXCOORD";
    LayoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    LayoutDesc[1].InputSlot = 1;
    LayoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    Hr = Device->CreateInputLayout(LayoutDesc, 2, VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &Layout);
    Context->IASetInputLayout(Layout);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	///////////////////////////////////////////////////////////////////////////
	// Texture Setup

	ID3D11Texture2D						*Texture,
										*Copy,
										*Stage;
	D3D11_TEXTURE2D_DESC				TextureDesc = {0},
										StagingDesc = {0};
	D3D11_SUBRESOURCE_DATA				TextureSubData = {0};
	BYTE								*TextureData;
	INT									Width, Height, Nr;
	D3D11_SHADER_RESOURCE_VIEW_DESC		TextureSRVDesc;
	D3D11_UNORDERED_ACCESS_VIEW_DESC	CopyUAVDesc;
	ID3D11ShaderResourceView			*TextureSRV,
										*CopySRV,
										*NullSRV[] = {NULL};
	ID3D11UnorderedAccessView			*TextureUAV,
										*CopyUAV,
										*NullUAV[] = {NULL};
	ID3D11SamplerState					*TextureSampler;
	D3D11_SAMPLER_DESC					TextureSamplerDesc;


	TextureData = stbi_load("danteh.png", &Width, &Height, &Nr, 4);
	TextureDesc.Width = Width;
	TextureDesc.Height = Height;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	TextureSubData.pSysMem = TextureData;
	TextureSubData.SysMemPitch = Width * 4;

	StagingDesc.Width = Width;
	StagingDesc.Height = Height;
	StagingDesc.MipLevels = 1;
	StagingDesc.ArraySize = 1;
	StagingDesc.SampleDesc.Count = 1;
	StagingDesc.SampleDesc.Quality = 0;
	StagingDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	StagingDesc.Usage = D3D11_USAGE_STAGING;
	StagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

	Hr = Device->CreateTexture2D(&TextureDesc, &TextureSubData, &Texture);
	Hr = Device->CreateTexture2D(&TextureDesc, NULL, &Copy);
	Hr = Device->CreateTexture2D(&StagingDesc, NULL, &Stage);

	TextureSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	TextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	TextureSRVDesc.Texture2D.MipLevels = 1;
	TextureSRVDesc.Texture2D.MostDetailedMip = 0;

	ZeroMemory(&TextureSamplerDesc, sizeof(TextureSamplerDesc));
	TextureSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	TextureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	TextureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	TextureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	TextureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	TextureSamplerDesc.MinLOD = 0;
	TextureSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	CopyUAVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CopyUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	CopyUAVDesc.Texture2D.MipSlice = 0;

	Hr = Device->CreateShaderResourceView(Texture, &TextureSRVDesc, &TextureSRV);
	Hr = Device->CreateShaderResourceView(Copy, &TextureSRVDesc, &CopySRV);
	Hr = Device->CreateSamplerState(&TextureSamplerDesc, &TextureSampler);
	Hr = Device->CreateUnorderedAccessView(Texture, &CopyUAVDesc, &TextureUAV);
	Hr = Device->CreateUnorderedAccessView(Copy, &CopyUAVDesc, &CopyUAV);

	Context->CSSetShaderResources(0, 1, &TextureSRV);
	Context->CSSetUnorderedAccessViews(0, 1, &CopyUAV, NULL);
	Context->Dispatch(Width, Height, 1);
	Context->CSSetUnorderedAccessViews(0, 1, NullUAV, NULL);
	Context->CSSetShaderResources(0, 1, NullSRV);

	Context->PSSetShaderResources(0, 1, &CopySRV);
	Context->PSSetSamplers(0, 1, &TextureSampler);

    ///////////////////////////////////////////////////////////////////////////
    // Copymapping

	D3D11_MAPPED_SUBRESOURCE		Mapped = {0};
	FLOAT							*Buffer;


	Context->CopyResource(Stage, Copy);

	Context->Map(Stage, 0, D3D11_MAP_READ, 0, &Mapped);
		Buffer = (FLOAT *)malloc(Mapped.RowPitch * Height * sizeof(FLOAT));
		memcpy(Buffer, Mapped.pData, (Mapped.RowPitch * Height));
	Context->Unmap(Stage, 0);

	stbi_write_bmp("foo.bmp", Mapped.RowPitch / 4, Height, 4, Buffer);

	free(Buffer);

    ///////////////////////////////////////////////////////////////////////////
    // Main Loop

    while (1)
    {
        if (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            if (Message.message == WM_QUIT)
                break;
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else
        {
            static FLOAT BgColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};

            Context->ClearRenderTargetView(RenderTargetView, BgColor);
            Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            Context->DrawIndexed(6, 0, 0);

            SwapChain->Present(0, 0);
        }
    }

    return 0;
}

LRESULT CALLBACK
WndProc(HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam)
{
    LRESULT         Result = 0;


    switch (Msg)
    {
        case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
        } break;
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            DestroyWindow(hWnd);
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            DestroyWindow(hWnd);
        } break;

        default:
        {
            Result = DefWindowProc(hWnd, Msg, wParam, lParam);
        } break;
    }

    return (Result);
}

