#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <TlHelp32.h>  
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

//minimun ini parser from 3Dmigoto project.
#define SENSIBLE_ENUM(ENUMTYPE) \
inline int operator | (ENUMTYPE a, ENUMTYPE b) { return (((int)a) | ((int)b)); } \
inline int operator & (ENUMTYPE a, ENUMTYPE b) { return (((int)a) & ((int)b)); } \
inline int operator ^ (ENUMTYPE a, ENUMTYPE b) { return (((int)a) ^ ((int)b)); } \
inline int operator ~ (ENUMTYPE a) { return (~((int)a)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) |= ((int)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) &= ((int)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) ^= ((int)b)); } \
inline bool operator || (ENUMTYPE a,  ENUMTYPE b) { return (((int)a) || ((int)b)); } \
inline bool operator || (    bool a,  ENUMTYPE b) { return (((int)a) || ((int)b)); } \
inline bool operator || (ENUMTYPE a,      bool b) { return (((int)a) || ((int)b)); } \
inline bool operator && (ENUMTYPE a,  ENUMTYPE b) { return (((int)a) && ((int)b)); } \
inline bool operator && (    bool a,  ENUMTYPE b) { return (((int)a) && ((int)b)); } \
inline bool operator && (ENUMTYPE a,      bool b) { return (((int)a) && ((int)b)); } \
inline bool operator ! (ENUMTYPE a) { return (!((int)a)); }

template <class T1, class T2>
struct EnumName_t {
    T1 name;
    T2 val;
};

const char* find_ini_section_lite(const char* buf, const char* section_name);
bool find_ini_setting_lite(const char* buf, const char* setting, char* ret, size_t n);
bool find_ini_bool_lite(const char* buf, const char* setting, bool def);
int find_ini_int_lite(const char* buf, const char* setting, int def);


using namespace std;

// Technique 1: CreateRemoteThread
DWORD demoCreateRemoteThreadW(PCWSTR pszLibFile, DWORD dwProcessId);

static const char* skip_space(const char* buf)
{
    for (; *buf == ' ' || *buf == '\t'; buf++) {}
    return buf;
}

// Returns a pointer to the next non-whitespace character on a following line
static const char* next_line(const char* buf)
{
    for (; *buf != '\0' && *buf != '\n' && *buf != '\r'; buf++) {}
    for (; *buf == '\n' || *buf == '\r' || *buf == ' ' || *buf == '\t'; buf++) {}
    return buf;
}

// Returns a pointer to the first non-whitespace character on the line
// following [section_name] (which may be a pointer to the zero terminator if
// EOF is encountered), or NULL if the section is not found. section_name must
// be lower case.
const char* find_ini_section_lite(const char* buf, const char* section_name)
{
    const char* p;

    for (buf = skip_space(buf); *buf; buf = next_line(buf)) {
        if (*buf == '[') {
            for (buf++, p = section_name; *p && (tolower((unsigned char)*buf) == *p); buf++, p++) {}
            if (*buf == ']' && *p == '\0')
                return next_line(buf);
        }
    }

    return 0;
}

// Searches for the setting. If found in the current section, copies the value
// to ret stripping any whitespace from the end of line and returns true. If
// not found or the buffer size is insufficient, returns false.
bool find_ini_setting_lite(const char* buf, const char* setting, char* ret, size_t n)
{
    const char* p;
    char* r;
    size_t i;

    for (buf = skip_space(buf); *buf; buf = next_line(buf)) {
        // Check for end of section
        if (*buf == '[')
            return false;

        // Check if line matches setting
        for (p = setting; *p && tolower((unsigned char)*buf) == *p; buf++, p++) {}
        buf = skip_space(buf);
        if (*buf != '=' || *p != '\0')
            continue;

        // Copy setting until EOL/EOF to ret buffer
        buf = skip_space(buf + 1);
        for (i = 0, r = ret; i < n; i++, buf++, r++) {
            *r = *buf;
            if (*buf == '\n' || *buf == '\r' || *buf == '\0') {
                // Null terminate return buffer and strip any whitespace from EOL:
                for (; r >= ret && (*r == '\0' || *r == '\n' || *r == '\r' || *r == ' ' || *r == '\t'); r--)
                    *r = '\0';
                return true;
            }
        }
        // Insufficient room in buffer
        return false;
    }
    return false;
}

bool find_ini_bool_lite(const char* buf, const char* setting, bool def)
{
    char val[8];

    if (!find_ini_setting_lite(buf, setting, val, 8))
        return def;

    if (!_stricmp(val, "1") || !_stricmp(val, "true") || !_stricmp(val, "yes") || !_stricmp(val, "on"))
        return true;

    if (!_stricmp(val, "0") || !_stricmp(val, "false") || !_stricmp(val, "no") || !_stricmp(val, "off"))
        return false;

    return def;
}

int find_ini_int_lite(const char* buf, const char* setting, int def)
{
    char val[16];

    if (!find_ini_setting_lite(buf, setting, val, 16))
        return def;

    return atoi(val);
}

int main()
{
    printf("Usage Instruction: \n");
    printf("1.Ensure that loader_config.ini exists in the directory where this program is located.\n");
    printf("2.Copy 3Dmigoto files to target program's directory.\n");
    printf("3.Edit loader_config.ini, set target= to your destination program.\n");
    printf("4.Ensure run GameStarter.exe as Administrator.\n");
    printf("5.If every thing works fine, your game will open with 3Dmigoto injected after you press any key to continue.\n\n");
    printf("Notes: \n");
    printf("1.Don't put GameStarter.exe in your destination program's directory,or it won't work.\n");
    printf("2.Always use GameStarter.exe to start your game,or your d3d11.dll may be deleted by target program.\n");
    printf("3.The path in loader_config.ini needs to be copied from the path bar and cannot be copied from the right-click file properties, otherwise there will be garbled code and the path cannot be recognized.\n");
    printf("4.The end of the path in loader_config.ini need a line break,or it can not work.\n\n");
    printf("This is a free software offered by ShaderFreedom Community.\nAuthor: NicoMico.\nShaderFreedom Community Discord: https://discord.gg/U8cRdUYZrR \n\n");


    HANDLE hToken;
    PROCESS_INFORMATION pi;
    STARTUPINFOW si;
    BOOL bResult;

    // Get a handle to the primary token of the current process
    bResult = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);

    if (!bResult)
    {
        printf("OpenProcessToken failed: %d\n", GetLastError());
        return -1;
    }

    // Initialize the PROCESS_INFORMATION structure
    ZeroMemory(&pi, sizeof(pi));

    // Initialize the STARTUPINFO structure
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    // open an ini file
    HANDLE ini_file = CreateFile(L"loader_config.ini", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (ini_file == INVALID_HANDLE_VALUE) {
        printf("Unable to open loader_config.ini\n");
        system("pause");
        exit(1);
    }
    // initialize variables
    char* buf, target[MAX_PATH], setting[MAX_PATH], module_path[MAX_PATH];

    DWORD filesize = GetFileSize(ini_file, NULL);
    DWORD readsize;
    buf = new char[filesize + 1];
    if (!buf)
    {
        printf("Unable to allocate loader_config.ini buffer\n");
        system("pause");
        exit(1);
    }


    if (!ReadFile(ini_file, buf, filesize, &readsize, 0) || filesize != readsize)
    {
        printf("Error reading loader_config.ini\n");
        system("pause");
        exit(1);
    }

    CloseHandle(ini_file);
    const char* ini_section = find_ini_section_lite(buf, "loader");
    if (!ini_section)
    {
        printf("loader_config.ini missing required [Loader] section\n");
        system("pause");
        exit(1);
    }

    // Check that the target is configured. We don't do anything with this
    // setting from here other than to make sure it is set, because the
    // injection method we are using cannot single out a specific process.
    // Once 3DMigoto has been injected it into a process it will check this
    // value and bail if it is in the wrong one.
    if (!find_ini_setting_lite(ini_section, "target", target, MAX_PATH))
    {
        printf("loader_config.ini [Loader] section missing required \"target\" setting\n");
        system("pause");
        exit(1);
    }

    // 假设你有一个char*类型的字符串
    char* str = target;

    // 声明一个wchar_t*类型的指针
    wchar_t* gamepath = NULL;
    // 获取转换后的字符串长度
    int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    // 分配内存空间
    gamepath = new wchar_t[len];

    // 调用MultiByteToWideChar函数进行转换
    MultiByteToWideChar(CP_ACP, 0, str, -1, gamepath, len);

    ;
    // Create a new process as the specified user
    bResult = CreateProcessAsUserW(
        hToken, // user token
        //L"D:\\Honkai Impact 3\\Games\\BH3.exe", // application name
        gamepath, // application name
        NULL, // command line
        NULL, // process attributes
        NULL, // thread attributes
        FALSE, // inherit handles
        CREATE_SUSPENDED, // creation flags
        NULL, // environment
        NULL, // current directory
        &si, // startup info
        &pi // process information
    );

    if (!bResult)
    {
        printf("CreateProcessAsUserA failed: %d\n", GetLastError());
        return -1;
    }

    //inject DLL  This line is only for test purpose.
    //PCWSTR pszLibFile = L"C:\\Users\\Administrator\\Desktop\\HSR_GIMI6.0_Loader\\d3d11.dll";


    // 假设你有一个char*类型的dll路径
    char* dllPath = module_path;
    // 声明一个wchar_t*类型的指针
    wchar_t* wptr = NULL;
    // 获取转换后的字符串长度
    len = MultiByteToWideChar(CP_ACP, 0, dllPath, -1, NULL, 0);
    // 分配内存空间
    wptr = new wchar_t[len];
    // 调用MultiByteToWideChar函数进行转换
    MultiByteToWideChar(CP_ACP, 0, dllPath, -1, wptr, len);
    // 强制转换成PCWSTR类型

    //PCWSTR pszLibFile = L"C:\\Users\\Administrator\\Desktop\\HSR_GIMI6.0_Loader\\d3d11.dll";
    PCWSTR pszLibFile = (PCWSTR)wptr;

    // Do not uncomment these code if you don't need inject dll into game.
    //DWORD dwProcessId = pi.dwProcessId;
    // demoCreateRemoteThreadW(pszLibFile, dwProcessId);


    printf("Target program run succes.\n");

    system("pause");
    //ResumeThread
    ResumeThread(pi.hThread);

    // Close the handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hToken);

    return 0;
    //BOOL result = CreateProcessAsUserA(hToken, lpGameNameStr,NULL,NULL,NULL,TRUE, EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED, NULL, GameFile, (LPSTARTUPINFOA)&si, &pi);

}


//This method copied from github,didn't remember where it comes from.
DWORD demoCreateRemoteThreadW(PCWSTR pszLibFile, DWORD dwProcessId)
{
    // Calculate the number of bytes needed for the DLL's pathname
    DWORD dwSize = (lstrlenW(pszLibFile) + 1) * sizeof(wchar_t);

    // Get process handle passing in the process ID
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION |
        PROCESS_CREATE_THREAD |
        PROCESS_VM_OPERATION |
        PROCESS_VM_WRITE,
        FALSE, dwProcessId);
    if (hProcess == NULL)
    {
        wprintf(TEXT("[-] Error: Could not open process for PID (%d).\n"), dwProcessId);
        return(1);
    }

    // Allocate space in the remote process for the pathname
    LPVOID pszLibFileRemote = (PWSTR)VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
    if (pszLibFileRemote == NULL)
    {
        wprintf(TEXT("[-] Error: Could not allocate memory inside PID (%d).\n"), dwProcessId);
        return(1);
    }

    // Copy the DLL's pathname to the remote process address space
    DWORD n = WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID)pszLibFile, dwSize, NULL);
    if (n == 0)
    {
        wprintf(TEXT("[-] Error: Could not write any bytes into the PID [%d] address space.\n"), dwProcessId);
        return(1);
    }

    // Get the real address of LoadLibraryW in Kernel32.dll
    PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
    if (pfnThreadRtn == NULL)
    {
        wprintf(TEXT("[-] Error: Could not find LoadLibraryA function inside kernel32.dll library.\n"));
        return(1);
    }

    // Create a remote thread that calls LoadLibraryW(DLLPathname)
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, pszLibFileRemote, 0, NULL);
    if (hThread == NULL)
    {
        wprintf(TEXT("[-] Error: Could not create the Remote Thread.\n"));
        return(1);
    }
    //else
    //	wprintf(TEXT("[+] Success: DLL injected via CreateRemoteThread().\n"));

    // Wait for the remote thread to terminate
    WaitForSingleObject(hThread, INFINITE);

    // Free the remote memory that contained the DLL's pathname and close Handles
    if (pszLibFileRemote != NULL)
        VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

    if (hThread != NULL)
        CloseHandle(hThread);

    if (hProcess != NULL)
        CloseHandle(hProcess);

    return(0);
}