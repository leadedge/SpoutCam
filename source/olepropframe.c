/*
 * Valentin Schmidt 2020
 *
 * Based on Wine code (https://dl.winehq.org/wine/source/) and this suggested extension/fix by
 * Jason Winter: https://www.winehq.org/pipermail/wine-bugs/2015-July/417234.html
 */
#ifdef DIALOG_WITHOUT_REGISTRATION
#include <stdarg.h>

#define COBJMACROS
#define NONAMELESSUNION

#include <windows.h>
#include "ole2.h"
#include "olectl.h"
#include "oledlg.h"

#define DWL_MSGRESULT 0
static HWND hwndFrom = NULL;
static HWND hwndFor = NULL;

typedef struct 
{
    IPropertyPageSite IPropertyPageSite_iface;
    LCID lcid;
    LONG ref;
} PropertyPageSite;

static inline PropertyPageSite *impl_from_IPropertyPageSite(IPropertyPageSite *iface)
{
    return CONTAINING_RECORD(iface, PropertyPageSite, IPropertyPageSite_iface);
}

static INT_PTR CALLBACK property_sheet_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    IPropertyPage *property_page = (IPropertyPage*)GetWindowLongPtrW(hwnd, DWLP_USER);

    switch(msg) 
	{
    case WM_INITDIALOG: 
		{
			RECT rect;
			property_page = (IPropertyPage*)((LPPROPSHEETPAGEW)lparam)->lParam;
			GetClientRect(hwnd, &rect);
			IPropertyPage_Activate(property_page, hwnd, &rect, TRUE);
			IPropertyPage_Show(property_page, SW_SHOW);
			SetWindowLongPtrW(hwnd, DWLP_USER, (LONG_PTR)property_page);
			return FALSE;
		}

	case WM_NOTIFY:
		switch (((NMHDR FAR *)lparam)->code)
		{
		case PSN_APPLY:
			if (IPropertyPage_Apply(property_page) == NOERROR)
			{
				SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
				PropSheet_UnChanged(((NMHDR FAR *)lparam)->hwndFrom, hwnd); // Reset Apply button
			}
			else
				SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_INVALID);
			break;
		case PSN_SETACTIVE:
			hwndFrom = ((NMHDR FAR *)lparam)->hwndFrom;
			hwndFor = hwnd;
			break;
		case PSN_KILLACTIVE:
			SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
			break;
		}
		return FALSE;

    case WM_DESTROY:
        IPropertyPage_Show(property_page, SW_HIDE);
        IPropertyPage_Deactivate(property_page);
		hwndFrom = NULL;
		hwndFor = NULL;
        return FALSE;

    default:
        return FALSE;
    }
}

static HRESULT WINAPI PropertyPageSite_QueryInterface(IPropertyPageSite*  iface,
        REFIID  riid, void**  ppv)
{
    if(IsEqualGUID(&IID_IUnknown, riid)
            || IsEqualGUID(&IID_IPropertyPageSite, riid))
        *ppv = iface;
    else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    IUnknown_AddRef((IUnknown*)*ppv);
    return S_OK;
}

static ULONG WINAPI PropertyPageSite_AddRef(IPropertyPageSite* iface)
{
    PropertyPageSite *this = impl_from_IPropertyPageSite(iface);
    LONG ref = InterlockedIncrement(&this->ref);
    return ref;
}

static ULONG WINAPI PropertyPageSite_Release(IPropertyPageSite* iface)
{
    PropertyPageSite *this = impl_from_IPropertyPageSite(iface);
    LONG ref = InterlockedDecrement(&this->ref);
    if(!ref)
        HeapFree(GetProcessHeap(), 0, this);
    return ref;
}

static HRESULT WINAPI PropertyPageSite_OnStatusChange(
		IPropertyPageSite *iface, DWORD dwFlags)
{
	IPropertyPage *property_page;
	if ((hwndFrom) && (hwndFor))
	{ 
		// Were they set?
		if ((dwFlags & PROPPAGESTATUS_DIRTY) == PROPPAGESTATUS_DIRTY)
			PropSheet_Changed(hwndFrom, hwndFor); // Enable Apply button

		if ((dwFlags & PROPPAGESTATUS_VALIDATE) == PROPPAGESTATUS_VALIDATE)
		{
			property_page = (IPropertyPage*)GetWindowLongPtrW(hwndFor,
				DWLP_USER);
			if ((property_page) &&
				(IPropertyPage_Apply(property_page) == NOERROR))
				PropSheet_UnChanged(hwndFrom, hwndFor); // Reset Apply button
		}

		if ((dwFlags & PROPPAGESTATUS_CLEAN) == PROPPAGESTATUS_CLEAN)
			PropSheet_UnChanged(hwndFrom, hwndFor); // Reset Apply button
	}
	return S_OK;
}

static HRESULT WINAPI PropertyPageSite_GetLocaleID(
        IPropertyPageSite *iface, LCID *pLocaleID)
{
    PropertyPageSite *this = impl_from_IPropertyPageSite(iface);
    *pLocaleID = this->lcid;
    return S_OK;
}

static HRESULT WINAPI PropertyPageSite_GetPageContainer(
        IPropertyPageSite* iface, IUnknown** ppUnk)
{
    return E_NOTIMPL;
}

static HRESULT WINAPI PropertyPageSite_TranslateAccelerator(
        IPropertyPageSite* iface, MSG *pMsg)
{
    return E_NOTIMPL;
}

static IPropertyPageSiteVtbl PropertyPageSiteVtbl = {
    PropertyPageSite_QueryInterface,
    PropertyPageSite_AddRef,
    PropertyPageSite_Release,
    PropertyPageSite_OnStatusChange,
    PropertyPageSite_GetLocaleID,
    PropertyPageSite_GetPageContainer,
    PropertyPageSite_TranslateAccelerator
};

LONG WINAPI GdiGetCharDimensions(HDC hdc, LPTEXTMETRICW lptm, LONG *height)
{
	SIZE sz;
	static const WCHAR alphabet[] = {
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q',
		'r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H',
		'I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',0 };
	if (lptm && !GetTextMetricsW(hdc, lptm)) return 0;
	if (!GetTextExtentPointW(hdc, alphabet, 52, &sz)) return 0;
	if (height) *height = sz.cy;
	return (sz.cx / 26 + 1) / 2;
}

/***********************************************************************
 * Custom function by Valentin Schmidt, based on Wine's OleCreatePropertyFrame implementation
 */
HRESULT WINAPI OleCreatePropertyFrameDirect(
	HWND hwndOwner, 
	LPCOLESTR lpszCaption, 
	LPUNKNOWN* ppUnk, 
	IPropertyPage * page)
{
	static const WCHAR comctlW[] = { 'c','o','m','c','t','l','3','2','.','d','l','l',0 };
	PROPSHEETHEADERW property_sheet;
	PROPSHEETPAGEW property_sheet_page;
	struct
	{
		DLGTEMPLATE template;
		WORD menu;
		WORD class;
		WORD title;
	} *dialogs;
	PropertyPageSite *property_page_site;
	HRESULT res;
	HMODULE hcomctl;
	HRSRC property_sheet_dialog_find = NULL;
	HGLOBAL property_sheet_dialog_load = NULL;
	WCHAR *property_sheet_dialog_data = NULL;
	HDC hdc;
	LOGFONTW font_desc;
	HFONT hfont;
	LONG font_width = 4, font_height = 8;
	hdc = GetDC(NULL);
	hcomctl = LoadLibraryW(comctlW);
	if (hcomctl)
		property_sheet_dialog_find = FindResourceW(hcomctl,
			MAKEINTRESOURCEW(1006 /*IDD_PROPSHEET*/), (LPWSTR)RT_DIALOG);
	if (property_sheet_dialog_find)
		property_sheet_dialog_load = LoadResource(hcomctl, property_sheet_dialog_find);
	if (property_sheet_dialog_load)
		property_sheet_dialog_data = LockResource(property_sheet_dialog_load);

	if (property_sheet_dialog_data)
	{
		if (property_sheet_dialog_data[1] == 0xffff)
		{
			FreeLibrary(hcomctl);
			return E_OUTOFMEMORY;
		}

		property_sheet_dialog_data += sizeof(DLGTEMPLATE) / sizeof(WCHAR);
		/* Skip menu, class and title */
		property_sheet_dialog_data += lstrlenW(property_sheet_dialog_data) + 1;
		property_sheet_dialog_data += lstrlenW(property_sheet_dialog_data) + 1;
		property_sheet_dialog_data += lstrlenW(property_sheet_dialog_data) + 1;

		memset(&font_desc, 0, sizeof(LOGFONTW));
		/* Calculate logical height */
		font_desc.lfHeight = -MulDiv(property_sheet_dialog_data[0],
			GetDeviceCaps(hdc, LOGPIXELSY), 72);
		font_desc.lfCharSet = DEFAULT_CHARSET;
		memcpy(font_desc.lfFaceName, property_sheet_dialog_data + 1,
			sizeof(WCHAR)*(lstrlenW(property_sheet_dialog_data + 1) + 1));
		hfont = CreateFontIndirectW(&font_desc);

		if (hfont)
		{
			hfont = SelectObject(hdc, hfont);
			font_width = GdiGetCharDimensions(hdc, NULL, &font_height);
			SelectObject(hdc, hfont);
		}
	}
	if (hcomctl)
		FreeLibrary(hcomctl);
	ReleaseDC(NULL, hdc);

	memset(&property_sheet, 0, sizeof(property_sheet));
	property_sheet.dwSize = sizeof(property_sheet);
	if (lpszCaption)
	{
		property_sheet.dwFlags = PSH_PROPTITLE;
		property_sheet.pszCaption = lpszCaption;
	}

	property_sheet.u3.phpage = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HPROPSHEETPAGE));

	dialogs = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*dialogs));
	if (!property_sheet.u3.phpage || !dialogs)
	{
		HeapFree(GetProcessHeap(), 0, property_sheet.u3.phpage);
		HeapFree(GetProcessHeap(), 0, dialogs);
		return E_OUTOFMEMORY;
	}

	memset(&property_sheet_page, 0, sizeof(PROPSHEETPAGEW));
	property_sheet_page.dwSize = sizeof(PROPSHEETPAGEW);
	property_sheet_page.dwFlags = PSP_DLGINDIRECT | PSP_USETITLE;

	property_sheet_page.pfnDlgProc = property_sheet_proc;

	PROPPAGEINFO page_info;
	property_page_site = HeapAlloc(GetProcessHeap(), 0, sizeof(PropertyPageSite));
	if (!property_page_site)
		goto done;

	property_page_site->IPropertyPageSite_iface.lpVtbl = &PropertyPageSiteVtbl;
	property_page_site->ref = 1;
	property_page_site->lcid = 0;

	res = IPropertyPage_SetPageSite(page, &property_page_site->IPropertyPageSite_iface);
	IPropertyPageSite_Release(&property_page_site->IPropertyPageSite_iface);
	if (FAILED(res))
		goto done;

	res = IPropertyPage_SetObjects(page, 1, ppUnk);
	res = IPropertyPage_GetPageInfo(page, &page_info);
	if (FAILED(res))
		goto done;

	dialogs[0].template.cx = MulDiv(page_info.size.cx, 4, font_width);
	dialogs[0].template.cy = MulDiv(page_info.size.cy, 8, font_height);

	property_sheet_page.u.pResource = &dialogs[0].template;
	property_sheet_page.lParam = (LPARAM)page;
	property_sheet_page.pszTitle = page_info.pszTitle;
	property_sheet.u3.phpage[property_sheet.nPages++] = CreatePropertySheetPageW(&property_sheet_page);
	PropertySheetW(&property_sheet);

done:
	HeapFree(GetProcessHeap(), 0, dialogs);
	HeapFree(GetProcessHeap(), 0, property_sheet.u3.phpage);
	return S_OK;
}
#endif
