// verify_shellinfo.cpp
// lpReserved → ShellInfo の流れをプログラムで確認
#include <windows.h>
#include <winternl.h>
#include <stdio.h>

// NtReadVirtualMemory のプロトタイプ
typedef NTSTATUS (NTAPI* NtReadVirtualMemory_t)(
    HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);

// NtQueryInformationProcess のプロトタイプ
typedef NTSTATUS (NTAPI* NtQueryInformationProcess_t)(
    HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // --- 関数ポインタ取得 ---
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    auto NtReadVMem = (NtReadVirtualMemory_t)
        GetProcAddress(ntdll, "NtReadVirtualMemory");
    auto NtQueryIP = (NtQueryInformationProcess_t)
        GetProcAddress(ntdll, "NtQueryInformationProcess");

    // --- マーカー文字列を lpReserved に入れてプロセス起動 ---
    STARTUPINFOW si = { 0 };
    si.cb = sizeof(si);
    WCHAR marker[] = L"WELCOME_TO_HACKFES_2026";
    si.lpReserved = marker;

    PROCESS_INFORMATION pi = { 0 };
    CreateProcessW(
        L"C:\\Windows\\System32\\notepad.exe",
        NULL, NULL, NULL, FALSE,
        CREATE_SUSPENDED,  // 一時停止状態で起動
        NULL, NULL, &si, &pi
    );
    printf("[+] 子プロセス起動 PID=%d\n", pi.dwProcessId);

    // --- Step1: PEB アドレス取得 ---
    PROCESS_BASIC_INFORMATION pbi = { 0 };
    NtQueryIP(pi.hProcess, ProcessBasicInformation,
              &pbi, sizeof(pbi), NULL);
    printf("[+] PEB アドレス: %p\n", pbi.PebBaseAddress);

    // --- Step2: PEB を読む ---
    PEB peb = { 0 };
    SIZE_T bytesRead;
    NtReadVMem(pi.hProcess, pbi.PebBaseAddress,
               &peb, sizeof(peb), &bytesRead);

    // --- Step3: RTL_USER_PROCESS_PARAMETERS を読む ---
    RTL_USER_PROCESS_PARAMETERS params = { 0 };
    NtReadVMem(pi.hProcess, peb.ProcessParameters,
               &params, sizeof(params), &bytesRead);

    printf("[+] ProcessParameters アドレス: %p\n",
           peb.ProcessParameters);

    // --- ShellInfo.Buffer の内容を読む ---
    // RTL_USER_PROCESS_PARAMETERS の ShellInfo オフセットは
    // Windows バージョンによって異なるため、
    // 実際のオフセットは WinDbg で確認すること
    // 目安: +0x0d0 付近（Win11 24H2）

    // CommandLine は RTL_USER_PROCESS_PARAMETERS に定義済み
    // ShellInfo は非公開なのでオフセットを直接使う
    // ここでは CommandLine で動作確認（ShellInfo は後述）
    WCHAR cmdline[256] = { 0 };
    NtReadVMem(pi.hProcess,
               params.CommandLine.Buffer,
               cmdline, sizeof(cmdline), &bytesRead);
    printf("[+] CommandLine.Buffer: %ls\n", cmdline);

    // --- ShellInfo は生ポインタで読む ---
    // params のアドレス + ShellInfo のオフセット
    // Win11 では +0xD0 が ShellInfo (_UNICODE_STRING)
    UNICODE_STRING shellInfo = { 0 };
    BYTE* paramBase = (BYTE*)peb.ProcessParameters;

    // オフセットは dt コマンドで確認した値を使う
    const DWORD SHELLINFO_OFFSET = 0xD0; // 要WinDbgで確認

    NtReadVMem(pi.hProcess,
               paramBase + SHELLINFO_OFFSET,
               &shellInfo, sizeof(shellInfo), &bytesRead);

    printf("[+] ShellInfo.Length: %d\n", shellInfo.Length);
    printf("[+] ShellInfo.Buffer アドレス: %p\n",
           shellInfo.Buffer);

    if (shellInfo.Buffer && shellInfo.Length > 0) {
        WCHAR buf[256] = { 0 };
        NtReadVMem(pi.hProcess, shellInfo.Buffer,
                   buf, shellInfo.Length, &bytesRead);
        printf("[+] ShellInfo.Buffer の中身: %ls\n", buf);
        // ↑ ここに "WELCOME_TO_HACKFES_2026" が出れば確認完了
    }

    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, 3000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
