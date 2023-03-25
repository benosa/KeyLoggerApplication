#include "defines.h"

/* Create a DACL that will allow everyone to have full control over our pipe. */
VOID BuildDACL(PSECURITY_DESCRIPTOR pDescriptor)
{
    PSID pSid;
    EXPLICIT_ACCESS ea;
    PACL pAcl;

    SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;

    AllocateAndInitializeSid(&sia, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0,
        &pSid);

    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea.grfAccessPermissions = FILE_ALL_ACCESS;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPTSTR)pSid;

    if (SetEntriesInAcl(1, &ea, NULL, &pAcl) == ERROR_SUCCESS)
    {
        if (SetSecurityDescriptorDacl(pDescriptor, TRUE, pAcl, FALSE) == 0)
            _tprintf(_T("[*] Failed to set DACL (%u)\n"), GetLastError());
    }
    else
        _tprintf(_T("[*] Failed to add ACE in DACL (%u)\n"), GetLastError());
}


/* Create a SACL that will allow low integrity processes connect to our pipe. */
VOID BuildSACL(PSECURITY_DESCRIPTOR pDescriptor)
{
    PSID pSid;
    PACL pAcl;

    SID_IDENTIFIER_AUTHORITY sia = SECURITY_MANDATORY_LABEL_AUTHORITY;
    DWORD dwACLSize = sizeof(ACL) + sizeof(SYSTEM_MANDATORY_LABEL_ACE) +
        GetSidLengthRequired(1);

    pAcl = (PACL)LocalAlloc(LPTR, dwACLSize);
    InitializeAcl(pAcl, dwACLSize, ACL_REVISION);

    AllocateAndInitializeSid(&sia, 1, SECURITY_MANDATORY_LOW_RID, 0, 0, 0, 0,
        0, 0, 0, &pSid);

    if (AddMandatoryAce(pAcl, ACL_REVISION, 0, SYSTEM_MANDATORY_LABEL_NO_WRITE_UP,
        pSid) == TRUE)
    {
        if (SetSecurityDescriptorSacl(pDescriptor, TRUE, pAcl, FALSE) == 0)
            _tprintf(_T("[*] Failed to set SACL (%u)\n"), GetLastError());
    }
    else
        _tprintf(_T("[*] Failed to add ACE in SACL (%u)\n"), GetLastError());
}

/* Initialize security attributes to be used by `CreateNamedPipe()' below. */
VOID InitSecurityAttributes(PSECURITY_ATTRIBUTES pAttributes)
{
    PSECURITY_DESCRIPTOR pDescriptor;

    pDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
        SECURITY_DESCRIPTOR_MIN_LENGTH);
    InitializeSecurityDescriptor(pDescriptor, SECURITY_DESCRIPTOR_REVISION);

    BuildDACL(pDescriptor);
    BuildSACL(pDescriptor);

    pAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
    pAttributes->lpSecurityDescriptor = pDescriptor;
    pAttributes->bInheritHandle = TRUE;
}

std::wstring stringToWString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }

    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], len);
    return wstr;
}

std::string wStringToString(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::string();
    }

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    std::string str(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &str[0], len, NULL, NULL);
    return str;
}
