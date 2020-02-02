//
//		SpoutDX
//
//		Send a DirectX11 shared texture
//		Receive from a Spout sender DirectX11 shared texture
//		DirectX9 not supported
//
// ====================================================================================
//		Revisions :
//
//		07.06.19	- started DirectX helper class
//		17.10.19	- For revison of SpoutCam from OpenGL to DirectX
//					  Added DirectX initialise and release functions
//					  Added support for pixel read via staging texture
//					  Added pixel copy functions
//					  Added memoryshare support
//		30.01.20	- Simplify names for Get received texture methods
//					  Revise CreateDX11Texture for user flags
//					  Add SetTextureFlags
//
// ====================================================================================
/*
	Copyright (c) 2014-2020, Lynn Jarvis. All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, 
	are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
	IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include "spoutDX.h"

spoutDX::spoutDX()
{
	// Initialize variables
	m_pd3dDevice = nullptr;
	m_pImmediateContext = nullptr;
	m_pReceivedTexture = nullptr;
	
	m_ShareHandle = NULL;
	m_pSharedTexture = nullptr;

	m_pStagingTexture = nullptr;
	m_dxShareHandle = NULL;
	m_SenderNameSetup[0] = 0;
	m_SenderName[0] = 0;
	m_dwFormat = 0;
	m_MiscFlags = 0;
	m_Width = 0;
	m_Height = 0;
	m_bUpdate = false;
	bSpoutInitialized = false;
	m_bConnected = false;
	bSpoutPanelOpened = false;
	bSpoutPanelActive = false;
	m_bUseActive = true;

}

spoutDX::~spoutDX()
{
	ReleaseSender();
	ReleaseReceiver();
}

// Initialize and prepare Directx 11
bool spoutDX::OpenDirectX11()
{
	// Quit if already initialized
	if (m_pd3dDevice != NULL)
		return true;

	SpoutLogNotice("spoutDX::OpenDirectX11()");

	// Create a DirectX 11 device
	if (!m_pd3dDevice)
		m_pd3dDevice = spoutdx.CreateDX11device();

	if (!m_pd3dDevice)
		return false;

	// Retrieve the context pointer
	m_pd3dDevice->GetImmediateContext(&m_pImmediateContext);

	return true;
}

void spoutDX::CleanupDX11()
{
	if (m_pd3dDevice != NULL) {

		SpoutLogNotice("spoutDX::CleanupDX11()");

		unsigned long refcount = 0;
		
		if(m_pStagingTexture)
			refcount = spoutdx.ReleaseDX11Texture(m_pd3dDevice, m_pStagingTexture);

		// Important to set pointer to NULL or it will crash if released again
		m_pStagingTexture = NULL;

		// 12.11.18 - To avoid memory leak with dynamic objects
		//            must always be freed, not only on exit.
		//            Device recreated for a new sender.
		refcount = spoutdx.ReleaseDX11Device(m_pd3dDevice);
		if (refcount > 0)
			SpoutLogWarning("CleanupDX11:ReleaseDX11Device - refcount = %d", refcount);

		// NULL the pointers
		m_pImmediateContext = nullptr;
		m_pd3dDevice = nullptr;

	}
}


//---------------------------------------------------------
//
// SENDER
//

bool spoutDX::CreateSender(const char* sendername, unsigned int width, unsigned int height, HANDLE sharehandle, DXGI_FORMAT format)
{
	if (spoutsender.CreateSender(sendername, width, height, sharehandle, (DWORD)format)) {
		strcpy_s(m_SenderName, 256, sendername);
		// Save width and height to test for sender size changes
		m_Width = width;
		m_Height = height;
		// Save texture details
		m_dwFormat = (DWORD)format;
		m_dxShareHandle = sharehandle;
		// Create a sender mutex for access to the shared texture
		frame.CreateAccessMutex(m_SenderName);
		// Enable frame counting so the receiver gets frame number and fps
		frame.EnableFrameCount(m_SenderName);
		return true;
	}
	return false;
}

bool spoutDX::UpdateSender(const char *sendername, unsigned int width, unsigned int height, HANDLE sharehandle, DXGI_FORMAT format)
{
	return spoutsender.UpdateSender(sendername, width, height, sharehandle, (DWORD)format);
}

bool spoutDX::LockTexture()
{
	// Get access to the shared texture
	return frame.CheckAccess();
}

void spoutDX::UnlockTexture()
{
	// Signal a new frame while the mutex is still locked
	frame.SetNewFrame();
	// Allow access to the shared texture
	frame.AllowAccess();
}

bool spoutDX::CopyTexture(ID3D11Device* pd3dDevice, ID3D11Texture2D* pTexture, HANDLE sharehandle)
{
	if (pTexture && sharehandle) {
		// Get a pointer to the shared texture (pSharedTexture) via the share handle
		ID3D11Texture2D* pSharedTexture = nullptr;
		if (spoutdx.OpenDX11shareHandle(pd3dDevice, &pSharedTexture, sharehandle)) {
			ID3D11DeviceContext* pImmediateContext = nullptr;
			pd3dDevice->GetImmediateContext(&pImmediateContext);
			if (pImmediateContext) {
				// Copy the texture to the shared texture
				// Copy during the mutex lock for sole access to the shared texture.
				pImmediateContext->CopyResource(pSharedTexture, pTexture);
				// Flush and wait until CopyResource is finished
				// so that the receiver can read this frame.
				spoutdx.FlushWait(pd3dDevice, pImmediateContext);
				return true;
			}
			else printf("No immediate context\n");
		}
		else printf("Could not open share handle\n");
	}
	else printf("No pTexture or sharehandle\n");

	return false;
}

//
// Compatible formats
//
// DX9 compatible formats
// DXGI_FORMAT_R8G8B8A8_UNORM; // default DX11 format - compatible with DX9 (28)
// DXGI_FORMAT_B8G8R8A8_UNORM; // compatible DX11 format - works with DX9 (87)
// DXGI_FORMAT_B8G8R8X8_UNORM; // compatible DX11 format - works with DX9 (88)
//
// Other formats that work with DX11 but not with DX9
// DXGI_FORMAT_R16G16B16A16_FLOAT
// DXGI_FORMAT_R16G16B16A16_SNORM
// DXGI_FORMAT_R10G10B10A2_UNORM
//
bool spoutDX::SendTexture(const char* sendername, ID3D11Device* pd3dDevice, ID3D11Texture2D* &pTexture)
{
	if (!sendername[0] || !pd3dDevice || !pTexture) {
		printf("!sendername[0] || !pDevice || !pTexture\n");
		return false;
	}

	// Get the texture details
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	pTexture->GetDesc(&desc);
	if (desc.Width == 0 || desc.Height == 0) {
		printf("zero texture with or height\n");
		return false;
	}

	// If a sender has not been initialized yet, create one
	if (!bSpoutInitialized) {

		// Save width and height to test for sender size changes
		m_Width = desc.Width;
		m_Height = desc.Height;

		// Is the application texture shared ?
		if (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) {
			// Get its sharehandle
			IDXGIResource* pOtherResource(NULL);
			if (pTexture->QueryInterface(__uuidof(IDXGIResource), (void**)&pOtherResource) != S_OK) {
				SpoutLogError("spoutDX - QueryInterface error");
				return false;
			}
			// Get the shared texture handle
			pOtherResource->GetSharedHandle(&m_dxShareHandle);
			pOtherResource->Release();
		}
		else {
			// Create a local shared texture the same size and format as the texture
			spoutdx.CreateSharedDX11Texture(pd3dDevice, m_Width, m_Height, desc.Format, &m_pSharedTexture, m_dxShareHandle);
		}

		// Create a sender, specifying the same texture format
		strcpy_s(m_SenderName, 256, sendername);
		bSpoutInitialized = spoutsender.CreateSender(m_SenderName, m_Width, m_Height, m_dxShareHandle, (DWORD)desc.Format);

		// Create a sender mutex for access to the shared texture
		frame.CreateAccessMutex(m_SenderName);

		// Enable frame counting so the receiver gets frame number and fps
		frame.EnableFrameCount(m_SenderName);

	}
	// Otherwise check for change of render size
	else if (m_Width != desc.Width || m_Height != desc.Height) {

		// Initialized but has the source texture changed size ?
		m_Width = desc.Width;
		m_Height = desc.Height;

		// Is the application texture shared ?
		if (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) {
			// TODO : IsUpdated() - must be called before SendTexture
			// Get its sharehandle
			IDXGIResource* pOtherResource(NULL);
			if (pTexture->QueryInterface(__uuidof(IDXGIResource), (void**)&pOtherResource) != S_OK) {
				SpoutLogError("spoutDX - QueryInterface error");
				return false;
			}
			// Get the shared texture handle
			pOtherResource->GetSharedHandle(&m_dxShareHandle);
			pOtherResource->Release();
		}
		else {
			// Release and re-create the local shared texture to match
			if (m_pSharedTexture) m_pSharedTexture->Release();
			spoutdx.CreateSharedDX11Texture(pd3dDevice, m_Width, m_Height, desc.Format, &m_pSharedTexture, m_dxShareHandle);
		}
	}

	// Check the sender mutex for access to the shared texture
	if (frame.CheckAccess()) {
		if (!(desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED)) {
			ID3D11DeviceContext* pImmediateContext = nullptr;
			pd3dDevice->GetImmediateContext(&pImmediateContext);
			if (pImmediateContext) {
				// Copy the texture to the local sender shared texture
				pImmediateContext->CopyResource(m_pSharedTexture, pTexture);
				// Flush and wait until CopyResource is finished
				// so that the receiver can read this frame.
				spoutdx.FlushWait(pd3dDevice, pImmediateContext);
			}
		}
		// Signal a new frame while the mutex is still locked
		frame.SetNewFrame();
		// Allow access to the shared texture
		frame.AllowAccess();
	}

	return true;
}

void spoutDX::ReleaseSender()
{
	// Release class shared texture if created
	if (m_pSharedTexture)
		m_pSharedTexture->Release();

	// Release the sender from the name list
	if (bSpoutInitialized)
		spoutsender.ReleaseSenderName(m_SenderName);

	m_pSharedTexture = nullptr;
	m_dxShareHandle = NULL;
	m_dwFormat = 0;
	m_Width = 0;
	m_Height = 0;
	m_SenderName[0] = 0;
	bSpoutInitialized = false;

}

void spoutDX::HoldFps(int fps)
{
	frame.HoldFps(fps);
}

unsigned int spoutDX::GetWidth()
{
	return m_Width;
}

unsigned int spoutDX::GetHeight()
{
	return m_Height;
}

double spoutDX::GetFps()
{
	return (frame.GetSenderFps());
}

long spoutDX::GetFrame()
{
	return (frame.GetSenderFrame());
}



//---------------------------------------------------------
//
// RECEIVER
//


// Set the sender name to connect to
void spoutDX::SetReceiverName(const char * SenderName)
{
	if (SenderName && SenderName[0]) {
		strcpy_s(m_SenderNameSetup, 256, SenderName);
		strcpy_s(m_SenderName, 256, SenderName);
	}
}


//---------------------------------------------------------
//	o Connect to a sender and inform the application to update receiving texture dimensions
//  o Receive texture details from the sender for copy to the receiving texture
bool spoutDX::ReceiveTextureData()
{
	m_bUpdate = false;

	// Initialization is recorded in this class
	// m_Width or m_Height are established when the receiver connects to a sender
	if (!IsConnected()) {
		// Create a DirectX device if not already
		if (!OpenDirectX11())
			return false;
		// Set starting values
		SetupReceiver();
	}

	// If SpoutPanel has been opened the sender name will be different
	if (CheckSpoutPanel(m_SenderName, 256)) {
		// Reset receiver setup name
		m_SenderNameSetup[0] = 0;
		ReleaseReceiver();
	}

	// Save sender name and dimensions to test for change
	char name[256];
	strcpy_s(name, 256, m_SenderName);
	unsigned int width = m_Width;
	unsigned int height = m_Height;

	// Find if the sender exists.
	// If a name has been specified, return false if not found.
	// For a null name, return the active sender name if that exists.
	// Return width, height, sharehandle and format.
	if (spoutsender.FindSender(m_SenderName, width, height, m_dxShareHandle, m_dwFormat)) {

		// There is no share handle for memoryshare mode
		if (!m_dxShareHandle)
			return false;

		// Check for sender size changes
		if (m_Width != width || m_Height != height) {
			// Release resources
			// This also resets the sender name if one was set 
			// by the application with SetReceiverName at the beginning.
			ReleaseReceiver();
		}

		// Save the sender's width and height to check for changes
		m_Width = width;
		m_Height = height;

		m_bConnected = true;

		// Set up if not initialized yet
		if (!bSpoutInitialized) {
			bSpoutInitialized = true;
			// Create a named sender mutex for access to the sender's shared texture
			frame.CreateAccessMutex(m_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(m_SenderName);
			// Tell the application that the sender has changed
			// The update flag is reset when the receiving application calls IsUpdated()
			m_bUpdate = true;
			// Return to update the receiving texture or image
			return true;
		} // end not initialized

		// Connected and intialized
		// Sender name, width, height, format and share handle have been retrieved
		// Fall through

	} // end find sender
	else {
		// There is no sender or the connected sender closed.
		ReleaseReceiver();
		// The local texture pointer will be null.
		// Let the application know.
		m_bConnected = false;
		return false;
	}

	// The application can now access and copy the sender texture
	return true;

}

bool spoutDX::LockSenderTexture()
{
	// Access the sender shared texture
	if (frame.CheckAccess()) {
		// Check if the sender has produced a new frame.
		// This function must be called within a sender mutex lock so the sender does not
		// write a frame and increment the frame count while a receiver is reading it.
		if (frame.GetNewFrame()) {
			return true;
		}
	}
	frame.AllowAccess();
	return false;
}

void spoutDX::UnlockSenderTexture()
{
	frame.AllowAccess();
}


bool spoutDX::CopySenderTexture(ID3D11Device* pd3dDevice, ID3D11Texture2D* pTexture)
{
	if (pTexture && m_dxShareHandle) {
		// Get a pointer to the sender's texture via the share handle
		ID3D11Texture2D* pSharedTexture = nullptr;
		if (spoutdx.OpenDX11shareHandle(pd3dDevice, &pSharedTexture, m_dxShareHandle)) {
			ID3D11DeviceContext* pImmediateContext = nullptr;
			pd3dDevice->GetImmediateContext(&pImmediateContext);
			if (pImmediateContext) {
				// Use the shared texture pointer for copy to the local texture.
				// Copy during the mutex lock for sole access to the shared texture.
				pImmediateContext->CopyResource(pTexture, pSharedTexture);
				// Flush and wait until CopyResource is finished
				// so that the receiver can read this frame.
				spoutdx.FlushWait(pd3dDevice, pImmediateContext);
				return true;
			}
			else printf("No immediate context\n");
		}
		else printf("Could not open share handle\n");
	}
	else printf("No pTexture or sharehandle\n");

	return false;
}


// Receive a local DX11 texture from a sender
// Optional application device
bool spoutDX::ReceiveTexture()
{
	m_bUpdate = false;

	// Initialization is recorded in this class for sender or receiver
	// m_Width or m_Height are established when the receiver connects to a sender
	if (!IsConnected()) {

		// Create a DirectX device if not already
		if (!OpenDirectX11())
				return false;
		// Set to local variables to zero
		SetupReceiver();
		// Signal the application to update the receiving texture size
		// Retrieved with a call to the IsUpdated function
		m_bUpdate = true;
		m_bConnected = true;
		return true;
	}

	// Save sender name and dimensions to test for change
	char name[256];
	strcpy_s(name, 256, m_SenderName);
	unsigned int width = m_Width;
	unsigned int height = m_Height;

	// If SpoutPanel has been opened the sender name will be different
	if (CheckSpoutPanel(m_SenderName, 256)) {
		// Reset receiver setup name
		m_SenderNameSetup[0] = 0;
		ReleaseReceiver();
	}

	// Find if the sender exists.
	// If a name has been specified, return false if not found.
	// For a null name, return the active sender name if that exists.
	// Return width, height, sharehandle and format.
	if (spoutsender.FindSender(m_SenderName, width, height, m_dxShareHandle, m_dwFormat)) {

		// There is no share handle for memoryshare mode
		if (!m_dxShareHandle)
			return false;

		// Check for sender size changes
		if (m_Width != width || m_Height != height) {
			// Release resources
			// This also resets the sender name if one was set 
			// by the application with SetReceiverName at the beginning.
			ReleaseReceiver();
		}

		// Save the sender's width and height to check for changes
		m_Width = width;
		m_Height = height;
		
		// Set up if not initialized yet
		if (!bSpoutInitialized) {
			// Create a named sender mutex for access to the sender's shared texture
			frame.CreateAccessMutex(m_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(m_SenderName);
			// Create a local texture of the same size on the application device
			// for copy from the shared texture. The two textures must have compatible DXGI formats.
			// Default shared texture format (m_dwFormat) is DXGI_FORMAT_B8G8R8A8_UNORM.
			// https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/nf-d3d11-id3d11devicecontext-copyresource
			// Create a new texture as shared so that the handle can be used if necessary
			if (CreateDX11Texture(m_pd3dDevice,
				m_Width, m_Height,
				(DXGI_FORMAT)m_dwFormat,
				D3D11_RESOURCE_MISC_SHARED,
				&m_pReceivedTexture, &m_ShareHandle)) {
					bSpoutInitialized = true;
					// Tell the application that the sender has changed
					// The update flag is reset when the receiving application calls IsUpdated()
					m_bUpdate = true;
					// Return to update the receiving pixels
					return true;
			}
			else {
				return false;
			}
		}

		// Access the sender shared texture
		if (frame.CheckAccess()) {
			// Check if the sender has produced a new frame.
			// This function must be called within a sender mutex lock so the sender does not
			// write a frame and increment the frame count while a receiver is reading it.
			if (frame.GetNewFrame()) {
				if (spoutdx.OpenDX11shareHandle(m_pd3dDevice, &m_pSharedTexture, m_dxShareHandle)) {
					// Use the shared texture pointer for copy to the local texture.
					// Copy during the mutex lock for sole access to the shared texture.
					if (m_pReceivedTexture) {
						m_pImmediateContext->CopyResource(m_pReceivedTexture, m_pSharedTexture);
						// Flush and wait until CopyResource is finished
						// so that the receiver can read this frame.
						spoutdx.FlushWait(m_pd3dDevice, m_pImmediateContext);
					}
				}
			}
			// Allow access to the shared texture
			frame.AllowAccess();
		}
		m_bConnected = true;
	} // sender exists
	else {
		// There is no sender or the connected sender closed.
		ReleaseReceiver();
		// The local texture pointer will be null.
		// Let the application know.
		m_bConnected = false;
	}

	return m_bConnected;

}

// Receive rgba pixels from a sender via DX11 staging texture
bool spoutDX::ReceiveImage(unsigned char * pData)
{
	if (!m_pImmediateContext)
		return false;

	if (!IsConnected()) {
		SetupReceiver(m_Width, m_Height);
		m_bUpdate = true;
		m_bConnected = true;
		return true;
	}

	// Set the initial width and height to globals
	// width and height are returned from the sender
	unsigned int width = m_Width;
	unsigned int height = m_Height;

	// If SpoutPanel has been opened the sender name (m_SenderName) will be different
	if (CheckSpoutPanel(m_SenderName, 256)) {
		// Reset receiver setup name
		m_SenderNameSetup[0] = 0;
		ReleaseReceiver();
	}

	// Find if the sender exists.
	// If a name has been specified, return false if not found.
	// For a null name, return the active sender name if that exists.
	// Return width, height, sharehandle and format.
	if (spoutsender.FindSender(m_SenderName, width, height, m_dxShareHandle, m_dwFormat)) {

		// Check for sender size changes
		if (m_Width != width || m_Height != height) {
			// Release resources
			// This also resets the sender name if one was set 
			// by the application with SetReceiverName at the beginning.
			ReleaseReceiver();
		}

		// Save the sender's width and height to check for changes
		m_Width = width;
		m_Height = height;

		// Set up if not initialized yet
		if (!bSpoutInitialized) {

			// Create a named sender mutex for access to the sender's shared texture
			frame.CreateAccessMutex(m_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(m_SenderName);

			// If a staging texture has not been created or is a different size,
			// create a new one for copy from the shared texture to pixels.
			// The two textures must have compatible DXGI formats.
			// The sender format is returned in m_dwFormat
			if (!CheckStagingTexture(width, height))
				return false;

			bSpoutInitialized = true;

			// Tell the application that the sender has changed
			// The update flag is reset when the receiving application calls IsUpdated()
			m_bUpdate = true;

			// Return to update the receiving pixels
			return true;

		}

		// Setup is done - ready to access the sender shared texture
		if (frame.CheckAccess()) {
			// Check if the sender has produced a new frame.
			// This function must be called within a sender mutex lock so the sender does not
			// write a frame and increment the frame count while a receiver is reading it.
			if (frame.GetNewFrame()) {
				// Get a pointer to the sender's shared texture
				if (spoutdx.OpenDX11shareHandle(m_pd3dDevice, &m_pSharedTexture, m_dxShareHandle)) {
					// Use the shared texture pointer for copy to the local staging texture.
					// Copy during the mutex lock for sole access to the shared texture.
					if (m_pStagingTexture) {
						m_pImmediateContext->CopyResource(m_pStagingTexture, m_pSharedTexture);
						// Flush and wait until CopyResource is finished
						// There will be no other commands in he queue for this device
						spoutdx.FlushWait(m_pd3dDevice, m_pImmediateContext);
						// Now we have a staging texture and can map it
						// to retrieve the bgra pixels into the user buffer pData
						ReadRGBApixels(m_pStagingTexture, pData, m_Width, m_Height, false);
					}
				}
			}
			// Allow access to the shared texture
			frame.AllowAccess();
		}
		m_bConnected = true;
	} // sender exists
	else {
		// There is no sender or the connected sender closed.
		ReleaseReceiver();
		// The local texture pointer will be null.
		// Let the application know.
		m_bConnected = false;
	}

	return m_bConnected;

}

// Receive rgba pixels from a sender via DX11 staging texture to an rgb buffer of variable size
bool spoutDX::ReceiveRGBAimage(unsigned char * pData, unsigned int sourceWidth, unsigned int sourceHeight, bool bInvert)
{
	if (!m_pImmediateContext)
		return false;

	if (!IsConnected()) {
		SetupReceiver(m_Width, m_Height);
		m_bUpdate = true;
		m_bConnected = true;
		return true;
	}

	// Set the initial width and height to globals
	// width and height are returned from the sender
	unsigned int width = m_Width;
	unsigned int height = m_Height;

	// If SpoutPanel has been opened the sender name (m_SenderName) will be different
	if (CheckSpoutPanel(m_SenderName, 256)) {
		// Reset receiver setup name
		m_SenderNameSetup[0] = 0;
		ReleaseReceiver();
	}

	// Find if the sender exists.
	// If a name has been specified, return false if not found.
	// For a null name, return the active sender name if that exists.
	// Return width, height, sharehandle and format.
	if (spoutsender.FindSender(m_SenderName, width, height, m_dxShareHandle, m_dwFormat)) {

		// Check for sender size changes
		if (m_Width != width || m_Height != height) {
			// Release resources
			// This also resets the sender name if one was set 
			// by the application with SetReceiverName at the beginning.
			ReleaseReceiver();
		}

		// Save the sender's width and height to check for changes
		m_Width = width;
		m_Height = height;

		// Set up if not initialized yet
		if (!bSpoutInitialized) {
			// Create a named sender mutex for access to the sender's shared texture
			frame.CreateAccessMutex(m_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(m_SenderName);

			// If a staging texture has not been created or is a different size,
			// create a new one for copy from the shared texture to pixels.
			// The two textures must have compatible DXGI formats.
			// The sender format is returned in m_dwFormat
			if (!CheckStagingTexture(width, height))
				return false;

			bSpoutInitialized = true;
			// Tell the application that the sender has changed
			// The update flag is reset when the receiving application calls IsUpdated()
			m_bUpdate = true;
			// Return to update the receiving pixels
			return true;
		}

		// Access the sender shared texture
		if (frame.CheckAccess()) {
			// Check if the sender has produced a new frame.
			// This function must be called within a sender mutex lock so the sender does not
			// write a frame and increment the frame count while a receiver is reading it.
			if (frame.GetNewFrame()) {
				if (spoutdx.OpenDX11shareHandle(m_pd3dDevice, &m_pSharedTexture, m_dxShareHandle)) {
					// Use the shared texture pointer for copy to the local staging texture.
					// Copy during the mutex lock for sole access to the shared texture.
					if (m_pStagingTexture) {
						m_pImmediateContext->CopyResource(m_pStagingTexture, m_pSharedTexture);
						// Flush and wait until CopyResource is finished
						spoutdx.FlushWait(m_pd3dDevice, m_pImmediateContext);
						// Now we have an rgba staging texture and can map it to retrieve the pixels
						ReadRGBApixels(m_pStagingTexture, pData, sourceWidth, sourceHeight, bInvert);
					}
				}
			}
			// Allow access to the shared texture
			frame.AllowAccess();
		}
		m_bConnected = true;
	} // sender exists
	else {
		// There is no sender or the connected sender closed.
		ReleaseReceiver();
		// The local texture pointer will be null.
		// Let the application know.
		m_bConnected = false;
	}

	return m_bConnected;

}


// Receive pixels from a sender via DX11 staging texture to an rgb buffer of variable size
bool spoutDX::ReceiveRGBimage(unsigned char * pData, unsigned int sourceWidth, unsigned int sourceHeight, bool bInvert)
{
	if (!m_pImmediateContext)
		return false;

	if (!IsConnected()) {
		SetupReceiver(m_Width, m_Height);
		m_bUpdate = true;
		m_bConnected = true;
		return true;
	}

	// Set the initial width and height to globals
	// width and height are returned from the sender
	unsigned int width = m_Width;
	unsigned int height = m_Height;

	// If SpoutPanel has been opened the sender name (m_SenderName) will be different
	if (CheckSpoutPanel(m_SenderName, 256)) {
		// Reset receiver setup name
		m_SenderNameSetup[0] = 0;
		ReleaseReceiver();
	}

	// Find if the sender exists.
	// If a name has been specified, return false if not found.
	// For a null name, return the active sender name if that exists.
	// Return width, height, sharehandle and format.
	if (spoutsender.FindSender(m_SenderName, width, height, m_dxShareHandle, m_dwFormat)) {

		// Check for sender size changes
		if (m_Width != width || m_Height != height) {
			// Release resources
			// This also resets the sender name if one was set 
			// by the application with SetReceiverName at the beginning.
			ReleaseReceiver();
		}

		// Save the sender's width and height to check for changes
		m_Width = width;
		m_Height = height;

		// Set up if not initialized yet
		if (!bSpoutInitialized) {
			// Create a named sender mutex for access to the sender's shared texture
			frame.CreateAccessMutex(m_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(m_SenderName);

			// If a staging texture has not been created or is a different size,
			// create a new one for copy from the shared texture to pixels.
			// The two textures must have compatible DXGI formats.
			// The sender format is returned in m_dwFormat
			if (!CheckStagingTexture(width, height))
				return false;

			bSpoutInitialized = true;
			// Tell the application that the sender has changed
			// The update flag is reset when the receiving application calls IsUpdated()
			m_bUpdate = true;
			// Return to update the receiving pixels
			return true;
		}

		// Access the sender shared texture
		if (frame.CheckAccess()) {
			// Check if the sender has produced a new frame.
			// This function must be called within a sender mutex lock so the sender does not
			// write a frame and increment the frame count while a receiver is reading it.
			if (frame.GetNewFrame()) {
				if (spoutdx.OpenDX11shareHandle(m_pd3dDevice, &m_pSharedTexture, m_dxShareHandle)) {
					// Use the shared texture pointer for copy to the local staging texture.
					// Copy during the mutex lock for sole access to the shared texture.
					if (m_pStagingTexture) {
						m_pImmediateContext->CopyResource(m_pStagingTexture, m_pSharedTexture);
						// Flush and wait until CopyResource is finished
						spoutdx.FlushWait(m_pd3dDevice, m_pImmediateContext);
						// Now we have an rgba staging texture and can map it to retrieve the pixels
						ReadRGBpixels(m_pStagingTexture, pData, sourceWidth, sourceHeight, bInvert);
					}
				}
			}
			// Allow access to the shared texture
			frame.AllowAccess();
		}
		m_bConnected = true;
	} // sender exists
	else {
		// There is no sender or the connected sender closed.
		ReleaseReceiver();
		// The local texture pointer will be null.
		// Let the application know.
		m_bConnected = false;
	}

	return m_bConnected;

}

// Receive pixels from a sender via shared memory and receiving buffer of variable format and size
// TODO : not working yet
bool spoutDX::ReceiveMemoryImage(const char* sendername, 
								 unsigned char* pixels,
								 unsigned int destwidth, unsigned int destheight,
								 GLenum glFormat, bool bInvert)
{
	if (!m_pImmediateContext)
		return false;

	if (!IsConnected()) {
		SetupReceiver(m_Width, m_Height);
		m_bUpdate = true;
		m_bConnected = true;
		return true;
	}

	// Set the initial width and height to globals
	// width and height are returned from the sender
	unsigned int width = m_Width;
	unsigned int height = m_Height;

	// If SpoutPanel has been opened the sender name (m_SenderName) will be different
	if (CheckSpoutPanel(m_SenderName, 256)) {
		// Reset receiver setup name
		m_SenderNameSetup[0] = 0;
		ReleaseReceiver();
	}

	// Find if the sender exists.
	// If a name has been specified, return false if not found.
	// For a null name, return the active sender name if that exists.
	// Return width, height, sharehandle and format.
	if (spoutsender.FindSender(m_SenderName, width, height, m_dxShareHandle, m_dwFormat)) {

		// Check for sender size changes
		if (m_Width != width || m_Height != height) {
			// Release resources
			// This also resets the sender name if one was set 
			// by the application with SetReceiverName at the beginning.
			ReleaseReceiver();
		}

		// Save the sender's width and height to check for changes
		m_Width = width;
		m_Height = height;

		// Set up if not initialized yet
		if (!bSpoutInitialized) {
			// Create a named sender mutex for access to the sender's shared texture
			frame.CreateAccessMutex(m_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(m_SenderName);
			bSpoutInitialized = true;
			// Tell the application that the sender has changed
			// The update flag is reset when the receiving application calls IsUpdated()
			m_bUpdate = true;
			// Return to update the receiving pixels
			return true;
		}

		// m_Width and m_Height are set for the sender buffer size
		if (!ReceiveMemory(m_SenderName, pixels, destwidth, destheight, glFormat, bInvert))
			return false;

		m_bConnected = true;
	} // sender exists
	else {
		// There is no sender or the connected sender closed.
		ReleaseReceiver();
		// The local texture pointer will be null.
		// Let the application know.
		m_bConnected = false;
	}

	return m_bConnected;

}



//
// Read shared memory to image pixels
// rgba, bgra, rgb, bgr destination buffers supported
// Most efficient if the receiving buffer is rgba
// Invert currently not used.
// Open the sender memory map and close it afterwards for a read,
// so that the receiver does not retain a handle to the shared memory.
//
// Destination is what is passed
// Source is the sender memoryshare pixels
//
bool spoutDX::ReceiveMemory(const char* sendername,
	unsigned char* pixels,
	unsigned int destwidth,
	unsigned int destheight,
	GLenum glFormat, bool bInvert)
{
	if (!pixels)
		return false;

	if (!memoryshare.OpenSenderMemory(sendername))
		return false;

	if (!IsConnected()) {
		SetupReceiver(m_Width, m_Height);
		m_bUpdate = true;
		m_bConnected = true;
		return true;
	}

	// Buffere is m_Width*m_Height*4
	unsigned char* pBuffer = memoryshare.LockSenderMemory();
	if (!pBuffer) {
		memoryshare.CloseSenderMemory();
		// printf("spoutDX::ReceiveMemory - no buffer lock\n");
		// SpoutLogWarning("spoutDX::ReceiveMemory - no buffer lock");
		return false;
	}

	// Size of the memory is unknown - the sender has to be queried to get that
	// printf("ReceiveMemory - source = %d (%dx%d), dest = %d (%dx%d) - format = %d - 0x%x\n", pBuffer, m_Width, m_Height, pixels, destwidth, destheight, glFormat, glFormat);

	// Query a new frame and read pixels while the buffer is locked
	if (frame.GetNewFrame()) {

		// TODO : resample for rgba
		// Read pixels from sender shared memory
		if (glFormat == GL_RGBA) {
			// printf("spoutcopy.rgba\n");
			spoutcopy.CopyPixels(pBuffer, pixels, destwidth, destheight, GL_RGBA, bInvert);
		}
		else if (glFormat == 0x80E1) { // GL_BGRA_EXT if supported
			// printf("spoutcopy.bgra\n");
			spoutcopy.rgba2bgra((void *)pBuffer, (void *)pixels, destwidth, destheight, bInvert);
		}
		else if (glFormat == 0x80E0) { // GL_BGR_EXT if supported
			// printf("spoutcopy.bgr\n");
			// For SpoutCam with varying buffer size
			if (destwidth != m_Width || destheight != m_Height) {
				// printf("    rgba2bgrResample\n");
				spoutcopy.rgba2bgrResample(pBuffer, pixels,
											m_Width, m_Height, m_Width*m_Height*4,
											destwidth, destheight, bInvert);
			}
			else {
				// printf("    rgba2bgr\n");
				spoutcopy.rgba2bgr(pBuffer, pixels, destwidth, destheight, bInvert);
			}
		}
		else if (glFormat == GL_RGB) {
			// TODO : resample
			// printf("    rgba2rgb\n");
			spoutcopy.rgba2rgb((void *)pBuffer, (void *)pixels, destwidth, destheight, bInvert);
		}
	}

	memoryshare.UnlockSenderMemory();

	// Close the memory map handle so the sender can close the map
	memoryshare.CloseSenderMemory();

	return true;

}

// Return the sender texture copy
ID3D11Texture2D* spoutDX::GetSenderTexture()
{
	return m_pReceivedTexture;
}

// Return sender share handle
HANDLE spoutDX::GetSenderHandle()
{
	return m_dxShareHandle;
}

DXGI_FORMAT spoutDX::GetSenderFormat()
{
	if(m_dwFormat == 0)
		return DXGI_FORMAT_B8G8R8A8_UNORM; // default;
	else
		return (DXGI_FORMAT)m_dwFormat;
}

// Set misc creation flags for receved texture copy
void spoutDX::SetTextureFlags(UINT miscflags) {
	m_MiscFlags = miscflags;
}

void spoutDX::SetupReceiver(unsigned int width, unsigned int height)
{
	// Receiver will use the active sender unless the user 
	// has specified a sender to connect to using SetReceiverName
	if (!m_SenderNameSetup[0]) {
		m_SenderName[0] = 0;
		m_bUseActive = true;
	}

	// Record details for subsequent functions
	m_Width = width;
	m_Height = height;
	m_bUpdate = false;
	m_bConnected = false;

}

void spoutDX::ReleaseReceiver()
{
	// Release the local texture for texture copy
	if (m_pReceivedTexture)
		m_pReceivedTexture->Release();
	m_pReceivedTexture = nullptr;
	m_ShareHandle = NULL;

	// Release the local staging texture for pixel copy
	if (m_pStagingTexture)
		m_pStagingTexture->Release();
	m_pStagingTexture = nullptr;

	// Close the named access mutex and frame counting semaphore.
	frame.CloseAccessMutex();
	frame.CleanupFrameCount();

	// Restore the sender name if one was specified
	// by the application in SetReceiverName
	if(m_SenderNameSetup[0])
		strcpy_s(m_SenderName, 256, m_SenderNameSetup);

	// Initialize again when a sender is found
	bSpoutInitialized = false;
	m_bUpdate = false;

}

bool spoutDX::IsUpdated()
{
	bool bRet = m_bUpdate;
	// Reset the update flag
	m_bUpdate = false;
	return bRet;
}

bool spoutDX::IsConnected()
{
	return m_bConnected;
}

//---------------------------------------------------------
const char * spoutDX::GetSenderName()
{
	return m_SenderName;
}

unsigned int spoutDX::GetSenderWidth()
{
	return m_Width;
}

unsigned int spoutDX::GetSenderHeight()
{
	return m_Height;

}

double spoutDX::GetSenderFps()
{
	return frame.GetSenderFps();
}

long spoutDX::GetSenderFrame()
{
	return frame.GetSenderFrame();
}

int spoutDX::GetSenderCount()
{
	std::set<std::string> SenderNameSet;
	if (spoutsender.GetSenderNames(&SenderNameSet)) {
		return((int)SenderNameSet.size());
	}
	return 0;
}

// Get a sender name given an index into the sender names set
bool spoutDX::GetSender(int index, char* sendername, int sendernameMaxSize)
{
	std::set<std::string> SenderNameSet;
	std::set<std::string>::iterator iter;
	std::string namestring;
	char name[256];
	int i;

	if (spoutsender.GetSenderNames(&SenderNameSet)) {
		if (SenderNameSet.size() < (unsigned int)index) {
			return false;
		}
		i = 0;
		for (iter = SenderNameSet.begin(); iter != SenderNameSet.end(); iter++) {
			namestring = *iter; // the name string
			strcpy_s(name, 256, namestring.c_str()); // the 256 byte name char array
			if (i == index) {
				strcpy_s(sendername, sendernameMaxSize, name); // the passed name char array
				break;
			}
			i++;
		}
		return true;
	}
	return false;
}

bool spoutDX::GetActiveSender(char* Sendername)
{
	return spoutsender.GetActiveSender(Sendername);
}

bool spoutDX::SetActiveSender(const char* Sendername)
{
	return spoutsender.SetActiveSender(Sendername);
}


//---------------------------------------------------------
//
// COMMON
//

void spoutDX::DisableFrameCount()
{
	frame.DisableFrameCount();
}

bool spoutDX::IsFrameCountEnabled()
{
	return frame.IsFrameCountEnabled();
}

bool spoutDX::GetSenderInfo(const char* sendername, unsigned int &width, unsigned int &height, HANDLE &dxShareHandle, DWORD &dwFormat)
{
	return spoutsender.GetSenderInfo(sendername, width, height, dxShareHandle, dwFormat);
}

bool spoutDX::GetDX9()
{
	DWORD dwDX9 = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "DX9", &dwDX9);
	return (dwDX9 == 1);
}

bool spoutDX::GetMemoryShareMode()
{
	bool bRet = false;
	DWORD dwMem = 0;
	if (ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\Spout", "MemoryShare", &dwMem)) {
		bRet = (dwMem == 1);
	}
	return bRet;
}

int spoutDX::GetNumAdapters()
{
	return spoutdx.GetNumAdapters();
}

bool spoutDX::GetAdapterName(int index, char *adaptername, int maxchars)
{
	return spoutdx.GetAdapterName(index, adaptername, maxchars);
}

int spoutDX::GetAdapter()
{
	return spoutdx.GetAdapter();
}

bool spoutDX::SetAdapter(int index)
{
	if (spoutdx.SetAdapter(index)) {
		return true;
	}
	SpoutLogError("spoutDX::SetAdapter(%d) failed", index);
	spoutdx.SetAdapter(-1); // make sure globals are reset to default
	return false;
}

int spoutDX::GetMaxSenders()
{
	return(spoutsender.GetMaxSenders());
}

void spoutDX::SetMaxSenders(int maxSenders)
{
	spoutsender.SetMaxSenders(maxSenders);
}



//
// PRIVATE
//

//
// Create a new texture
//
// miscflags
// 0 - default, not shared
// D3D11_RESOURCE_MISC_SHARED - this texture will be shared
// D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX - keyed mutex associated with the texture
//
bool spoutDX::CreateDX11Texture(ID3D11Device* pd3dDevice,
	unsigned int width,
	unsigned int height,
	DXGI_FORMAT format,
	UINT miscflags,
	ID3D11Texture2D** pSharedTexture, HANDLE* pShareHandle)
{
	ID3D11Texture2D* pTexture = nullptr;
	HANDLE sharehandle = NULL;

	// Use the format passed in
	// If that is zero, use the default format
	DXGI_FORMAT texformat = DXGI_FORMAT_B8G8R8A8_UNORM;
	if (format != 0)
		texformat = format;

	// If m_MiscFlags has been set by the application, use it
	// Otherwise use the flags passed in
	UINT MiscFlags = miscflags;
	if (m_MiscFlags != 0)
		MiscFlags = m_MiscFlags;

	if (pd3dDevice == NULL) {
		SpoutLogError("spoutDX::CreateDX11Texture NULL device");
		return false;
	}

	// SpoutLogNotice("spoutDX::CreateDX11Texture");
	// SpoutLogNotice("    pDevice = 0x%Ix, width = %d, height = %d, format = %d", (intptr_t)pd3dDevice, width, height, format);

	pTexture = *pSharedTexture; // The texture pointer

	// Release the texture if it already exists
	if (pTexture) spoutdx.ReleaseDX11Texture(pd3dDevice, pTexture);
	pTexture = nullptr;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = texformat;
	// Default sampler mode, with no anti-aliasing
	desc.SampleDesc.Quality = 0;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = MiscFlags;

	HRESULT res = pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture);

	if (res != S_OK) {
		char tmp[256];
		sprintf_s(tmp, 256, "spoutDX::CreateSharedDX11Texture ERROR - [0x%x] : ", res);
		switch (res) {
		case D3DERR_INVALIDCALL:
			strcat_s(tmp, 256, "D3DERR_INVALIDCALL");
			break;
		case E_INVALIDARG:
			strcat_s(tmp, 256, "E_INVALIDARG");
			break;
		case E_OUTOFMEMORY:
			strcat_s(tmp, 256, "E_OUTOFMEMORY");
			break;
		default:
			strcat_s(tmp, 256, "Unlisted error");
			break;
		}
		SpoutLogError("%s", tmp);
		return false;
	}

	// TODO : create NT sharehandle
	// The received DX11 texture is created OK
	// was the texture created as shared ?
	if (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) {
		// Get the texture share handle by querying the resource
		// for the IDXGIResource interface and then calling GetSharedHandle.
		IDXGIResource* pOtherResource(NULL);
		if (pTexture->QueryInterface(__uuidof(IDXGIResource), (void**)&pOtherResource) == S_OK) {
			// Get the shared texture handle
			pOtherResource->GetSharedHandle(&sharehandle);
			pOtherResource->Release();
		}
	}

	*pSharedTexture = pTexture;
	if(pShareHandle) *pShareHandle = sharehandle;

	SpoutLogNotice("    pTexture = 0x%Ix", pTexture);

	return true;

}


// Create a DirectX 11 staging texture for read and write
bool spoutDX::CreateDX11StagingTexture(ID3D11Device* pd3dDevice,
	unsigned int width,
	unsigned int height,
	DXGI_FORMAT format,
	ID3D11Texture2D** pStagingTexture)
{
	ID3D11Texture2D* pTexture = NULL;
	if (pd3dDevice == NULL) return false;

	pTexture = *pStagingTexture; // The texture pointer
	if (pTexture) {
		pTexture->Release();
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;

	HRESULT res = pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture);

	if (res != S_OK) {
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476174%28v=vs.85%29.aspx
		char tmp[256];
		sprintf_s(tmp, 256, "spoutDirectX::CreateDX11StagingTexture ERROR : [0x%x] : ", res);
		switch (res) {
		case D3DERR_INVALIDCALL:
			strcat_s(tmp, 256, "D3DERR_INVALIDCALL");
			break;
		case E_INVALIDARG:
			strcat_s(tmp, 256, "E_INVALIDARG");
			break;
		case E_OUTOFMEMORY:
			strcat_s(tmp, 256, "E_OUTOFMEMORY");
			break;
		default:
			strcat_s(tmp, 256, "Unlisted error");
			break;
		}
		SpoutLogFatal("%s", tmp);
		return false;
	}

	*pStagingTexture = pTexture;

	return true;

}

//
// Create a new global staging texture if it has changed size or does not exist yet
// Required format must have been established already
//
bool spoutDX::CheckStagingTexture(unsigned int width, unsigned int height)
{
	D3D11_TEXTURE2D_DESC desc = { 0 };

	if (m_pStagingTexture) {
		m_pStagingTexture->GetDesc(&desc);
		if (desc.Width != width || desc.Height != height) {
			m_pStagingTexture->Release();
			m_pStagingTexture = NULL;
		}
		else
			return true;
	}

	if (!m_pStagingTexture) {
		if (CreateDX11StagingTexture(m_pd3dDevice, width, height, (DXGI_FORMAT)m_dwFormat, &m_pStagingTexture)) {
			return true;
		}
	}

	return false;
}


//
// COPY FROM A DX11 STAGING TEXTURE TO A USER RGBA PIXEL BUFFER
//
bool spoutDX::ReadRGBApixels(ID3D11Texture2D* pStagingTexture,
	unsigned char* pixels, unsigned int width,
	unsigned int height, bool bInvert)
{
	if (!m_pImmediateContext)
		return false;

	if (!pixels)
		return false;

	// if (width != m_Width || height != m_Height)
		// return false;

	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	HRESULT hr;
	void * dataPointer = nullptr;

	// Map the resource so we can access the pixels
	hr = m_pImmediateContext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
	if (SUCCEEDED(hr)) {
		// Get a pointer to the staging texture data
		dataPointer = mappedSubResource.pData;
		// Copy the the staging texture to the user pixel buffer
		// TODO - formats (texture is bgra)
		if (width != m_Width || height != m_Height) {
			spoutcopy.rgba2rgbaResample((unsigned char *)dataPointer, // source - staging texture
				pixels,				// dest - SpoutCam image
				m_Width, m_Height,	// source size - staging texture size
				mappedSubResource.RowPitch, // source line pitch
				width, height,		// dest size - SpoutCam size
				bInvert);
		}
		else {
			spoutcopy.CopyPixels((unsigned char *)dataPointer, pixels, width, height, GL_RGBA, bInvert);
		}
		m_pImmediateContext->Unmap(m_pStagingTexture, 0);
		return true;
	} // endif DX11 map OK

	return false;
} // end ReadDX11pixels


//
// COPY FROM A DX11 STAGING TEXTURE TO A USER RGB or BGR PIXEL BUFFER OF VARIABLE SIZE
//
bool spoutDX::ReadRGBpixels(ID3D11Texture2D* pStagingTexture,
	unsigned char* pixels, unsigned int sourcewidth,
	unsigned int sourceheight, bool bInvert)
{
	if (!m_pImmediateContext)
		return false;

	if (!pixels)
		return false;

	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	HRESULT hr;
	void * dataPointer = nullptr;

	// Map the resource so we can access the pixels
	// Size is m_Width, m_Height 
	hr = m_pImmediateContext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
	if (SUCCEEDED(hr)) {

		// Get a pointer to the staging texture data
		dataPointer = mappedSubResource.pData;

		// Write the the staging texture to the user pixel buffer
		// The same functions are suitable for bgra to bgr
		// NOTE : the rgba staging texture can have a pitch greater that w*h*4
		if (sourcewidth != m_Width || sourceheight != m_Height) {
			spoutcopy.rgba2rgbResample((unsigned char *)dataPointer, // source - staging texture
										pixels,				// dest - output buffer
										m_Width, m_Height,	// source size - staging texture size
										mappedSubResource.RowPitch, // staging texture pitch
										sourcewidth, sourceheight,	// different dest size e.g for SpoutCam
										bInvert);
		}
		else {
			spoutcopy.rgba2rgb(dataPointer, (void *)pixels, m_Width, m_Height, bInvert);
		}

		m_pImmediateContext->Unmap(m_pStagingTexture, 0);
		return true;
	} // endif DX11 map OK

	return false;
} // end ReadRGBpixels


//
// The following functions are adapted from equivalents in SpoutSDK.cpp
// for applications not using the entire Spout SDK.
//

//
// Check whether SpoutPanel opened and return the new sender name
//
bool spoutDX::CheckSpoutPanel(char *sendername, int maxchars)
{
	// If SpoutPanel has been activated, test if the user has clicked OK
	if (bSpoutPanelOpened) { // User has activated spout panel

		SharedTextureInfo TextureInfo;
		HANDLE hMutex = NULL;
		DWORD dwExitCode;
		char newname[256];
		bool bRet = false;

		// Must find the mutex to signify that SpoutPanel has opened
		// and then wait for the mutex to close
		hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

		// Has it been activated 
		if (!bSpoutPanelActive) {
			// If the mutex has been found, set the active flag true and quit
			// otherwise on the next round it will test for the mutex closed
			if (hMutex) bSpoutPanelActive = true;
		}
		else if (!hMutex) { // It has now closed
			bSpoutPanelOpened = false; // Don't do this part again
			bSpoutPanelActive = false;
			// call GetExitCodeProcess() with the hProcess member of SHELLEXECUTEINFO
			// to get the exit code from SpoutPanel
			if (g_ShExecInfo.hProcess) {
				GetExitCodeProcess(g_ShExecInfo.hProcess, &dwExitCode);
				// Only act if exit code = 0 (OK)
				if (dwExitCode == 0) {
					// SpoutPanel has been activated and OK clicked
					// Test the active sender which should have been set by SpoutPanel
					newname[0] = 0;
					if (!spoutsender.GetActiveSender(newname)) {
						// Otherwise the sender might not be registered.
						// SpoutPanel always writes the selected sender name to the registry.
						if (ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "Sendername", newname)) {
							// Register the sender if it exists
							if (newname[0] != 0) {
								if (spoutsender.getSharedInfo(newname, &TextureInfo)) {
									// Register in the list of senders and make it the active sender
									spoutsender.RegisterSenderName(newname);
									spoutsender.SetActiveSender(newname);
								}
							}
						}
					}
					// Now do we have a valid sender name ?
					if (newname[0] != 0) {
						// Pass back the new name
						strcpy_s(sendername, maxchars, newname);
						bRet = true;
					} // endif valid sender name
				} // endif SpoutPanel OK
			} // got the exit code
		} // endif no mutex so SpoutPanel has closed
		// If we opened the mutex, close it now or it is never released
		if (hMutex) CloseHandle(hMutex);
		return bRet;
	} // SpoutPanel has not been opened

	return false;

}

//
// Pop up SpoutPanel to allow the user to select a sender
// activated by RH click in this application
//
void spoutDX::SelectSender()
{
	HANDLE hMutex1 = NULL;
	HMODULE module = NULL;
	char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH];

	// The selected sender is then the "Active" sender and this receiver switches to it.
	// If Spout is not installed, SpoutPanel.exe has to be in the same folder
	// as this executable. This rather complicated process avoids having to use a dialog
	// which causes problems with host GUI messaging.

	// First find if there has been a Spout installation >= 2.002 with an install path for SpoutPanel.exe
	if (!ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "InstallPath", path)) {
		// Path not registered so find the path of the host program
		// where SpoutPanel should have been copied
		module = GetModuleHandle(NULL);
		GetModuleFileNameA(module, path, MAX_PATH);
		_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
		_makepath_s(path, MAX_PATH, drive, dir, "SpoutPanel", ".exe");
		// Does SpoutPanel.exe exist in this path ?
		if (!PathFileExistsA(path)) {
			// Try the current working directory
			if (_getcwd(path, MAX_PATH)) {
				strcat_s(path, MAX_PATH, "\\SpoutPanel.exe");
				// Does SpoutPanel exist here?
				if (!PathFileExistsA(path)) {
					SpoutLogWarning("spoutDX::SelectSender - SpoutPanel path not found");
					return;
				}
			}
		}
	}

	// Check whether the panel is already running
	// Try to open the application mutex.
	hMutex1 = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");
	if (!hMutex1) {
		// No mutex, so not running, so can open it
		// Use ShellExecuteEx so we can test its return value later
		ZeroMemory(&g_ShExecInfo, sizeof(g_ShExecInfo));
		g_ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		g_ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		g_ShExecInfo.hwnd = NULL;
		g_ShExecInfo.lpVerb = NULL;
		g_ShExecInfo.lpFile = (LPCSTR)path;
		g_ShExecInfo.lpDirectory = NULL;
		g_ShExecInfo.nShow = SW_SHOW;
		g_ShExecInfo.hInstApp = NULL;
		ShellExecuteExA(&g_ShExecInfo);
		Sleep(125); // allow time for SpoutPanel to open nominally 0.125s
		//
		// The flag "bSpoutPanelOpened" is set here to indicate that the user
		// has opened the panel to select a sender. This flag is local to 
		// this process so will not affect any other receiver instance
		// Then when the selection panel closes, sender name is tested
		//
		bSpoutPanelOpened = true;
	}
	else {
		// The mutex exists, so another instance is already running.
		// Find the SpoutPanel window and bring it to the top.
		// SpoutPanel is opened as topmost anyway but pop it to
		// the front in case anything else has stolen topmost.
		HWND hWnd = FindWindowA(NULL, (LPCSTR)"SpoutPanel");
		if (hWnd && IsWindow(hWnd)) {
			SetForegroundWindow(hWnd);
			// prevent other windows from hiding the dialog
			// and open the window wherever the user clicked
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		}
		else if (path[0]) {
			// If the window was not found but the mutex exists
			// and SpoutPanel is installed, it has crashed.
			// Terminate the process and the mutex or the mutex will remain
			// and SpoutPanel will not be started again.
			PROCESSENTRY32 pEntry;
			pEntry.dwSize = sizeof(pEntry);
			bool done = false;
			// Take a snapshot of all processes and threads in the system
			HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
			if (hProcessSnap == INVALID_HANDLE_VALUE) {
				SpoutLogError("spoutDX::OpenSpoutPanel - CreateToolhelp32Snapshot error");
			}
			else {
				// Retrieve information about the first process
				BOOL hRes = Process32First(hProcessSnap, &pEntry);
				if (!hRes) {
					SpoutLogError("spoutDX::OpenSpoutPanel - Process32First error");
					CloseHandle(hProcessSnap);
				}
				else {
					// Look through all processes
					while (hRes && !done) {
						int value = _tcsicmp(pEntry.szExeFile, _T("SpoutPanel.exe"));
						if (value == 0) {
							HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
							if (hProcess != NULL) {
								// Terminate SpoutPanel and it's mutex
								TerminateProcess(hProcess, 9);
								CloseHandle(hProcess);
								done = true;
							}
						}
						if (!done)
							hRes = Process32Next(hProcessSnap, &pEntry); // Get the next process
						else
							hRes = NULL; // found SpoutPanel
					}
					CloseHandle(hProcessSnap);
				}
			}
			// Now SpoutPanel will start the next time the user activates it
		} // endif SpoutPanel crashed
	} // endif SpoutPanel already open

	// If we opened the mutex, close it now or it is never released
	if (hMutex1) CloseHandle(hMutex1);

	return;

} // end SelectSender






