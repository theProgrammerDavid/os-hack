#pragma once

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <winbio.h>

class BioReg
{
private:
	typedef struct _ENROLL_CALLBACK_CONTEXT {
		WINBIO_SESSION_HANDLE SessionHandle;
		WINBIO_UNIT_ID UnitId;
		WINBIO_BIOMETRIC_SUBTYPE SubFactor;
	} ENROLL_CALLBACK_CONTEXT, *PENROLL_CALLBACK_CONTEXT;



public:


	VOID CALLBACK EnrollCaptureCallback(
		__in_opt PVOID EnrollCallbackContext,
		__in HRESULT OperationStatus,
		__in WINBIO_REJECT_DETAIL RejectDetail
	);

	HRESULT EnrollSysPoolWithCallback(
		BOOL bCancel,
		BOOL bDiscard,
		WINBIO_BIOMETRIC_SUBTYPE subFactor);

	BioReg();
	~BioReg();
};

