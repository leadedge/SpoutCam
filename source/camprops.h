#include <dshow.h>

class CSpoutCamProperties : public CBasePropertyPage
{

public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

	INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT OnConnect(IUnknown *pUnknown);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	// HRESULT OnDeactivate();
	HRESULT OnApplyChanges();

	CSpoutCamProperties(LPUNKNOWN lpunk, HRESULT *phr);

	BOOL m_bIsInitialized;      // Used to ignore startup messages
	ICamSettings *m_pCamSettings;     // The custom interface on the filter

};

