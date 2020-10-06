//
//		SpoutCam - filters.h
//
//	Updated 23.08.13
//	Updated 24.12.13
//	Updated 03.01.14 - cleanup
//	Updated 10.04.14
//	28.09.15 - updated with modifications by John MacCormick, 2012.
//	10.07.16 - Modified for "SpoutCamConfig" for starting fps and resolution
//	17.10.19 - Clean up for DirectX methods
//

#pragma once

#pragma warning(disable:4995)

#define DECLARE_PTR(type, ptr, expr) type* ptr = (type*)(expr);


// leak checking
// http://www.codeproject.com/Articles/9815/Visual-Leak-Detector-Enhanced-Memory-Leak-Detectio
//
// #include "libs\vld.h"
//
// http://msdn.microsoft.com/en-us/library/x98tx3cf%28VS.71%29.aspx
//
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "..\SpoutDX\source\SpoutDX.h"
#include <streams.h>

EXTERN_C const GUID CLSID_SpoutCam;
EXTERN_C const WCHAR SpoutCamName[MAX_PATH];

class CVCamStream;
class CVCam : public CSource
{
public:
    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);

	// LJ additons
	STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    IFilterGraph *GetGraph() {return m_pGraph;}

	STDMETHODIMP JoinFilterGraph(__inout_opt IFilterGraph * pGraph, __in_opt LPCWSTR pName);

private:

    CVCam(LPUNKNOWN lpunk, HRESULT *phr);

/////////////////////////////////////
// all inherited virtual functions //
/////////////////////////////////////
public:

	virtual CBasePin *GetPin(int n);
	#ifdef PERF
    virtual void RegisterPerfId() { m_idTransInPlace = MSR_REGISTER(TEXT("Vcam")); }
	#endif // PERF
	virtual int GetPinCount();
	virtual HRESULT StreamTime(CRefTime& rtStream);
	virtual LONG GetPinVersion();
	virtual __out_opt LPAMOVIESETUP_FILTER GetSetupData();
	virtual HRESULT STDMETHODCALLTYPE EnumPins(__out  IEnumPins **ppEnum);
	virtual HRESULT STDMETHODCALLTYPE FindPin(LPCWSTR Id, __out  IPin **ppPin);
	virtual HRESULT STDMETHODCALLTYPE QueryFilterInfo(__out  FILTER_INFO *pInfo);
	virtual HRESULT STDMETHODCALLTYPE QueryVendorInfo(__out  LPWSTR *pVendorInfo);
	virtual HRESULT STDMETHODCALLTYPE Stop( void);
	virtual HRESULT STDMETHODCALLTYPE Pause( void);
	virtual HRESULT STDMETHODCALLTYPE Run(REFERENCE_TIME tStart);
	// virtual HRESULT STDMETHODCALLTYPE GetState(DWORD dwMilliSecsTimeout, __out  FILTER_STATE *State);
	virtual HRESULT STDMETHODCALLTYPE SetSyncSource(__in_opt  IReferenceClock *pClock);
	virtual HRESULT STDMETHODCALLTYPE GetSyncSource(__deref_out_opt  IReferenceClock **pClock);
	virtual STDMETHODIMP GetClassID(__out CLSID *pClsID);
	virtual ULONG STDMETHODCALLTYPE AddRef( void);
	virtual ULONG STDMETHODCALLTYPE Release( void);
	virtual HRESULT STDMETHODCALLTYPE Register( void);
    virtual HRESULT STDMETHODCALLTYPE Unregister( void);


};

class CVCamStream : public CSourceStream, public IAMStreamConfig, public IKsPropertySet, public IAMDroppedFrames
{

public:

    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef() { return GetOwner()->AddRef(); }                                                          \
    STDMETHODIMP_(ULONG) Release() { return GetOwner()->Release(); }

    //////////////////////////////////////////////////////////////////////////
    //  IQualityControl
    //////////////////////////////////////////////////////////////////////////
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    //////////////////////////////////////////////////////////////////////////
    //  IAMStreamConfig
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **ppmt);
    HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *piCount, int *piSize);
    HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC);

	/////////////////// RED5 /////////////////////////////////////////////////
    //  IAMDroppedFrames
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE GetAverageFrameSize( long* plAverageSize);
	HRESULT STDMETHODCALLTYPE GetDroppedInfo(long  lSize,long* plArray,long* plNumCopied);
	HRESULT STDMETHODCALLTYPE GetNumDropped(long *plDropped);
	HRESULT STDMETHODCALLTYPE GetNumNotDropped(long *plNotDropped);

    //////////////////////////////////////////////////////////////////////////
    //  IKsPropertySet
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
    HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID, void *pInstanceData,DWORD cbInstanceData, void *pPropData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);
    
 	//////////////////////////////////////////////////////////////////////////
    //  IPin
    //////////////////////////////////////////////////////////////////////////
	virtual HRESULT STDMETHODCALLTYPE QueryPinInfo(__out  PIN_INFO *pInfo);
	virtual HRESULT STDMETHODCALLTYPE Connect(IPin *pReceivePin, __in_opt  const AM_MEDIA_TYPE *pmt);
	virtual HRESULT STDMETHODCALLTYPE ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
	virtual HRESULT STDMETHODCALLTYPE Disconnect( void);
	virtual HRESULT STDMETHODCALLTYPE ConnectedTo(__out  IPin **pPin);
	virtual HRESULT STDMETHODCALLTYPE ConnectionMediaType(__out  AM_MEDIA_TYPE *pmt);
	virtual HRESULT STDMETHODCALLTYPE QueryDirection(__out  PIN_DIRECTION *pPinDir);
	virtual HRESULT STDMETHODCALLTYPE QueryId(__out  LPWSTR *Id);
	virtual HRESULT STDMETHODCALLTYPE QueryAccept(const AM_MEDIA_TYPE *pmt);
	virtual HRESULT STDMETHODCALLTYPE EnumMediaTypes(__out  IEnumMediaTypes **ppEnum);
	virtual HRESULT STDMETHODCALLTYPE QueryInternalConnections(__out_ecount_part_opt(*nPin, *nPin) IPin **apPin, ULONG *nPin);
	virtual HRESULT STDMETHODCALLTYPE EndOfStream( void);
	virtual HRESULT STDMETHODCALLTYPE BeginFlush( void);
	virtual HRESULT STDMETHODCALLTYPE EndFlush( void);
	virtual HRESULT STDMETHODCALLTYPE NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	///////// jmac ////////
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv);

	//////////////////////////////////////////////////////////////////////////
    //  CSourceStream
    //////////////////////////////////////////////////////////////////////////
    CVCamStream(HRESULT *phr, CVCam *pParent, LPCWSTR pPinName);
    ~CVCamStream();

    HRESULT FillBuffer(IMediaSample *pms);
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProperties);
	// HRESULT GetMediaType(CMediaType *pmt);
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT OnThreadCreate(void);

	int m_Fps;
	int m_Resolution;
	bool m_bLock;
	
	void SetFps(DWORD dwFps);
	void SetResolution(DWORD dwResolution);

	// ============== IPC functions ==============
	//
	//	opengl/directx interop texture sharing
	//
	// SPOUTDX
	spoutDX receiver;
	
	char g_SenderName[256];
	char g_ActiveSender[256];    // The name of any Spout sender being received
	ID3D11Device* g_pd3dDevice;  // DirectX 11.0 device pointer
	bool bMemoryMode;            // true = memory, false = texture
	bool bInvert;
	bool bDebug;
	bool bInitialized;
	bool bDXinitialized;
	bool bDisconnected;				// Sender had started but it has stopped or changed image size

	GLenum glBGRmode;

	unsigned int g_Width;			 // The global filter image width
	unsigned int g_Height;			 // The global filter image height

	DWORD dwFps;					// Fps from SpoutCamConfig
	DWORD dwResolution;				// Resolution from SpoutCamConfig
	DWORD dwLock;                   // Fix to the selected resolution
	int g_FrameTime;                // Frame time to use based on fps selection

private:

	CVCam *m_pParent;
	long long NumDroppedFrames, NumFrames;
	REFERENCE_TIME 
		m_rtLastTime,	// running timestamp
		refSync1,		// Graphmanager clock time, to compute dropped frames.
		refSync2,		// Clock time for Sleeping each frame if not dropping.
		refStart,		// Real time at start from Graphmanager clock time.
		rtStreamOff;	// IAMPushSource Get/Set data member.

	DWORD dwLastTime;
    CCritSec m_cSharedState;
    IReferenceClock *m_pClock;

	///////// jmac ////////
	LONG GetMediaTypeVersion();
	HRESULT CompleteConnect(IPin *pReceivePin);
	HRESULT CheckConnect(IPin *pPin);
	HRESULT BreakConnect();
	protected:
	HRESULT Active(void);
	////////////////////////////// end jmac /////////////


};
