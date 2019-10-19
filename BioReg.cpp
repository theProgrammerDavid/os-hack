#include "BioReg.h"



BioReg::BioReg()
{
}


BioReg::~BioReg()
{
}


HRESULT BioReg::EnrollSysPoolWithCallback(
	BOOL bCancel,
	BOOL bDiscard,
	WINBIO_BIOMETRIC_SUBTYPE subFactor)

{
	// Declare variables
	HRESULT hr = S_OK;
	WINBIO_IDENTITY identity = { 0 };
	WINBIO_SESSION_HANDLE sessionHandle = NULL;
	WINBIO_UNIT_ID unitId = 0;
	BOOLEAN isNewTemplate = TRUE;
	ENROLL_CALLBACK_CONTEXT callbackContext = { 0 };

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
		wprintf_s(L"\n WinBioEnumBiometricUnits failed. ");
		wprintf_s(L"hr = 0x%x\n", hr);
		goto e_Exit;
	}

	// Locate the sensor.
	wprintf_s(L"\n Swipe your finger to locate the sensor...\n");
	hr = WinBioLocateSensor(sessionHandle, &unitId);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioLocateSensor failed. hr = 0x%x\n", hr);
		goto e_Exit;
	}

	// Begin the enrollment sequence. 
	hr = WinBioEnrollBegin(
		sessionHandle,      // Handle to open biometric session
		subFactor,          // Finger to create template for
		unitId              // Biometric unit ID
	);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioEnrollBegin failed. hr = 0x%x\n", hr);
		goto e_Exit;
	}

	// Set up the custom callback context structure.
	callbackContext.SessionHandle = sessionHandle;
	callbackContext.UnitId = unitId;
	callbackContext.SubFactor = subFactor;

	// Call WinBioEnrollCaptureWithCallback. This is an asynchronous
	// method that returns immediately.
	hr = WinBioEnrollCaptureWithCallback(
		sessionHandle,          // Handle to open biometric session
		(PWINBIO_ENROLL_CAPTURE_CALLBACK)EnrollCaptureCallback,  // Callback function
		&callbackContext        // Pointer to the custom context
	);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioEnrollCaptureWithCallback failed. ");
		wprintf_s(L"hr = 0x%x\n", hr);
		goto e_Exit;
	}
	wprintf_s(L"\n Swipe the sensor with the appropriate finger...\n");

	// Cancel the enrollment if the bCancel flag is set.
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

	// Wait for the asynchronous enrollment process to complete
	// or be canceled.
	hr = WinBioWait(sessionHandle);
	if (FAILED(hr))
	{
		wprintf_s(L"\n WinBioWait failed. hr = 0x%x\n", hr);
	}

	// Discard the enrollment if the bDiscard flag is set.
	// Commit the enrollment if the flag is not set.
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









};