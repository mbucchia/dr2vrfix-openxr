// MIT License
//
// Copyright(c) 2022 Matthieu Bucchianeri
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pch.h"

namespace {
	wil::unique_hmodule g_d3d11Dll;
	wil::unique_hmodule g_dr2vrfixDll;
	decltype(D3D11CreateDevice)* g_dr2vrfixD3D11CreateDeviceProc = nullptr;
	decltype(D3D11CreateDeviceAndSwapChain)* g_realD3D11CreateDeviceAndSwapChainProc = nullptr;
	decltype(D3D11On12CreateDevice)* g_realD3D11On12CreateDeviceProc = nullptr;
	decltype(CreateDirect3D11SurfaceFromDXGISurface)* g_realCreateDirect3D11SurfaceFromDXGISurfaceProc = nullptr;
	decltype(CreateDirect3D11DeviceFromDXGIDevice)* g_realCreateDirect3D11DeviceFromDXGIDeviceProc = nullptr;

	HRESULT LoadSystemDLL(_Out_ HMODULE* pModule, _In_ LPCWSTR dllName) {
		wchar_t fullPath[MAX_PATH];

		if (0 == GetSystemDirectory(fullPath, _countof(fullPath))) {
			return HRESULT_FROM_WIN32(GetLastError());
		}

		if (wcscat_s(fullPath, MAX_PATH, L"\\") || wcscat_s(fullPath, MAX_PATH, dllName)) {
			return E_UNEXPECTED;
		}

		*pModule = LoadLibrary(fullPath);
		return *pModule ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//
		// Resolve all the symbols.
		//

		*g_dr2vrfixDll.put() = LoadLibrary(L"dr2vrfix.dll");

		g_dr2vrfixD3D11CreateDeviceProc = reinterpret_cast<decltype(g_dr2vrfixD3D11CreateDeviceProc)>(
			GetProcAddress(g_dr2vrfixDll.get(), "D3D11CreateDevice"));

		LoadSystemDLL(g_d3d11Dll.put(), L"d3d11.dll");

		g_realD3D11CreateDeviceAndSwapChainProc = reinterpret_cast<decltype(g_realD3D11CreateDeviceAndSwapChainProc)>(
			GetProcAddress(g_d3d11Dll.get(), "D3D11CreateDeviceAndSwapChain"));
		g_realD3D11On12CreateDeviceProc = reinterpret_cast<decltype(g_realD3D11On12CreateDeviceProc)>(
			GetProcAddress(g_d3d11Dll.get(), "D3D11On12CreateDevice"));
		g_realCreateDirect3D11SurfaceFromDXGISurfaceProc = reinterpret_cast<decltype(g_realCreateDirect3D11SurfaceFromDXGISurfaceProc)>(
			GetProcAddress(g_d3d11Dll.get(), "CreateDirect3D11SurfaceFromDXGISurface"));
		g_realCreateDirect3D11DeviceFromDXGIDeviceProc = reinterpret_cast<decltype(g_realCreateDirect3D11DeviceFromDXGIDeviceProc)>(
			GetProcAddress(g_d3d11Dll.get(), "CreateDirect3D11DeviceFromDXGIDevice"));

		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//
// Forward to dr2vrfix.dll
//

HRESULT WINAPI D3D11CreateDevice(
	IDXGIAdapter* pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	ID3D11Device** ppDevice,
	D3D_FEATURE_LEVEL* pFeatureLevel,
	ID3D11DeviceContext** ppImmediateContext) {

	// OpenComposite might pass null but dr2vrfix expects non-null.
	ComPtr<ID3D11Device> dummy1;
	ComPtr<ID3D11DeviceContext> dummy2;

	if (!ppDevice) {
		ppDevice = dummy1.GetAddressOf();
	}
	if (!ppImmediateContext) {
		ppImmediateContext = dummy2.GetAddressOf();
	}

	return g_dr2vrfixD3D11CreateDeviceProc(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}

//
// Passthrough to real d3d11.dll
//

HRESULT WINAPI D3D11CreateDeviceAndSwapChain(
	IDXGIAdapter* pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
	IDXGISwapChain** ppSwapChain,
	ID3D11Device** ppDevice,
	D3D_FEATURE_LEVEL* pFeatureLevel,
	ID3D11DeviceContext** ppImmediateContext) {
	return g_realD3D11CreateDeviceAndSwapChainProc(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
}


HRESULT WINAPI D3D11On12CreateDevice(
	IUnknown* pDevice,
	UINT Flags,
	CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	IUnknown* CONST* ppCommandQueues,
	UINT NumQueues,
	UINT NodeMask,
	ID3D11Device** ppDevice,
	ID3D11DeviceContext** ppImmediateContext,
	D3D_FEATURE_LEVEL* pChosenFeatureLevel) {
	return g_realD3D11On12CreateDeviceProc(pDevice, Flags, pFeatureLevels, FeatureLevels, ppCommandQueues, NumQueues, NodeMask, ppDevice, ppImmediateContext, pChosenFeatureLevel);
}

HRESULT WINAPI CreateDirect3D11SurfaceFromDXGISurface(
	IDXGISurface* dxgiSurface,
	IInspectable** graphicsSurface) {
	return g_realCreateDirect3D11SurfaceFromDXGISurfaceProc(dxgiSurface, graphicsSurface);
}

HRESULT WINAPI CreateDirect3D11DeviceFromDXGIDevice(
	IDXGIDevice* dxgiDevice,
	IInspectable** graphicsDevice) {
	return g_realCreateDirect3D11DeviceFromDXGIDeviceProc(dxgiDevice, graphicsDevice);
}
