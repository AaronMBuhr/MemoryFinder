#define NOMINMAX
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Psapi.lib")

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include <vector>
#include <TlHelp32.h>

#include <Psapi.h>

DWORD GetProcessIdByName(const std::wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 processEntry = {};
    processEntry.dwSize = sizeof(processEntry);

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (std::wstring(processEntry.szExeFile) == processName) {
                CloseHandle(snapshot);
                return processEntry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &processEntry));
    }
    CloseHandle(snapshot);
    return 0;
}

//bool SearchMemory(HANDLE process, const std::string& searchString, size_t contextSize = 50) {  // default 50 bytes before and after
//    const size_t chunkSize = 4096; // 4KB
//    std::vector<char> buffer(chunkSize + searchString.size());  // increased size to ensure we don't miss overlapping strings between chunks
//
//    MEMORY_BASIC_INFORMATION memInfo = {};
//    char* addr = 0;
//    while (VirtualQueryEx(process, addr, &memInfo, sizeof(memInfo))) {
//        if (memInfo.State == MEM_COMMIT && (memInfo.Protect == PAGE_READWRITE || memInfo.Protect == PAGE_READONLY || memInfo.Protect == PAGE_EXECUTE_READWRITE)) {
//            SIZE_T bytesRead;
//            if (ReadProcessMemory(process, addr, buffer.data(), buffer.size(), &bytesRead)) {
//                for (size_t i = 0; i < (bytesRead - searchString.size()); i++) {
//                    if (memcmp(buffer.data() + i, searchString.c_str(), searchString.size()) == 0) {
//                        std::cout << "Found at address: " << (void*)(addr + i) << std::endl;
//
//                        size_t startContext = (i > contextSize) ? (i - contextSize) : 0;
//                        size_t endContext = std::min(i + searchString.size() + contextSize, bytesRead);
//
//                        std::string beforeMatch(buffer.data() + startContext, buffer.data() + i);
//                        std::string afterMatch(buffer.data() + i + searchString.size(), buffer.data() + endContext);
//
//                        std::cout << "Before: " << beforeMatch << std::endl;
//                        std::cout << "Match: " << searchString << std::endl;
//                        std::cout << "After: " << afterMatch << std::endl;
//
//                        return true;
//                    }
//                }
//            }
//        }
//        addr += memInfo.RegionSize;
//    }
//    return false;
//}

bool SearchMemoryAndSave(HANDLE process, const std::string& searchString, size_t contextSize = 50) {
    const size_t chunkSize = 4096; // 4KB
    std::vector<char> buffer(chunkSize + searchString.size()); // to account for overlapping

    MEMORY_BASIC_INFORMATION memInfo = {};
    char* addr = 0;
    int matchCount = 0; // to give unique filenames to matches
    bool foundMatch = false;

    //PROCESS_MEMORY_COUNTERS pmc;
    //if (GetProcessMemoryInfo(process, &pmc, sizeof(pmc))) {
    //    std::cout << "PageFaultCount: " << pmc.PageFaultCount << std::endl;
    //    std::cout << "PeakWorkingSetSize: " << pmc.PeakWorkingSetSize << std::endl;
    //    std::cout << "WorkingSetSize: " << pmc.WorkingSetSize << std::endl;
    //    std::cout << "QuotaPeakPagedPoolUsage: " << pmc.QuotaPeakPagedPoolUsage << std::endl;
    //    std::cout << "QuotaPagedPoolUsage: " << pmc.QuotaPagedPoolUsage << std::endl;
    //    std::cout << "QuotaPeakNonPagedPoolUsage: " << pmc.QuotaPeakNonPagedPoolUsage << std::endl;
    //    std::cout << "QuotaNonPagedPoolUsage: " << pmc.QuotaNonPagedPoolUsage << std::endl;
    //    std::cout << "PagefileUsage: " << pmc.PagefileUsage << std::endl;
    //    std::cout << "PeakPagefileUsage: " << pmc.PeakPagefileUsage << std::endl;
    //}
    //else {
    //    std::cerr << "GetProcessMemoryInfo failed." << std::endl;
    //}

    //std::cout << "Before VirtualQueryEx" << std::endl;
    //while (true) {
    //    SIZE_T result = VirtualQueryEx(process, addr, &memInfo, sizeof(memInfo));
    //    if (result == 0) {
    //        DWORD error = GetLastError();
    //        std::cout << "VirtualQueryEx failed with error code: " << error << std::endl;
    //        break;  // or handle error appropriately
    //    }
    //    std::cout << "State: " << memInfo.State << ", Protect: " << memInfo.Protect << std::endl;
    //    std::cout << "Reading block " << addr << std::endl;
    //    if (memInfo.State == MEM_COMMIT && (memInfo.Protect == PAGE_READWRITE || memInfo.Protect == PAGE_READONLY || memInfo.Protect == PAGE_EXECUTE_READWRITE)) {
    //        std::cout << "memInfo.State matched" << std::endl;
    //        SIZE_T bytesRead;
    //        if (int result = ReadProcessMemory(process, addr, buffer.data(), buffer.size(), &bytesRead)) {
    //            std::cout << "ReadProcessMemory result " << result << " read " << bytesRead << std::endl;
    //            for (size_t i = 0; i < (bytesRead - searchString.size()); i++) {
    //                if (memcmp(buffer.data() + i, searchString.c_str(), searchString.size()) == 0) {
    //                    foundMatch = true;

    //                    size_t startContext = (i > contextSize) ? (i - contextSize) : 0;
    //                    size_t endContext = std::min(i + searchString.size() + contextSize, bytesRead);

    //                    // Extract bytes for the context
    //                    std::vector<char> contextBytes(buffer.data() + startContext, buffer.data() + endContext);

    //                    // Save to a file
    //                    matchCount++;
    //                    std::string filename = "match_" + std::to_string(matchCount) + ".bin";
    //                    std::ofstream outFile(filename, std::ios::binary);
    //                    outFile.write(contextBytes.data(), contextBytes.size());
    //                    outFile.close();

    //                    std::cout << "Found match #" << matchCount << " at address: " << (void*)(addr + i) << ". Saved to " << filename << std::endl;

    //                    i += searchString.size() - 1; // Skip the current match to avoid overlapping matches
    //                }
    //            }
    //        }
    //    }
    //    std::cout << "incrementing addr" << std::endl;
    //    addr += memInfo.RegionSize;
    //}
    //return foundMatch;

    std::cout << "Before VirtualQueryEx" << std::endl;
    while (true) {
        SIZE_T result = VirtualQueryEx(process, addr, &memInfo, sizeof(memInfo));
        if (result == 0) {
            DWORD error = GetLastError();
            std::cout << "VirtualQueryEx failed with error code: " << error << std::endl;
            break;  // or handle error appropriately
        }
        std::cout << "State: " << memInfo.State << ", Protect: " << memInfo.Protect << std::endl;
        //std::cout << "Reading block ";// << addr << std::endl;

        std::cout << "checking meminfo.state" << std::endl;
        if (memInfo.State == MEM_COMMIT && (memInfo.Protect == PAGE_READWRITE || memInfo.Protect == PAGE_READONLY || memInfo.Protect == PAGE_EXECUTE_READWRITE)) {
            std::cout << "memInfo.State matched" << std::endl;
            SIZE_T bytesRead;
            if (int result = ReadProcessMemory(process, addr, buffer.data(), buffer.size(), &bytesRead)) {
                std::cout << "ReadProcessMemory result " << result << " read " << bytesRead << std::endl;
                for (size_t i = 0; i < (bytesRead - searchString.size()); i++) {
                    if (memcmp(buffer.data() + i, searchString.c_str(), searchString.size()) == 0) {
                        foundMatch = true;

                        size_t startContext = (i > contextSize) ? (i - contextSize) : 0;
                        size_t endContext = std::min(i + searchString.size() + contextSize, bytesRead);

                        // Extract bytes for the context
                        std::vector<char> contextBytes(buffer.data() + startContext, buffer.data() + endContext);

                        // Save to a file
                        matchCount++;
                        std::string filename = "match_" + std::to_string(matchCount) + ".bin";
                        std::ofstream outFile(filename, std::ios::binary);
                        outFile.write(contextBytes.data(), contextBytes.size());
                        outFile.close();

                        std::cout << "Found match #" << matchCount << " at address: " << (void*)(addr + i) << ". Saved to " << filename << std::endl;

                        i += searchString.size() - 1; // Skip the current match to avoid overlapping matches
                    }
                }
            }
        }
        else {
            std::cout << "meminfo.state did not match: " << memInfo.State << std::endl;
        }

        std::cout << "incrementing addr" << std::endl;
        addr += memInfo.RegionSize;
    }

    return foundMatch;

}




int main(int argc, char **argv) {
    std::wstring targetProcessName = L"Vindictus_x64.exe";  // replace with your program's exe name
    std::string searchString = std::string(argv[1]);  // replace with the string you are searching for
    std::cout << "Searching for: " << searchString << std::endl;
    size_t contextSize = 1000;

    DWORD processId = GetProcessIdByName(targetProcessName);
    if (!processId) {
        std::cerr << "Failed to find process." << std::endl;
        return 1;
    }

    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

    if (!process) {
        std::cerr << "Failed to open process." << std::endl;
        return 1;
    }

    //if (!SearchMemory(process, searchString, contextSize)) {
    //    std::cout << "String not found in process memory." << std::endl;
    //}

    if (!SearchMemoryAndSave(process, searchString, contextSize)) {
        std::cout << "String not found in process memory." << std::endl;
    }

    CloseHandle(process);
    return 0;
}
