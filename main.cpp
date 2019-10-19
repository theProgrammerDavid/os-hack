//------------------------------------------------------------------------
// EnrollSystemPoolWithCallback.cpp : Entry point for the application.
//
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <winbio.h>
#include <vector>
#include <iostream>
std::vector< WINBIO_IDENTITY>students;

int WinBioExit(WINBIO_SESSION_HANDLE &sessionHandle) {
	if (sessionHandle != NULL)
	{
		WinBioCloseSession(sessionHandle);
		sessionHandle = NULL;
	}

	return 0;
}

int WinExit(HANDLE &h) {
	WinBioCloseSession(h);
	h = NULL;
}

HRESULT EnrollSysPoolWithCallback(
	BOOL bCancel,
	BOOL bDiscard,
	WINBIO_BIOMETRIC_SUBTYPE subFactor);

VOID CALLBACK EnrollCaptureCallback(
	__in_opt PVOID EnrollCallbackContext,
	__in HRESULT OperationStatus,
	__in WINBIO_REJECT_DETAIL RejectDetail);

typedef struct _ENROLL_CALLBACK_CONTEXT {
	WINBIO_SESSION_HANDLE SessionHandle;
	WINBIO_UNIT_ID UnitId;
	WINBIO_BIOMETRIC_SUBTYPE SubFactor;
} ENROLL_CALLBACK_CONTEXT, *PENROLL_CALLBACK_CONTEXT;



HRESULT Verify(WINBIO_BIOMETRIC_SUBTYPE subFactor)
{
	HRESULT hr = S_OK;
	WINBIO_SESSION_HANDLE sessionHandle = NULL;
	WINBIO_UNIT_ID unitId = 0;
	WINBIO_REJECT_DETAIL rejectDetail = 0;
	WINBIO_IDENTITY identity = { 0 };
	BOOLEAN match = FALSE;

	// Find the identity of the user.
	/*hr = GetCurrentUserIdentity(&identity);
	if (FAILED(hr))
	{
		wprintf_s(L"\n User identity not found. hr = 0x%x\n", hr);
		goto e_Exit;
	}
	*/
	// Connect to the system pool. 
	hr = WinBioOpenSession(
		WINBIO_TYPE_FINGERPRINT,    // Service provider
		WINBIO_POOL_SYSTEM,         // Pool type
		WINBIO_FLAG_DEFAULT,        // Configuration and access
		NULL,                       // Array of biometric unit IDs
		0,                          // Count of biometric unit IDs
		NULL,                       // Database ID
		&sessionHandle              // [out] Session handle
	);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioOpenSession failed. hr = 0x%x\n", hr);
		WinBioExit(sessionHandle);
	}

	// Verify a biometric sample.
	wprintf_s(L"\n Calling WinBioVerify - Swipe finger on sensor...\n");
	hr = WinBioVerify(
		sessionHandle,
		&identity,
		subFactor,
		&unitId,
		&match,
		&rejectDetail
	);
	wprintf_s(L"\n Swipe processed - Unit ID: %d\n", unitId);
	if (FAILED(hr))
	{
		if (hr == WINBIO_E_NO_MATCH)
		{
			wprintf_s(L"\n- NO MATCH - identity verification failed.\n");
		}
		else if (hr == WINBIO_E_BAD_CAPTURE)
		{
			wprintf_s(L"\n- Bad capture; reason: %d\n", rejectDetail);
		}
		else
		{
			wprintf_s(L"\n WinBioVerify failed. hr = 0x%x\n", hr);
		}
		WinBioExit(sessionHandle);
	}
	wprintf_s(L"\n Fingerprint %d verified:\n", unitId);

}

//------------------------------------------------------------------------
// The following function retrieves the identity of the current user.
// This is a helper function and is not part of the Windows Biometric
// Framework API.
//
HRESULT GetCurrentUserIdentity(__inout PWINBIO_IDENTITY Identity)
{
	// Declare variables.
	HRESULT hr = S_OK;
	HANDLE tokenHandle = NULL;
	DWORD bytesReturned = 0;
	struct {
		TOKEN_USER tokenUser;
		BYTE buffer[SECURITY_MAX_SID_SIZE];
	} tokenInfoBuffer;

	// Zero the input identity and specify the type.
	ZeroMemory(Identity, sizeof(WINBIO_IDENTITY));
	Identity->Type = WINBIO_ID_TYPE_NULL;

	// Open the access token associated with the
	// current process
	if (!OpenProcessToken(
		GetCurrentProcess(),            // Process handle
		TOKEN_READ,                     // Read access only
		&tokenHandle))                  // Access token handle
	{
		DWORD win32Status = GetLastError();
		wprintf_s(L"Cannot open token handle: %d\n", win32Status);
		hr = HRESULT_FROM_WIN32(win32Status);
		goto e_Exit;
	}

	// Zero the tokenInfoBuffer structure.
	ZeroMemory(&tokenInfoBuffer, sizeof(tokenInfoBuffer));

	// Retrieve information about the access token. In this case,
	// retrieve a SID.
	if (!GetTokenInformation(
		tokenHandle,                    // Access token handle
		TokenUser,                      // User for the token
		&tokenInfoBuffer.tokenUser,     // Buffer to fill
		sizeof(tokenInfoBuffer),        // Size of the buffer
		&bytesReturned))                // Size needed
	{
		DWORD win32Status = GetLastError();
		wprintf_s(L"Cannot query token information: %d\n", win32Status);
		hr = HRESULT_FROM_WIN32(win32Status);
		goto e_Exit;
	}

	// Copy the SID from the tokenInfoBuffer structure to the
	// WINBIO_IDENTITY structure. 
	CopySid(
		SECURITY_MAX_SID_SIZE,
		Identity->Value.AccountSid.Data,
		tokenInfoBuffer.tokenUser.User.Sid
	);

	// Specify the size of the SID and assign WINBIO_ID_TYPE_SID
	// to the type member of the WINBIO_IDENTITY structure.
	Identity->Value.AccountSid.Size = GetLengthSid(tokenInfoBuffer.tokenUser.User.Sid);
	Identity->Type = WINBIO_ID_TYPE_SID;

e_Exit:

	if (tokenHandle != NULL)
	{
		CloseHandle(tokenHandle);
	}

	return hr;
}




int wmain()
{
	HRESULT hr = S_OK;
	int choice = 0;
	std::cout << "1. add fingerprint\n2. verify fingerprint\n3 .exit" << std::endl;
	std::cin >> choice;

	while (true) {
		if (choice == 1) {
			hr = EnrollSysPoolWithCallback(
				FALSE,
				FALSE,
				WINBIO_ANSI_381_POS_RH_INDEX_FINGER);
		}
		else if (choice == 2) {

		}
		else {
			std::cout << "Enter correct choice" << std::endl;
		}
	}


	

	return 0;
}


HRESULT EnrollSysPoolWithCallback(
	BOOL bCancel,
	BOOL bDiscard,
	WINBIO_BIOMETRIC_SUBTYPE subFactor)
{
	
	HRESULT hr = S_OK;
	WINBIO_IDENTITY identity = { 0 };
	WINBIO_SESSION_HANDLE sessionHandle = NULL;
	WINBIO_UNIT_ID unitId = 0;
	BOOLEAN isNewTemplate = TRUE;
	ENROLL_CALLBACK_CONTEXT callbackContext = { 0 };

	
	hr = WinBioOpenSession(
		WINBIO_TYPE_FINGERPRINT,    
		WINBIO_POOL_SYSTEM,        
		WINBIO_FLAG_DEFAULT,        
		NULL,                       
		0,                          
		NULL,                       
		&sessionHandle            
	);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioEnumBiometricUnits failed. ");
		wprintf_s(L"hr = 0x%x\n", hr);
		goto e_Exit;
	}

	
	wprintf_s(L"\n Swipe your finger to locate the sensor...\n");
	hr = WinBioLocateSensor(sessionHandle, &unitId);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioLocateSensor failed. hr = 0x%x\n", hr);
		goto e_Exit;
	}

	 
	hr = WinBioEnrollBegin(
		sessionHandle,      
		subFactor,         
		unitId             
	);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioEnrollBegin failed. hr = 0x%x\n", hr);
		goto e_Exit;
	}

	
	callbackContext.SessionHandle = sessionHandle;
	callbackContext.UnitId = unitId;
	callbackContext.SubFactor = subFactor;

	
	hr = WinBioEnrollCaptureWithCallback(
		sessionHandle,          
		EnrollCaptureCallback,  
		&callbackContext       
	);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioEnrollCaptureWithCallback failed. ");
		wprintf_s(L"hr = 0x%x\n", hr);
		goto e_Exit;
	}
	wprintf_s(L"\n Swipe the sensor with the appropriate finger...\n");

	
	if (bCancel)
	{
		wprintf_s(L"\n Starting CANCEL timer...\n");
		Sleep(7000);

		wprintf_s(L"\n Calling WinBioCancel\n");
		hr = WinBioCancel(sessionHandle);
		if (FAILED(hr))
		{
			wprintf_s(L"\n WinBioCancel failed. hr = 0x%x\n", hr);
			goto e_Exit;
		}
	}

	
	hr = WinBioWait(sessionHandle);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioWait failed. hr = 0x%x\n", hr);
	}

	if (bDiscard)
	{
		wprintf_s(L"\n Discarding enrollment...\n");
		hr = WinBioEnrollDiscard(sessionHandle);
		if (FAILED(hr))
		{
			wprintf_s(L"\n WinBioLocateSensor failed. ");
			wprintf_s(L"hr = 0x%x\n", hr);
		}
		goto e_Exit;
	}
	else
	{
		wprintf_s(L"\n Committing enrollment...\n");
		hr = WinBioEnrollCommit(
			sessionHandle,      // Handle to open biometric session
			&identity,          // WINBIO_IDENTITY object for the user
			&isNewTemplate);    // Is this a new template

		if (FAILED(hr))
		{
			wprintf_s(L"\n WinBioEnrollCommit failed. hr = 0x%x\n", hr);
			goto e_Exit;
		}
	}

e_Exit:
	if (sessionHandle != NULL)
	{
		WinBioCloseSession(sessionHandle);
		sessionHandle = NULL;
	}

	wprintf_s(L"\n Press any key to exit...");
	_getch();

	return hr;
}

//------------------------------------------------------------------------
// The following function is the callback for the Windows Biometric
// Framework WinBioEnrollCaptureWithCallback() function. 
//
VOID CALLBACK EnrollCaptureCallback(
	__in_opt PVOID EnrollCallbackContext,
	__in HRESULT OperationStatus,
	__in WINBIO_REJECT_DETAIL RejectDetail
)
{
	// Declare variables.
	HRESULT hr = S_OK;
	static SIZE_T swipeCount = 1;

	PENROLL_CALLBACK_CONTEXT callbackContext =
		(PENROLL_CALLBACK_CONTEXT)EnrollCallbackContext;

	wprintf_s(L"\n EnrollCaptureCallback executing\n");
	wprintf_s(L"\n Sample %d captured", swipeCount++);

	// The capture was not acceptable or the enrollment operation
	// failed.
	if (FAILED(OperationStatus))
	{
		if (OperationStatus == WINBIO_E_BAD_CAPTURE)
		{
			wprintf_s(L"\n Bad capture; reason: %d\n", RejectDetail);
			wprintf_s(L"\n Swipe your finger to capture another sample.\n");

			// Try again.
			hr = WinBioEnrollCaptureWithCallback(
				callbackContext->SessionHandle, // Open session handle
				EnrollCaptureCallback,          // Callback function
				EnrollCallbackContext           // Callback context
			);
			if (FAILED(hr))
			{
				wprintf_s(L"WinBioEnrollCaptureWithCallback failed.");
				wprintf_s(L"hr = 0x%x\n", hr);
			}
		}
		else
		{
			wprintf_s(L"EnrollCaptureCallback failed.");
			wprintf_s(L"OperationStatus = 0x%x\n", OperationStatus);
		}
		goto e_Exit;
	}

	// The enrollment operation requires more fingerprint swipes.
	// This is normal and depends on your hardware. Typically, at least
	// three swipes are required.
	if (OperationStatus == WINBIO_I_MORE_DATA)
	{
		wprintf_s(L"\n More data required.");
		wprintf_s(L"\n Swipe your finger on the sensor again.");

		hr = WinBioEnrollCaptureWithCallback(
			callbackContext->SessionHandle,
			EnrollCaptureCallback,
			EnrollCallbackContext
		);
		if (FAILED(hr))
		{
			wprintf_s(L"WinBioEnrollCaptureWithCallback failed. ");
			wprintf_s(L"hr = 0x%x\n", hr);
		}
		goto e_Exit;
	}

	wprintf_s(L"\n Template completed\n");

e_Exit:

	return;
}