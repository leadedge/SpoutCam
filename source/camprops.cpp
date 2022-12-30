#include <stdio.h>
#include <conio.h>
#include <olectl.h>
#include <dshow.h>

#include "cam.h"

#include "camprops.h"
#include "resource.h"
#include "version.h"


// For dialog drawing
static HBRUSH g_hbrBkgnd = NULL;

// CreateInstance
// Used by the DirectShow base classes to create instances
CUnknown *CSpoutCamProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	ASSERT(phr);
	CUnknown *punk = new CSpoutCamProperties(lpunk, phr);
	if (punk == NULL)
	{
		if (phr)
			*phr = E_OUTOFMEMORY;
	}
	return punk;
} // CreateInstance

// Constructor
CSpoutCamProperties::CSpoutCamProperties(LPUNKNOWN pUnk, HRESULT *phr) :
	CBasePropertyPage("Settings", pUnk, IDD_SPOUTCAMBOX, IDS_TITLE),
	m_pCamSettings(NULL),
	m_bIsInitialized(FALSE)
{
	ASSERT(phr);
	if (phr)
		*phr = S_OK;
}

//OnReceiveMessage is called when the dialog receives a window message.
INT_PTR CSpoutCamProperties::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HCURSOR cursorHand = NULL;
	LPDRAWITEMSTRUCT lpdis;
	std::wstring wstr;

	switch (uMsg)
	{
		// Owner draw button
	case WM_CTLCOLORSTATIC:
		{
			HDC hdcStatic = (HDC)wParam;
			
			// Make disable warnings light grey
			if (GetDlgItem(hwnd, IDC_SILENT) == (HWND)lParam) {
				SetTextColor(hdcStatic, RGB(128, 128, 128));
				SetBkColor(hdcStatic, RGB(240, 240, 240));
				if (g_hbrBkgnd == NULL)
					g_hbrBkgnd = CreateSolidBrush(RGB(240, 240, 240));
			}

			// Make version light grey
			if (GetDlgItem(hwnd, IDC_VERS) == (HWND)lParam) {
				SetTextColor(hdcStatic, RGB(128, 128, 128));
				SetBkColor(hdcStatic, RGB(240, 240, 240));
				if (g_hbrBkgnd == NULL)
					g_hbrBkgnd = CreateSolidBrush(RGB(240, 240, 240));
			}

			return (INT_PTR)g_hbrBkgnd;
		}
		break;

	case WM_DRAWITEM:
			lpdis = (LPDRAWITEMSTRUCT)lParam;
			if (lpdis->itemID == -1) break;
			switch (lpdis->CtlID) {
				// The blue hyperlink
				case IDC_SPOUT_URL:
					SetTextColor(lpdis->hDC, RGB(6, 69, 173));
					DrawText(lpdis->hDC, L"https://spout.zeal.co", -1, &lpdis->rcItem, DT_LEFT);
					break;

				default:
					break;
			}
			break;

		case WM_INITDIALOG:
			// Hyperlink hand cursor
			cursorHand = LoadCursor(NULL, IDC_HAND);
			SetClassLongPtr(GetDlgItem(hwnd, IDC_SPOUT_URL), GCLP_HCURSOR, (LONG_PTR)cursorHand);
			break;

		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {

				case IDC_SPOUT_URL :
					ShellExecute(hwnd, L"open", L"http://spout.zeal.co", NULL, NULL, SW_SHOWNORMAL);
					break;
				
				case IDC_SILENT:
					// Toggle warning silent mode
					if (IsDlgButtonChecked(hwnd, IDC_SILENT) == BST_CHECKED)
						m_bSilent = TRUE;
					else
						m_bSilent = FALSE;
					break;

				default :
					break;
			}

			if (m_bIsInitialized)
			{
				m_bDirty = TRUE;
				if (m_pPageSite)
				{
					m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
				}
			}

			return (LRESULT)1;
		}
	}

	// Let the parent class handle the message.
	return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

//OnConnect is called when the client creates the property page. It sets the IUnknown pointer to the filter.
HRESULT CSpoutCamProperties::OnConnect(IUnknown *pUnknown)
{
	TRACE("OnConnect");
	CheckPointer(pUnknown, E_POINTER);
	ASSERT(m_pCamSettings == NULL);

	HRESULT hr = pUnknown->QueryInterface(IID_ICamSettings, (void **)&m_pCamSettings);
	if (FAILED(hr))
	{
		return E_NOINTERFACE;
	}

	// Get the initial image FX property
	CheckPointer(m_pCamSettings, E_FAIL);

	m_bIsInitialized = FALSE;

	return S_OK;
}

//OnDisconnect is called when the user dismisses the property sheet.
HRESULT CSpoutCamProperties::OnDisconnect()
{
	TRACE("OnDisconnect");
	// Release of Interface after setting the appropriate old effect value
	if (m_pCamSettings)
	{
		m_pCamSettings->Release();
		m_pCamSettings = NULL;
	}

	return S_OK;
}

//OnActivate is called when the dialog is created.
HRESULT CSpoutCamProperties::OnActivate()
{
	TRACE("OnActivate");

	HWND hwndCtl = nullptr;
	DWORD dwValue = 0;
	wchar_t wname[256];
	char name[256];

	////////////////////////////////////////
	// Fps
	////////////////////////////////////////

	// Retrieve fps from registry
	if (!ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "fps", &dwValue) || dwValue > 5)
	{
		dwValue = 3; // default 3 = 30 fps
	}

	hwndCtl = GetDlgItem(this->m_Dlg, IDC_FPS);
	
	WCHAR fps_values[6][3] =
	{
		L"10",
		L"15",
		L"25",
		L"30", // default
		L"50",
		L"60"
	};

	WCHAR fps[3];
	int k = 0;

	memset(&fps, 0, sizeof(fps));
	for (k = 0; k <= 5; k += 1)
	{
		wcscpy_s(fps, sizeof(fps) / sizeof(WCHAR), fps_values[k]);
		ComboBox_AddString(hwndCtl, fps);
	}

	// Select default
	ComboBox_SetCurSel(hwndCtl, dwValue);

	////////////////////////////////////////
	// Resolution
	////////////////////////////////////////

	// Retrieve resolution from registry "SpoutCamConfig"
	if (!ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "resolution", &dwValue) || dwValue > 10)
	{
		dwValue = 0; // default 0 = active sender
	}

	hwndCtl = GetDlgItem(this->m_Dlg, IDC_RESOLUTION);

	WCHAR res_values[11][14] =
	{
		L"Active Sender",
		L"320 x 240",
		L"640 x 360",
		L"640 x 480", // default if no sender
		L"800 x 600",
		L"1024 x 720",
		L"1024 x 768",
		L"1280 x 720",
		L"1280 x 960",
		L"1280 x 1024",
		L"1920 x 1080"
	};

	WCHAR res[14];
	memset(&res, 0, sizeof(res));
	for (k = 0; k <= 10; k += 1)
	{
		wcscpy_s(res, sizeof(res) / sizeof(WCHAR), res_values[k]);
		ComboBox_AddString(hwndCtl, res);
	}

	// Select current value
	ComboBox_SetCurSel(hwndCtl, dwValue);

	////////////////////////////////////////
	// Starting sender name
	////////////////////////////////////////

	// Note - registry read is not wide chars
	if (!ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "senderstart", name))
	{
		wname[0] = 0;
	}
	if (wname[0] != 0) {
		// Convert registry char* string to a wchar_t* string.
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wname, 256, name, 256);
		// Show it in the dialog edit control
		hwndCtl = GetDlgItem(this->m_Dlg, IDC_NAME);
		Edit_SetText(hwndCtl, wname);
	}

	////////////////////////////////////////
	// Mirror image
	////////////////////////////////////////
	if (!ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "mirror", &dwValue))
	{
		dwValue = 0;
	}
	hwndCtl = GetDlgItem(this->m_Dlg, IDC_MIRROR);
	Button_SetCheck(hwndCtl, dwValue);

	////////////////////////////////////////
	// Swap RGB <> BGR
	////////////////////////////////////////
	if (!ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "swap", &dwValue))
	{
		dwValue = 0;
	}
	hwndCtl = GetDlgItem(this->m_Dlg, IDC_SWAP);
	Button_SetCheck(hwndCtl, dwValue);

	////////////////////////////////////////
	// Flip image
	// Default is flipped due to upside down Windows bitmap
	// If set false, the result comes out inverted
	// bInvert = false;
	////////////////////////////////////////
	if (!ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "flip", &dwValue))
	{
		dwValue = 0;
	}
	hwndCtl = GetDlgItem(this->m_Dlg, IDC_FLIP);
	Button_SetCheck(hwndCtl, dwValue);

	// Warning disable mode
	if (!ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "silent", &dwValue))
	{
		dwValue = 0; // Disable warnings off by default
	}
	m_bSilent = (dwValue > 0);
	hwndCtl = GetDlgItem(this->m_Dlg, IDC_SILENT);
	Button_SetCheck(hwndCtl, dwValue);

	// Show SpoutCam version
	hwndCtl = GetDlgItem(this->m_Dlg, IDC_VERS);
	Static_SetText(hwndCtl, L"Version: " _VER_VERSION_STRING);

	m_bIsInitialized = TRUE;

	if (m_pPageSite)
	{
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
	}

	return S_OK;
}

//OnDeActivate is called when the dialog is closed.
HRESULT CSpoutCamProperties::OnDeactivate()
{
	if (g_hbrBkgnd)
		DeleteObject(g_hbrBkgnd);
	return S_OK;
}

//OnApplyChanges is called when the user commits the property changes by clicking the OK or Apply button.
HRESULT CSpoutCamProperties::OnApplyChanges()
{
	TRACE("OnApplyChanges");

	DWORD dwFps, dwResolution, dwMirror, dwSwap, dwFlip, dwSilent;
	wchar_t wname[256];
	char name[256];

	// =================================
	// Get old fps and resolution for user warning
	DWORD dwOldFps, dwOldResolution;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "fps", &dwOldFps);
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "resolution", &dwOldResolution);
	dwFps = ComboBox_GetCurSel(GetDlgItem(this->m_Dlg, IDC_FPS));
	dwResolution = ComboBox_GetCurSel(GetDlgItem(this->m_Dlg, IDC_RESOLUTION));
	// Warn unless disabled
	if (!m_bSilent) {
		if (dwOldFps != dwFps || dwOldResolution != dwResolution) {
			if (MessageBoxA(NULL, "For change of resolution or fps, you\nhave to stop and re-start SpoutCam\nDo you want to change ? ", "Warning", MB_YESNO | MB_TOPMOST | MB_ICONQUESTION) == IDNO) {
				if (dwOldFps != dwFps)
					ComboBox_SetCurSel(GetDlgItem(this->m_Dlg, IDC_FPS), dwOldFps);
				if (dwOldResolution != dwResolution)
					ComboBox_SetCurSel(GetDlgItem(this->m_Dlg, IDC_RESOLUTION), dwOldResolution);
				return -1;
			}
		}
	}
	WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "fps", dwFps);
	WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "resolution", dwResolution);
	// =================================

	// If properties is implemented, the dialog may need to be updated 
	// to be the same as SpoutCamSettings
	HWND hwndCtl = GetDlgItem(this->m_Dlg, IDC_NAME);
	wname[0] = 0;
	Edit_GetText(hwndCtl, wname, 256);
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, name, 256, wname, _TRUNCATE);
	WritePathToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "senderstart", name);

	hwndCtl = GetDlgItem(this->m_Dlg, IDC_MIRROR);
	dwMirror = Button_GetCheck(hwndCtl);
	WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "mirror", dwMirror);

	hwndCtl = GetDlgItem(this->m_Dlg, IDC_SWAP);
	dwSwap = Button_GetCheck(hwndCtl);
	WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "swap", dwSwap);

	hwndCtl = GetDlgItem(this->m_Dlg, IDC_FLIP);
	dwFlip = Button_GetCheck(hwndCtl);
	WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "flip", dwFlip);

	// Silent mode is set only by the properties dialog
	dwSilent = Button_GetCheck(GetDlgItem(this->m_Dlg, IDC_SILENT));
	WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "silent", dwSilent);
	
	if (m_pCamSettings)
		m_pCamSettings->put_Settings(dwFps, dwResolution, dwMirror, dwSwap, dwFlip, name);

	return S_OK;
}
