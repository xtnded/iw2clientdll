#include "stdheader.h"
#include "memutil.h"
#include "storage.h"
#include "helpers.h"

// DirectX
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#pragma comment(lib, "d3d9.lib")
IDirect3DDevice9 *device = NULL;

template<typename func, typename vttype> void changeVTEx(void** vt, vttype n, func target)
{
	DWORD OldProtections = 0;
	VirtualProtect(&vt[n], sizeof(LPCVOID), PAGE_EXECUTE_READWRITE, &OldProtections);
	vt[n] = (void*)target;
	VirtualProtect(&vt[n], sizeof(LPCVOID), OldProtections, &OldProtections);
}

// preprocessor tools
#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)

#define getVT(ptr) (*(void***)ptr)
#define changeVT(n, target) changeVTEx(getVT(This), n, target)

#define hookVT(ptr, INTERFACE, METHOD) \
{\
    PPCAT(PPCAT(PPCAT(fo, INTERFACE),_), METHOD) = (PPCAT(PPCAT(PPCAT(f, INTERFACE), _), METHOD))(getVT(ptr)[PPCAT(PPCAT(PPCAT(vt, INTERFACE),_), METHOD)]);\
    changeVTEx(getVT(ptr), PPCAT(PPCAT(PPCAT(vt, INTERFACE),_), METHOD), PPCAT(PPCAT(PPCAT(fh, INTERFACE),_), METHOD));\
}

#define GEN_HOOK_(TYPE, ...) \
    PPCAT(PPCAT(PPCAT(f, INTERFACE), _), METHOD) PPCAT(PPCAT(PPCAT(fo, INTERFACE),_), METHOD) = NULL;\
    TYPE STDMETHODCALLTYPE PPCAT(PPCAT(PPCAT(fh, INTERFACE),_), METHOD)(__VA_ARGS__)

#define GEN_HOOK(...) \
    GEN_HOOK_(HRESULT, __VA_ARGS__)

#define UNHOOK \
    changeVT(PPCAT(PPCAT(PPCAT(vt, INTERFACE),_), METHOD), PPCAT(PPCAT(PPCAT(fo, INTERFACE),_), METHOD))

#define REHOOK \
    changeVT(PPCAT(PPCAT(PPCAT(vt, INTERFACE),_), METHOD), PPCAT(PPCAT(PPCAT(fh, INTERFACE),_), METHOD))

#define CALL_ORGINAL_(RET, ...) \
    UNHOOK;\
    RET = PPCAT(This->, METHOD)(__VA_ARGS__);\
    REHOOK;

#define CALL_ORGINAL(...) \
    CALL_ORGINAL_(HRESULT hr, __VA_ARGS__)

// Create function pointer types
#undef INTERFACE
#define INTERFACE IDirect3DDevice9
#undef PURE
#define PURE
#undef THIS_
#define THIS_                   INTERFACE FAR* This,
#undef THIS
#define THIS                    INTERFACE FAR* This
#undef STDMETHOD
#define STDMETHOD(method)       typedef HRESULT (STDMETHODCALLTYPE *PPCAT(PPCAT(PPCAT(f, INTERFACE),_), method))
#undef STDMETHOD_
#define STDMETHOD_(type,method) typedef type (STDMETHODCALLTYPE *PPCAT(PPCAT(PPCAT(f, INTERFACE),_), method))

// 1:1 copy from d3d9.h:
STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) PURE;
STDMETHOD_(ULONG, AddRef)(THIS) PURE;
STDMETHOD_(ULONG, Release)(THIS) PURE;
STDMETHOD(TestCooperativeLevel)(THIS) PURE;
STDMETHOD_(UINT, GetAvailableTextureMem)(THIS) PURE;
STDMETHOD(EvictManagedResources)(THIS) PURE;
STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9) PURE;
STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps) PURE;
STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode) PURE;
STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters) PURE;
STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) PURE;
STDMETHOD_(void, SetCursorPosition)(THIS_ int X, int Y, DWORD Flags) PURE;
STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow) PURE;
STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) PURE;
STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) PURE;
STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS) PURE;
STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters) PURE;
STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) PURE;
STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) PURE;
STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) PURE;
STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs) PURE;
STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) PURE;
STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp) PURE;
STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) PURE;
STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) PURE;
STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) PURE;
STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) PURE;
STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) PURE;
STDMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) PURE;
STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) PURE;
STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) PURE;
STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) PURE;
STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) PURE;
STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface) PURE;
STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) PURE;
STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) PURE;
STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) PURE;
STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) PURE;
STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) PURE;
STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil) PURE;
STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface) PURE;
STDMETHOD(BeginScene)(THIS) PURE;
STDMETHOD(EndScene)(THIS) PURE;
STDMETHOD(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) PURE;
STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) PURE;
STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) PURE;
STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*) PURE;
STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport) PURE;
STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport) PURE;
STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial) PURE;
STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial) PURE;
STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9*) PURE;
STDMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT9*) PURE;
STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable) PURE;
STDMETHOD(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable) PURE;
STDMETHOD(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane) PURE;
STDMETHOD(GetClipPlane)(THIS_ DWORD Index, float* pPlane) PURE;
STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value) PURE;
STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue) PURE;
STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) PURE;
STDMETHOD(BeginStateBlock)(THIS) PURE;
STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB) PURE;
STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus) PURE;
STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus) PURE;
STDMETHOD(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9** ppTexture) PURE;
STDMETHOD(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9* pTexture) PURE;
STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) PURE;
STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) PURE;
STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) PURE;
STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) PURE;
STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses) PURE;
STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries) PURE;
STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries) PURE;
STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber) PURE;
STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber) PURE;
STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect) PURE;
STDMETHOD(GetScissorRect)(THIS_ RECT* pRect) PURE;
STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware) PURE;
STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS) PURE;
STDMETHOD(SetNPatchMode)(THIS_ float nSegments) PURE;
STDMETHOD_(float, GetNPatchMode)(THIS) PURE;
STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) PURE;
STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) PURE;
STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) PURE;
STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) PURE;
STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) PURE;
STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) PURE;
STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl) PURE;
STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl) PURE;
STDMETHOD(SetFVF)(THIS_ DWORD FVF) PURE;
STDMETHOD(GetFVF)(THIS_ DWORD* pFVF) PURE;
STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) PURE;
STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader) PURE;
STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader) PURE;
STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) PURE;
STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount) PURE;
STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) PURE;
STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount) PURE;
STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) PURE;
STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount) PURE;
STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) PURE;
STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) PURE;
STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT Setting) PURE;
STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT* pSetting) PURE;
STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData) PURE;
STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData) PURE;
STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) PURE;
STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader) PURE;
STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader) PURE;
STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) PURE;
STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount) PURE;
STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) PURE;
STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount) PURE;
STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) PURE;
STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount) PURE;
STDMETHOD(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) PURE;
STDMETHOD(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) PURE;
STDMETHOD(DeletePatch)(THIS_ UINT Handle) PURE;
STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) PURE;

// Create vtable offsets
#undef STDMETHOD
#define STDMETHOD(method) PPCAT(PPCAT(PPCAT(vt, INTERFACE),_), method),
#undef STDMETHOD_
#define STDMETHOD_(type,method) PPCAT(PPCAT(PPCAT(vt, INTERFACE),_), method),

enum PPCAT(vt, INTERFACE)
{
	// 1:1 copy from d3d9.h: replace (THIS* -> ""
	STDMETHOD(QueryInterface)
		STDMETHOD_(ULONG, AddRef)
		STDMETHOD_(ULONG, Release)
		STDMETHOD(TestCooperativeLevel)
		STDMETHOD_(UINT, GetAvailableTextureMem)
		STDMETHOD(EvictManagedResources)
		STDMETHOD(GetDirect3D)
		STDMETHOD(GetDeviceCaps)
		STDMETHOD(GetDisplayMode)
		STDMETHOD(GetCreationParameters)
		STDMETHOD(SetCursorProperties)
		STDMETHOD_(void, SetCursorPosition)
		STDMETHOD_(BOOL, ShowCursor)
		STDMETHOD(CreateAdditionalSwapChain)
		STDMETHOD(GetSwapChain)
		STDMETHOD_(UINT, GetNumberOfSwapChains)
		STDMETHOD(Reset)
		STDMETHOD(Present)
		STDMETHOD(GetBackBuffer)
		STDMETHOD(GetRasterStatus)
		STDMETHOD(SetDialogBoxMode)
		STDMETHOD_(void, SetGammaRamp)
		STDMETHOD_(void, GetGammaRamp)
		STDMETHOD(CreateTexture)
		STDMETHOD(CreateVolumeTexture)
		STDMETHOD(CreateCubeTexture)
		STDMETHOD(CreateVertexBuffer)
		STDMETHOD(CreateIndexBuffer)
		STDMETHOD(CreateRenderTarget)
		STDMETHOD(CreateDepthStencilSurface)
		STDMETHOD(UpdateSurface)
		STDMETHOD(UpdateTexture)
		STDMETHOD(GetRenderTargetData)
		STDMETHOD(GetFrontBufferData)
		STDMETHOD(StretchRect)
		STDMETHOD(ColorFill)
		STDMETHOD(CreateOffscreenPlainSurface)
		STDMETHOD(SetRenderTarget)
		STDMETHOD(GetRenderTarget)
		STDMETHOD(SetDepthStencilSurface)
		STDMETHOD(GetDepthStencilSurface)
		STDMETHOD(BeginScene)
		STDMETHOD(EndScene)
		STDMETHOD(Clear)
		STDMETHOD(SetTransform)
		STDMETHOD(GetTransform)
		STDMETHOD(MultiplyTransform)
		STDMETHOD(SetViewport)
		STDMETHOD(GetViewport)
		STDMETHOD(SetMaterial)
		STDMETHOD(GetMaterial)
		STDMETHOD(SetLight)
		STDMETHOD(GetLight)
		STDMETHOD(LightEnable)
		STDMETHOD(GetLightEnable)
		STDMETHOD(SetClipPlane)
		STDMETHOD(GetClipPlane)
		STDMETHOD(SetRenderState)
		STDMETHOD(GetRenderState)
		STDMETHOD(CreateStateBlock)
		STDMETHOD(BeginStateBlock)
		STDMETHOD(EndStateBlock)
		STDMETHOD(SetClipStatus)
		STDMETHOD(GetClipStatus)
		STDMETHOD(GetTexture)
		STDMETHOD(SetTexture)
		STDMETHOD(GetTextureStageState)
		STDMETHOD(SetTextureStageState)
		STDMETHOD(GetSamplerState)
		STDMETHOD(SetSamplerState)
		STDMETHOD(ValidateDevice)
		STDMETHOD(SetPaletteEntries)
		STDMETHOD(GetPaletteEntries)
		STDMETHOD(SetCurrentTexturePalette)
		STDMETHOD(GetCurrentTexturePalette)
		STDMETHOD(SetScissorRect)
		STDMETHOD(GetScissorRect)
		STDMETHOD(SetSoftwareVertexProcessing)
		STDMETHOD_(BOOL, GetSoftwareVertexProcessing)
		STDMETHOD(SetNPatchMode)
		STDMETHOD_(float, GetNPatchMode)
		STDMETHOD(DrawPrimitive)
		STDMETHOD(DrawIndexedPrimitive)
		STDMETHOD(DrawPrimitiveUP)
		STDMETHOD(DrawIndexedPrimitiveUP)
		STDMETHOD(ProcessVertices)
		STDMETHOD(CreateVertexDeclaration)
		STDMETHOD(SetVertexDeclaration)
		STDMETHOD(GetVertexDeclaration)
		STDMETHOD(SetFVF)
		STDMETHOD(GetFVF)
		STDMETHOD(CreateVertexShader)
		STDMETHOD(SetVertexShader)
		STDMETHOD(GetVertexShader)
		STDMETHOD(SetVertexShaderConstantF)
		STDMETHOD(GetVertexShaderConstantF)
		STDMETHOD(SetVertexShaderConstantI)
		STDMETHOD(GetVertexShaderConstantI)
		STDMETHOD(SetVertexShaderConstantB)
		STDMETHOD(GetVertexShaderConstantB)
		STDMETHOD(SetStreamSource)
		STDMETHOD(GetStreamSource)
		STDMETHOD(SetStreamSourceFreq)
		STDMETHOD(GetStreamSourceFreq)
		STDMETHOD(SetIndices)
		STDMETHOD(GetIndices)
		STDMETHOD(CreatePixelShader)
		STDMETHOD(SetPixelShader)
		STDMETHOD(GetPixelShader)
		STDMETHOD(SetPixelShaderConstantF)
		STDMETHOD(GetPixelShaderConstantF)
		STDMETHOD(SetPixelShaderConstantI)
		STDMETHOD(GetPixelShaderConstantI)
		STDMETHOD(SetPixelShaderConstantB)
		STDMETHOD(GetPixelShaderConstantB)
		STDMETHOD(DrawRectPatch)
		STDMETHOD(DrawTriPatch)
		STDMETHOD(DeletePatch)
		STDMETHOD(CreateQuery)
};


typedef HRESULT(WINAPI *CreateDevice_t)(IDirect3D9* Direct3D_Object, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
	DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
	IDirect3DDevice9** ppReturnedDeviceInterface);
typedef HRESULT(WINAPI *EndScene_t)(IDirect3DDevice9* surface);

CreateDevice_t D3DCreateDevice_orig;
EndScene_t D3DEndScene_orig;

HRESULT WINAPI D3DCreateDevice_hook(IDirect3D9* Direct3D_Object, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
	DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
	IDirect3DDevice9** ppReturnedDeviceInterface);
HRESULT WINAPI D3DEndScene_hook(IDirect3DDevice9* device);

PDWORD IDirect3D9_vtable = NULL;

// Function indices: These are liable to change
// This is the functions index into the vtable
// Find these by reading through d3d9.h and counting in order
// IDirect3D9
#define CREATEDEVICE_VTI 16
// IDirect3DDevice9
#define ENDSCENE_VTI 42

HRESULT WINAPI HookCreateDevice();
DWORD WINAPI VTablePatchThread(LPVOID threadParam);

/**
 * This function sets up the hook for CreateDevice by replacing the pointer
 * to CreateDevice within IDirect3D9's vtable.
 */
HRESULT WINAPI HookCreateDevice()
{
	// Obtain a D3D object
	IDirect3D9* device = Direct3DCreate9(D3D_SDK_VERSION);
	if (!device)
	{
#ifdef _HKDEBUG_
		MessageBoxA(NULL, "Unable to create device", "CreateDevice", MB_ICONEXCLAMATION);
#endif
		return D3DERR_INVALIDCALL;
	}
	// Now we have an object, store a pointer to its vtable and release the object
	IDirect3D9_vtable = (DWORD*)*(DWORD*)device; // Confusing typecasts
	device->Release();

	// Unprotect the vtable for writing
	DWORD protectFlag;
	if (VirtualProtect(&IDirect3D9_vtable[CREATEDEVICE_VTI], sizeof(DWORD), PAGE_READWRITE, &protectFlag))
	{
		// Store the original CreateDevice pointer and shove our own function into the vtable
		*(DWORD*)&D3DCreateDevice_orig = IDirect3D9_vtable[CREATEDEVICE_VTI];
		*(DWORD*)&IDirect3D9_vtable[CREATEDEVICE_VTI] = (DWORD)D3DCreateDevice_hook;

		// Reprotect the vtable
		if (!VirtualProtect(&IDirect3D9_vtable[CREATEDEVICE_VTI], sizeof(DWORD), protectFlag, &protectFlag))
		{
#ifdef _HKDEBUG_
			MessageBoxA(NULL, "Unable to access vtable", "CreateDevice", MB_ICONEXCLAMATION);
#endif
			return D3DERR_INVALIDCALL;
		}
	}
	else {
#ifdef _HKDEBUG_
		MessageBoxA(NULL, "Unable to access vtable", "CreateDevice", MB_ICONEXCLAMATION);
#endif
		return D3DERR_INVALIDCALL;
	}
#ifdef _HKDEBUG_
	MessageBoxA(NULL, "Hooked CreateDevice call", "CreateDevice", MB_ICONEXCLAMATION);
#endif
	return D3D_OK;
}

D3DPRESENT_PARAMETERS presentationParms;
bool renderCommandFlags = 0;
typedef enum RCFlags
{
	RCFResize = 1
};

/**
 * Thanks to HookCreateDevice(), The program should now call this method instead of
 * Direct3D's CreateDevice method. This allows us to then hook the IDirect3DDevice9
 * methods we need
 */
HRESULT WINAPI D3DCreateDevice_hook(IDirect3D9* Direct3D_Object, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
	DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
	IDirect3DDevice9** ppReturnedDeviceInterface)
{
#ifdef _HKDEBUG_
	MessageBoxA(NULL, "CreateDevice called", "CreateDevice", MB_ICONEXCLAMATION);
#endif
	// Append the almighty D3DCREATE_MULTITHREADED flag...
	memcpy(&presentationParms, pPresentationParameters, sizeof(presentationParms));
	HRESULT result = D3DCreateDevice_orig(Direct3D_Object, Adapter, DeviceType, hFocusWindow, BehaviorFlags | D3DCREATE_MULTITHREADED, pPresentationParameters, ppReturnedDeviceInterface);

	// Now we've intercepted the program's call to CreateDevice and we have the IDirect3DDevice9 that it uses
	// We can get it's vtable and patch in our own detours
	// Reset the CreateDevice hook since it's no longer needed
	// Unprotect the vtable for writing
	DWORD protectFlag;
	if (VirtualProtect(&IDirect3D9_vtable[CREATEDEVICE_VTI], sizeof(DWORD), PAGE_READWRITE, &protectFlag))
	{
		// Store the original CreateDevice pointer and shove our own function into the vtable
		*(DWORD*)&IDirect3D9_vtable[CREATEDEVICE_VTI] = (DWORD)D3DCreateDevice_orig;

		// Reprotect the vtable
		if (!VirtualProtect(&IDirect3D9_vtable[CREATEDEVICE_VTI], sizeof(DWORD), protectFlag, &protectFlag))
		{
			return D3DERR_INVALIDCALL;
		}
	}
	else {
		return D3DERR_INVALIDCALL;
	}

	if (result == D3D_OK)
	{
		// Load the new vtable
		IDirect3D9_vtable = (DWORD*)*(DWORD*)*ppReturnedDeviceInterface;
#ifdef _HKDEBUG_
		MessageBoxA(NULL, "Loaded IDirect3DDevice9 vtable", "CreateDevice", MB_ICONEXCLAMATION);
#endif

		// Store pointers to the original functions that we want to hook
		*(PDWORD)&D3DEndScene_orig = (DWORD)IDirect3D9_vtable[ENDSCENE_VTI];

		if (!CreateThread(NULL, 0, VTablePatchThread, NULL, NULL, NULL))
		{
			return D3DERR_INVALIDCALL;
		}
	}
	device = *ppReturnedDeviceInterface;
	return result;
}

/**
 * This is a thread which indefinately resets the vtable pointers to our own functions
 * This is needed because the program might set these pointers back to
 * their original values at any point
 */
DWORD WINAPI VTablePatchThread(LPVOID threadParam)
{
#ifdef _HKDEBUG_
	MessageBoxA(NULL, "VTable patch thread started", "Patch Thread", MB_ICONEXCLAMATION);
#endif
	while (true)
	{
		Sleep(100);

		*(DWORD*)&IDirect3D9_vtable[ENDSCENE_VTI] = (DWORD)D3DEndScene_hook;
	}
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = device->Reset(&presentationParms);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

/**
 * This is called when each frame has finished
 * from here we can intercept the video output
 */
HRESULT WINAPI D3DEndScene_hook(IDirect3DDevice9* device)
{
	HRESULT rresult;
	/*
	if (!g_mpegEncoder)
	{
		g_mpegEncoder = new MPEGEncoder(device);
		result = D3DEndScene_orig(device);
		g_mpegEncoder->Start();
		return result;
	}
	*/
	//Com_Printf("endscene!\n");
	static bool do_once = false;
	extern bool imguiEnabled;
	if (imguiEnabled)
	{
		if (!do_once)
		{
			HWND hWnd = FindWindowA(NULL, "Call of Duty 2 Multiplayer");
			if (!hWnd) {
				MessageBoxA(0, "failed to find wnd!", "", 0);
				exit(0);
			}
			// Setup Dear ImGui binding
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;

			ImGui_ImplWin32_Init(hWnd);
			ImGui_ImplDX9_Init(device);

			ImGui::StyleColorsDark();

			do_once = true;
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		bool aaa = true;
		ImGui::ShowDemoWindow(&aaa);

		void handleImGuiWindows();
		handleImGuiWindows();
	}
	auto *g_pd3dDevice = device;

	// Rendering
	if(imguiEnabled)
	ImGui::EndFrame();
#if 0
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	//D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*255.0f), (int)(clear_color.y*255.0f), (int)(clear_color.z*255.0f), (int)(clear_color.w*255.0f));
	//g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

	if (g_pd3dDevice->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		g_pd3dDevice->EndScene();
	}
	HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
#endif
	if (imguiEnabled)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	if (renderCommandFlags & RCFResize)
	{
		ResetDevice();
	}

	rresult = D3DEndScene_orig(device);

	// Here, we can get the output of the d3d device using GetBackBuffer, and send it off to
	// a file, or to be encoded, or whatever. *borat* Great success!!

	//D3DXSaveSurfaceToFile("Capture.bmp", D3DXIFF_BMP, capture, NULL, NULL);

	return rresult;
}

bool r_inited = false;

DWORD gfx_dll_mp;
#define GFX_OFF(x) (gfx_dll_mp + (x - 0x10000000))

//IDirect3D9 *d3d9 = NULL;
#if 0
#undef METHOD
#undef INTERFACE

#define INTERFACE IDirect3DDevice9
#define METHOD Reset
GEN_HOOK(INTERFACE FAR* This, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	Com_Printf("^6Reset!\n");
	MessageBoxA(0, 0, 0, 0);
	CALL_ORGINAL(pPresentationParameters);

	return hr;
}
#undef METHOD
#undef METHOD
#undef INTERFACE

#define INTERFACE IDirect3DDevice9
#define METHOD EndScene
GEN_HOOK(INTERFACE FAR* This)
{
	Com_Printf("^EndScene!\n");
	MessageBoxA(0, 0, 0, 0);
	CALL_ORGINAL();

	return hr;
}
#undef METHOD
#endif

void R_Resize(int w, int h)
{
	if (device)
	{
		presentationParms.BackBufferWidth = w;
		presentationParms.BackBufferHeight = h;
		renderCommandFlags |= RCFResize;
	}
}

void R_Init()
{

	void(*o)(void) = (void(*)(void))GFX_OFF(0x10013080);
	o();

	void patch_opcode_glbindtexture(void);
	//patch_opcode_glbindtexture();

	HWND wnd = FindWindowA(NULL, "Call of Duty 2 Multiplayer");
	Com_Printf("^6wnd = %02X\n", wnd);
	Com_Printf("^6bla: %d\n", GetCurrentThreadId());
	//TODO better check
	Com_Printf("^2dx device = %02X\n", device);

	if (device)
	{

		//hookVT(device, IDirect3DDevice9, Reset);
		//hookVT(device, IDirect3DDevice9, EndScene);
		//ImGui_ImplDX9_Init(device);
	}
	if(wnd)
	r_inited = true;
}

static DWORD addr;

void __declspec(naked) creating_dx_device()
{
	__asm
	{
		//og 10012730
		lea eax, addr
		call [eax]
		//mov device, eax
		retn
	}
}

void patch_gfx_dll()
{
	extern bool imguiEnabled;
	//sleep
	//__nop(GFX_OFF(0x10012778), GFX_OFF(0x10012778) + 8);

	//device = (IDirect3DDevice9*)GFX_OFF(0x101D1BF8);

	addr = GFX_OFF(0x10012730);
	//__call(GFX_OFF(0x1001294D), (int)creating_dx_device);
	__call(GFX_OFF(0x100132A2), (int)R_Init);
	/*
	if (storage::file_exists("qdraw.dll"))
	{
		XUNLOCK((void*)0x1019CC68, 1);
		*(unsigned char*)0x1019CC68 = 'q';
	}
	*/
	//extern char sys_cmdline[1024];
	cvar_t* r_windowed = Cvar_RegisterBool("r_windowed", false, CVAR_ARCHIVE);
	if (r_windowed->boolean) {
		XUNLOCK((void*)GFX_OFF(0x10011564), 1);
		XUNLOCK((void*)GFX_OFF(0x10012A8A), 1);
		*(unsigned char*)GFX_OFF(0x10011564) = 0xeb;
		*(unsigned char*)GFX_OFF(0x10012A8A) = 0xeb;
	}
	else {
		HookCreateDevice();
	}//STUFF FOR IMGUI
}