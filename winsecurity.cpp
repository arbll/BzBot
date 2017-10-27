#include "stdafx.h"
#include "winsecurity.h"

#define LOWINTEGRITYLEVEL_FULLACCESS         L"S:(ML;;NRNWNX;;;LW)"

void *WinSecurity::_getFullAccessDescriptors(SECURITY_ATTRIBUTES *sa, SECURITY_DESCRIPTOR *sd)
{
	if (InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION) && SetSecurityDescriptorDacl(sd, TRUE, NULL, FALSE))
	{
		PSECURITY_DESCRIPTOR sdLow;
		if (ConvertStringSecurityDescriptorToSecurityDescriptorW(LOWINTEGRITYLEVEL_FULLACCESS, SDDL_REVISION_1, &sdLow, NULL) != FALSE)
		{
			BOOL saclPresent, saclDefaulted;
			PACL sacl = NULL;
			if (GetSecurityDescriptorSacl(sdLow, &saclPresent, &sacl, &saclDefaulted) == FALSE ||
				SetSecurityDescriptorSacl(sd, saclPresent, sacl, saclDefaulted) == FALSE)
			{
				LocalFree(sdLow);
				sdLow = (PSECURITY_DESCRIPTOR)-1;
			}
		}
		else sdLow = (PSECURITY_DESCRIPTOR)-1;

		if (sa)
		{
			sa->nLength = sizeof(SECURITY_ATTRIBUTES);
			sa->lpSecurityDescriptor = sd;
			sa->bInheritHandle = FALSE;
		}

		return sdLow;
	}
	return NULL;
}