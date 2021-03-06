#ifndef libhis_clearnvdata_hpp
#define libhis_clearnvdata_hpp

#ifdef WINDOWS
	#include "tspi.h"
	#include "tss_error.h"
	#include "tss_defines.h"
#endif
#ifdef LINUX
	#include <tss/tspi.h>
	#include <tss/tss_error.h>
	#include <tss/tss_defines.h>
#endif

#include "libhis_exception.hpp"

class libhis_clearnvdata
{
public:
	libhis_clearnvdata()
	{
		//set defaults
		nvstore_index = 0;

		//create a context object
		result = Tspi_Context_Create(&hcontext);
		if(result != TSS_SUCCESS) throw libhis_exception("Create Conntext", result);

		//Create TPM policy
		result = Tspi_Context_CreateObject(hcontext, TSS_OBJECT_TYPE_POLICY, TSS_POLICY_USAGE, &hpolicy_tpm);
		if(result != TSS_SUCCESS) throw libhis_exception("Create TPM Policy", result);

		//Create NVSTore object
		result = Tspi_Context_CreateObject(hcontext, TSS_OBJECT_TYPE_NV, 0, &hnvstore);
		if(result != TSS_SUCCESS) throw libhis_exception("Create NVStore object", result);
	}

	void clearnvdata(
		unsigned char	*auth_tpm_value,
		unsigned long	auth_tpm_size,
		bool			auth_tpm_sha1,
		unsigned long	nv_index)
	{
		//set up the index value
		if(nv_index == 0)
			nvstore_index = TPM_NV_INDEX_EKCert;
		else if(nv_index == 1)
			nvstore_index = TPM_NV_INDEX_TPM_CC;
		else if(nv_index == 2)
			nvstore_index = TPM_NV_INDEX_PlatformCert;
		else if(nv_index == 3)
			nvstore_index = TPM_NV_INDEX_Platform_CC;
		else
			nvstore_index = nv_index;

		//establish a session
		result = Tspi_Context_Connect(hcontext, 0);
		if(result != TSS_SUCCESS) throw libhis_exception("Connect Context", result);

		//get the TPM object
		result = Tspi_Context_GetTpmObject(hcontext, &htpm);
		if(result != TSS_SUCCESS) throw libhis_exception("Get TPM Object", result);

		//set up TPM auth
		if(auth_tpm_sha1)
		{
			result = Tspi_Policy_SetSecret(hpolicy_tpm, TSS_SECRET_MODE_SHA1, auth_tpm_size, auth_tpm_value);
			if(result != TSS_SUCCESS) throw libhis_exception("Set TPM Secret SHA1", result);
		}
		else
		{
			result = Tspi_Policy_SetSecret(hpolicy_tpm, TSS_SECRET_MODE_PLAIN, auth_tpm_size, auth_tpm_value);
			if(result != TSS_SUCCESS) throw libhis_exception("Set TPM Secret Plain", result);
		}

		//assign the TPM auth to the TPM
		result = Tspi_Policy_AssignToObject(hpolicy_tpm, htpm);
		if(result != TSS_SUCCESS) throw libhis_exception("Assign TPM Secret to TPM", result);

		//assign the TPM auth to the NVStore
		result = Tspi_Policy_AssignToObject(hpolicy_tpm, hnvstore);
		if(result != TSS_SUCCESS) throw libhis_exception("Assign TPM Secret to NVStore", result);

		//force NVData to be readable by the owner only
		result = Tspi_SetAttribUint32(hnvstore, TSS_TSPATTRIB_NV_PERMISSIONS, 0, TPM_NV_PER_OWNERREAD | TPM_NV_PER_OWNERWRITE);
		if(result != TSS_SUCCESS) throw libhis_exception("Requier owner auth on NVStore read/write", result);

		//set the read address
		result = Tspi_SetAttribUint32(hnvstore, TSS_TSPATTRIB_NV_INDEX, 0, nvstore_index);
		if(result != TSS_SUCCESS) throw libhis_exception("Set NVStore index", result);

		//clear the nvstore space
		result = Tspi_NV_ReleaseSpace(hnvstore);
		if(result != TSS_SUCCESS) throw libhis_exception("Clear data at NVStore index", result);
	}

	~libhis_clearnvdata()
	{
		//clean up NVStore
		result = Tspi_Context_CloseObject(hcontext, hnvstore);
		if(result != TSS_SUCCESS) throw libhis_exception("Close NVStore object", result);

		//clean up TPM policy
		result = Tspi_Context_CloseObject(hcontext, hpolicy_tpm);
		if(result != TSS_SUCCESS) throw libhis_exception("Close TPM Policy", result);

		//close context
		result = Tspi_Context_Close(hcontext);
		if(result != TSS_SUCCESS) throw libhis_exception("Close Context", result);
	}

private:
	TSS_RESULT		result;
	TSS_HCONTEXT	hcontext;
	TSS_HTPM		htpm;
	TSS_HPOLICY		hpolicy_tpm;
	TSS_HNVSTORE	hnvstore;
	UINT32			nvstore_index;
};

#endif
