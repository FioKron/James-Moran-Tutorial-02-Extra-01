#include <windows.h>
#include <Windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>

/** To use instead of an array of characters */
#include <string>

/** To assist in varying the screen-clear colour in Extra Exercise 01a */
#include <random>

//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables (seems as though g_ would equate to a global variable then
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;

/** For Exercise 02 of Tutorial 01 */
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pD3DDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;

/** For exercise 01 of Tutorial 02 */
ID3D11RenderTargetView* g_pBackBufferRTView = NULL;

/** For the Additional Exercises of Tutorial 02 */

// Constant values:

const int G_CLEAR_COLOUR_ARRAY_SIZE = 4;

/** For clearing the back buffer (leave the alpha value at 1.0f) */
float  g_clear_colour[G_CLEAR_COLOUR_ARRAY_SIZE] = { 0.0f, 0.0f, 0.0f, 1.0f };

// Rename for each tutorial
//char		g_TutorialName[100] = "James Moran Tutorial 02 Exercise 01\0";

// (Attempting to use std::string instead):
std::string TutorialName = "James Moran Tutorial 02 Exercise 01\0";

//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/** For Exercise 02 of Tutorial 01 */
HRESULT InitialiseD3D();
void ShutdownD3D();

/** For Exercise 01 of Tutorial 02 */
void RenderFrame(void); // (enquire about the use of void here, to identify no parameters)

						//////////////////////////////////////////////////////////////////////////////////////
						// Entry point to the program. Initializes everything and goes into a message processing 
						// loop. Idle time is used to render the scene.
						//////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}

	if (FAILED(InitialiseD3D()))
	{
		DXTRACE_MSG("Failed to create Device");
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			RenderFrame();
		}
	}

	// For closing Direct3D correctly:
	ShutdownD3D();
	return (int)msg.wParam;
}


//////////////////////////////////////////////////////////////////////////////////////
// Register class and create window
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Give your app window your own name
	char Name[100] = "James Moran\0";

	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	//   wcex.hbrBackground = (HBRUSH )( COLOR_WINDOW + 1); // Needed for non-D3D apps
	wcex.lpszClassName = Name;

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// As const char* can be parsed in as an LPCCHAR: 
	g_hWnd = CreateWindow(Name, TutorialName.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
		rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Called every time the application receives a message
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	// For Extra Exercise 01b:
	float MouseXPosition = 0.0f;
	float MouseYPosition = 0.0f;
	float MouseWheelRotationDelta = 0.0f;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			DestroyWindow(g_hWnd);
		return 0;

		// For altering the red...
	case WM_MOUSEMOVE:
		MouseXPosition = LOWORD(lParam);
		MouseYPosition = HIWORD(lParam);

		g_clear_colour[0] = (MouseXPosition / MouseYPosition) / 100.0f;
		break;
		// green...
	case WM_LBUTTONDOWN:
		g_clear_colour[1] += 0.010f;
		break;

	case WM_RBUTTONDOWN:
		g_clear_colour[1] -= 0.010f;
		break;

		// as well as blue components of g_clear_colour.
	case WM_MOUSEWHEEL:

		MouseWheelRotationDelta = GET_WHEEL_DELTA_WPARAM(wParam);

		// Modify the blue component accordingly: 
		g_clear_colour[2] += MouseWheelRotationDelta / 1000.0f;

		// Keep it within the bounds:
		if (g_clear_colour[2] >= 1.0f)
		{
			g_clear_colour[2] = 1.0f;
		}
		else if (g_clear_colour[2] <= 0.0f)
		{
			g_clear_colour[2] = 0.0f;
		}
		break;

	default:
		// A break-point was thrown on the return line, if hWnd is not valid:
		// BREAKPOINT STILL THROWN AT TIMES GESU(GES[HJAD2!
		if (hWnd)
		{
			// Add or replace the above check with that of checking this value: hWnd->unused;
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Create D3D device and swap chain
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, // comment out this line if you need to test D3D 11.0 functionality on hardware that doesn't support it
		D3D_DRIVER_TYPE_WARP, // comment this out also to use reference device
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain,
			&g_pD3DDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	// Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBufferTexture);

	if (FAILED(hr)) return hr;

	// Use the back buffer texture pointer to create the render target view
	hr = g_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL,
		&g_pBackBufferRTView);
	pBackBufferTexture->Release();

	if (FAILED(hr)) return hr;

	// Set the render target view
	g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, NULL);

	// Set the viewport
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	g_pImmediateContext->RSSetViewports(1, &viewport);


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if (g_pBackBufferRTView) g_pBackBufferRTView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pD3DDevice) g_pD3DDevice->Release();
}

// Render frame
void RenderFrame(void)
{
	// Vary the clear colour value:
	//std::default_random_engine DefaultRandomGenerator(std::random_device{}());
	// (By default, such a distribtion has a range from 0.0f to 1.0f, but I wanted
	// to make this clear):
	//std::uniform_real_distribution<float> RandomDistribution(0.0f, 1.0f); 

	//for (int ColourComponentIterator = 0; ColourComponentIterator < G_CLEAR_COLOUR_ARRAY_SIZE - 1; ColourComponentIterator++)
	//{
	//g_clear_colour[ColourComponentIterator] = RandomDistribution(DefaultRandomGenerator);
	//}

	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, g_clear_colour);

	// RENDER HERE

	// Display what has just been rendered
	g_pSwapChain->Present(0, 0);
}