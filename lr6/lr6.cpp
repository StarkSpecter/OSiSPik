#include <windows.h>
#include <iostream>
#include <string>

std::wstring GetRegValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName) {
    wchar_t data[256];
    DWORD dataSize = sizeof(data);
    if (RegGetValueW(hKey, subKey.c_str(), valueName.c_str(), RRF_RT_REG_SZ, NULL, data, &dataSize) == ERROR_SUCCESS)
        return std::wstring(data);
    return L"";
}

int main() {
    // Версия Windows
    std::wstring osName = GetRegValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName");
    std::wstring osVersion = GetRegValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"CurrentBuild");

    // Активный пользователь
    wchar_t userName[256];
    DWORD userSize = sizeof(userName);
    GetUserNameW(userName, &userSize);
    std::wstring user(userName);

    // Процессор
    std::wstring cpu = GetRegValue(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", L"ProcessorNameString");

    // Оперативная память
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG ram = memInfo.ullTotalPhys / (1024 * 1024);

    // Диски
    DWORD drives = GetLogicalDrives();
    std::string disks;
    for (int i = 0; i < 26; ++i) {
        if (drives & (1 << i)) {
            char drive[4] = { char('A' + i), ':', '\\', 0 };
            disks += drive;
            disks += " ";
        }
    }

    // Вывод информации
    std::wcout << L"Windows: " << osName << L" " << osVersion << L"\n"
        << L"User: " << user << L"\n"
        << L"CPU: " << cpu << L"\n";

    std::cout << "RAM: " << ram << " MB\n"
        << "Disks: " << disks << std::endl;
}
