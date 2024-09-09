#include "SharedStuffLib.h"
#include "MemoryHackerLib.h"
#include <easylogging++.h>
#include <tclap/CmdLine.h>

#pragma warning(disable: 6387)  // `Variable could be zero`

#define PRODUCT_NAME L"ProcessMemoryScanner"
#define FILE_NAME "addresses.txt"
#define FILTERED_FILE_NAME "filtered_addresses.txt"
#define BUFF_SIZE (SIZE_T)(10'000 * 4096)
#define USER_MODE_VM_MAX_ADDRESS (SIZE_T)0x7FFF'FFFFFFFF

INITIALIZE_EASYLOGGINGPP

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: SearchProcessAddresses()

  Summary:  Searches for process addresses what point to specified value.
            Found addresses will be written to the file.

  Args:     _In_ HANDLE process,
                The process where addresses are pointing.
            _In_ DWORD64 value,
                Value that addresses should point.
            _In_ DWORD valueSize)
                Value size.
            _In_ BOOL isDriverMode
                Specify if function should use kernel driver.

  Returns:  void
-----------------------------------------------------------------F-F*/
void SearchProcessAddresses(
    _In_ HANDLE process,
    _In_ DWORD64 value,
    _In_ DWORD valueSize,
    _In_ BOOL isDriverMode);

void SearchProcessAddressesRange(
    _In_ HANDLE process,
    _In_ MEMORY_BASIC_INFORMATION memoryInfo,
    _In_ DWORD64 value,
    _In_ DWORD valueSize,
    _In_ BOOL isDriverMode,
    _In_ FILE* pFile);

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: FilterProcessAddresses()

  Summary:  Filters addresses within file. It reads process addresses,
            compares them with actual process values. Thoso addresses
            that still match the value will be written within the same file.

  Args:     _In_ HANDLE process,
                The process where addresses are point.
            _In_ DWORD64 value,
                Value to compare.
            _In_ DWORD valueSize)
                Value size.
            _In_ BOOL isDriverMode
                Specify if function should use kernel driver.

  Returns:  void
-----------------------------------------------------------------F-F*/
void FilterProcessAddresses(
    _In_ HANDLE process,
    _In_ DWORD64 value,
    _In_ DWORD valueSize,
    _In_ BOOL isDriverMode);

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: WriteProcessAddresses()

  Summary:  Writes to specified process addresses using specified value.
            It takes addresses from file.

  Args:     _In_ HANDLE process,
                The process where addresses are point.
            _In_ DWORD64 value,
                Value to write.
            _In_ DWORD valueSize)
                Value size.
            _In_ BOOL isDriverMode
                Specify if function should use kernel driver.

  Returns:  void
-----------------------------------------------------------------F-F*/
void WriteProcessAddresses(
    _In_ HANDLE process,
    _In_ DWORD64 value,
    _In_ DWORD valueSize,
    _In_ BOOL isDriverMode);

void ParseArguments(
    _In_ int argc,
    _In_ char** argv,
    _Out_ std::string& command,
    _Out_ std::string& valueStr,
    _Out_ std::string& valueType,
    _Out_ PDWORD pProcessId,
    _Out_ PBOOL pIsDriverMode);

DWORD64 ParseValue(
    _In_ std::string valueStr,
    _In_ std::string valueType,
    _In_ PDWORD pValueSize);

void OutputFile();


/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Summary:  It initializes the logger, 
            if everything is OK, starts `CheatLib` framework.

  Args:		ProcessMemoryScanner [<command>] <value> [<options>]

              The following commands are available:
                  search
                  filter
                  write

              The following options are available:
                   --driver-mode
                     Switch to use kernel driver to access specified process

                   --value-type <int32 | int64 | float | double>
                     (required)  The following types are avaiable: int32 | int64 | float |
                     double

                   --process-id <id>
                     (required)

                   --,  --ignore_rest
                     Ignores the rest of the labeled arguments following this flag.

                   --version
                     Displays version information and exits.

                   -h,  --help
                     Displays usage information and exits.
  Returns:  0
                If success.
            non-zero valie
                If fail.
-----------------------------------------------------------------F-F*/
int main(int argc, char* argv[])
{
    DWORD result = 0;
    HANDLE hProcess = NULL;
    std::string command;
    std::string valueStr;
    std::string valueType;
    DWORD processId = 0;
    BOOL isDriverMode = 0;
    DWORD valueSize = 0;
    DWORD64 value = 0;  // DWORD64 can hold value with each type

    try {
#ifdef _DEBUG
        ConfigureLoggers(PRODUCT_NAME, true);
#else
        ConfigureLoggers(PRODUCT_NAME, false);
#endif

        ParseArguments(argc, argv, command, valueStr, valueType, &processId, &isDriverMode);
        value = ParseValue(valueStr, valueType, &valueSize);

        LOG(INFO) << "command: " << command;
        LOG(INFO) << "value as DWORD64: " << value;
        LOG(INFO) << "processIdArg: " << processId;
        LOG(INFO) << "valueType: " << valueType;
        LOG(INFO) << "valueSize: " << valueSize;
        LOG(INFO) << "isDriverMode: " << isDriverMode;

        hProcess = OpenProcess(
            PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION,
            FALSE,
            processId);
        if (hProcess == NULL) {
            throw std::runtime_error("Failed to open process");
        }
        if (isDriverMode && IsDriverAvailable() == FALSE) {
            throw std::runtime_error("The driver is not available");
        }

        if (command == "search") {
            SearchProcessAddresses(hProcess, value, valueSize, isDriverMode);
        }
        else if (command == "filter") {
            FilterProcessAddresses(hProcess, value, valueSize, isDriverMode);
        }
        else if (command == "write") {
            WriteProcessAddresses(hProcess, value, valueSize, isDriverMode);
        }
        else {
            throw TCLAP::ArgException("Invalid command", "command");
        }

        OutputFile();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        result = 2;
    }
    catch (...) {
        result = 3;
    }

    if (hProcess)
        CloseHandle(hProcess);

    return 0;
}

void ParseArguments(
    _In_ int argc,
    _In_ char** argv,
    _Out_ std::string& command,
    _Out_ std::string& valueStr,
    _Out_ std::string& valueType,
    _Out_ PDWORD pProcessId,
    _Out_ PBOOL pIsDriverMode)
{
    TCLAP::CmdLine cmd(
        "`ProcessMemoryScanner` scans specified process for specified value.",
        ' ',
        "1.0.0");
    TCLAP::UnlabeledValueArg<std::string> commandArg(
        "command", "The following commands are available: search | filter | write", TRUE,
        "search | filter | write",
        "command", cmd);
    TCLAP::UnlabeledValueArg<std::string> valueStrArg(
        "value", "The value of one of the specified type", TRUE,
        "500",
        "value", cmd );
    TCLAP::ValueArg<DWORD> processIdArg(
        "", "process-id", "", TRUE,
        1224,
        "id", cmd );
    TCLAP::ValueArg<std::string> valueTypeArg(
        "", "value-type", "The following types are avaiable: int32 | int64 | float | double", TRUE,
        "int32",
        "int32 | int64 | float | double", cmd);
    TCLAP::SwitchArg isDriverModeSwitch(
        "", "driver-mode", "Switch to use kernel driver to access specified process",
        cmd, FALSE);
    cmd.parse(argc, argv);

    command = commandArg.getValue();
    valueStr = valueStrArg.getValue();
    valueType = valueTypeArg.getValue();
    *pProcessId = processIdArg.getValue();
    *pIsDriverMode = isDriverModeSwitch.getValue();
}

DWORD64 ParseValue(
    _In_ std::string valueStr,
    _In_ std::string valueType,
    _In_ PDWORD pValueSize)
{
    DWORD64 value = 0;
    if (valueType == "int32") {
        value = (DWORD64)std::stol(valueStr);
        *pValueSize = sizeof(INT32);
    }
    else if (valueType == "int64") {
        value = (DWORD64)std::stoll(valueStr);
        *pValueSize = sizeof(INT64);
    }
    else if (valueType == "float") {
        FLOAT valueFloat = std::stof(valueStr);
        memcpy(&value, &valueFloat, sizeof(FLOAT));
        *pValueSize = sizeof(FLOAT);
    }
    else if (valueType == "double") {
        DOUBLE valueDouble = std::stod(valueStr);
        memcpy(&value, &valueDouble, sizeof(DOUBLE));
        *pValueSize = sizeof(DOUBLE);
    }
    else {
        throw TCLAP::ArgParseException("--value-type");
    }
    return value;
}

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: SearchProcessAddresses()

  Summary:  Searches for process addresses what point to specified value.
            Found addresses will be written to the file.

  Args:     _In_ HANDLE process,
                The process where addresses are pointing.
            _In_ DWORD64 value,
                Value that addresses should point.
            _In_ DWORD valueSize)
                Value size.

  Returns:  void
-----------------------------------------------------------------F-F*/
void SearchProcessAddresses(
    _In_ HANDLE process,
    _In_ DWORD64 value,
    _In_ DWORD valueSize,
    _In_ BOOL isDriverMode)
{
    MEMORY_BASIC_INFORMATION memoryInfo = { 0 };
    SIZE_T i = 0x0;
    FILE* pFile;

    if (fopen_s(&pFile, FILE_NAME, "w")) {
        throw std::runtime_error("Failed to open output file");
    }

    while (i < USER_MODE_VM_MAX_ADDRESS) {
        if (VirtualQueryEx(
                process,
                (LPCVOID)i,
                &memoryInfo,
                sizeof(memoryInfo)) == 0) {
            break;
        }
        if (memoryInfo.State == MEM_COMMIT) {
            if (memoryInfo.RegionSize >= BUFF_SIZE) {
                LOG(WARNING) << "Found pages range with too big size: " << memoryInfo.RegionSize << ". Skipping it";
            }
            else {
                SearchProcessAddressesRange(
                    process,
                    memoryInfo,
                    value,
                    valueSize,
                    isDriverMode,
                    pFile);
            }
        }
        i += memoryInfo.RegionSize;
    }

    if (pFile)
        fclose(pFile);
}

void SearchProcessAddressesRange(
    _In_ HANDLE process,
    _In_ MEMORY_BASIC_INFORMATION memoryInfo,
    _In_ DWORD64 value,
    _In_ DWORD valueSize,
    _In_ BOOL isDriverMode,
    _In_ FILE* pFile)
{
    PBYTE buff = (PBYTE)VirtualAlloc(  // buff will contain read memory
        NULL,
        100 * BUFF_SIZE,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);
    BOOL result = FALSE;
    if (buff == NULL) {
        throw std::runtime_error("Not enough memory");
    }

    LOG(INFO) << "Start of range:" << memoryInfo.BaseAddress;
    LOG(INFO) << "End of range:" << std::dec << (SIZE_T)memoryInfo.BaseAddress + memoryInfo.RegionSize;
    LOG(INFO) << "Size of range:" << memoryInfo.BaseAddress;

    if (isDriverMode == FALSE) {
        result = ReadProcessMemory(
            process,
            (LPCVOID)memoryInfo.BaseAddress,
            buff,
            memoryInfo.RegionSize,
            NULL);
        // ReadProcessMemory() could return 0 if it cannot open process or
        // it cannot read specified virtual memory range.
    }
    else {
        result = ReadProcessMemoryDrivered(
            process,
            (LPCVOID)memoryInfo.BaseAddress,
            buff,
            memoryInfo.RegionSize,
            NULL);
    }
    LOG(INFO) << (result ? "Reading is successed" : "Reading is failed");
    if (result) {
        for (SIZE_T buff_i = 0x0; buff_i < memoryInfo.RegionSize; buff_i += valueSize) {
            LOG_IF(buff_i >= BUFF_SIZE, FATAL) << "Buff length is not enough";
            if (memcmp(buff + buff_i, &value, valueSize) == 0) {
                fprintf(pFile, "%p\n", (PVOID)((SIZE_T)memoryInfo.BaseAddress + buff_i));
            }
        }
    }
    VirtualFree(buff, 0, MEM_RELEASE);
}

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: FilterProcessAddresses()

  Summary:  Filters addresses within file. It reads process addresses,
            compares them with actual process values. Thoso addresses
            that still match the value will be written within the same file.

  Args:     _In_ HANDLE process,
                The process where addresses are point.
            _In_ DWORD64 value,
                Value to compare.
            _In_ DWORD valueSize)
                Value size.
            _In_ BOOL isDriverMode
                Specify if function should use kernel driver.

  Returns:  void
-----------------------------------------------------------------F-F*/
void FilterProcessAddresses(
    _In_ HANDLE process,
    _In_ DWORD64 value,
    _In_ DWORD valueSize,
    _In_ BOOL isDriverMode)
{
    FILE* pFile = NULL;
    FILE* pFilteredFile = NULL;  // Used by `filter` command. It contains filtered addresses
    PVOID address = NULL;
    DWORD64 processValue = 0;
    BOOL result = FALSE;

    if (fopen_s(&pFilteredFile, FILTERED_FILE_NAME, "w"))
        throw std::runtime_error("Failed to open output pFile");
    if (fopen_s(&pFile, FILE_NAME, "r"))
        throw std::runtime_error("Failed to open filter pFile");

    // Read addresses from `pFile`, compare their values to `value`. Matching one write to
    // `pFilteredFile`.
    while (fscanf_s(pFile, "%p", &address) != EOF) {
        LOG(INFO) << "address: " << address;
        if (isDriverMode == FALSE) {
            result = ReadProcessMemory(
                process,
                address,
                &processValue,
                valueSize,
                NULL);
        }
        else {
            result = ReadProcessMemoryDrivered(
                process,
                address,
                &processValue,
                valueSize,
                NULL);
        }

        if (result) {
            LOG(INFO) << "processValue: " << processValue;
            if (memcmp(&processValue, &value, valueSize) == 0) {
                fprintf(pFilteredFile, "%p\n", address);
            }
        }
        else {
            // ReadProcessMemory() could return 0 if it cannot open process or
            // it cannot read specified virtual memory range.
        }
    }

    if (pFile)
        fclose(pFile);
    if (pFilteredFile)
        fclose(pFilteredFile);

    // Replace FILE_NAME with FILTERED_FILE_NAME, so afterall only main file will be existing.
    CopyFileA(FILTERED_FILE_NAME, FILE_NAME, FALSE);
    DeleteFileA(FILTERED_FILE_NAME);
}

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: WriteProcessAddresses()

  Summary:  Writes to specified process addresses using specified value.
            It takes addresses from file.

  Args:     _In_ HANDLE process,
                The process where addresses are pointing.
            _In_ DWORD64 value,
                Value to write.
            _In_ DWORD valueSize)
                Value size.
            _In_ BOOL isDriverMode
                Specify if function should use kernel driver.

  Returns:  void
-----------------------------------------------------------------F-F*/
void WriteProcessAddresses(
    _In_ HANDLE process,
    _In_ DWORD64 value,
    _In_ DWORD valueSize,
    _In_ BOOL isDriverMode)
{
    FILE* pFile = NULL;
    PVOID address = NULL;
    BOOL result = FALSE;

    if (fopen_s(&pFile, FILE_NAME, "r")) {
        throw std::runtime_error("Failed to open filter pFile");
    }

    while (fscanf_s(pFile, "%p", &address) != EOF) {
        if (isDriverMode) {
            result = WriteProcessMemory(
                process,
                address,
                &value,
                valueSize,
                NULL);
        }
        else {
            result = WriteProcessMemoryDrivered(
                process,
                address,
                &value,
                valueSize,
                NULL);
        }
        if (result) {
            LOG(INFO) << "address 0x" << std::hex << address << " is written";
        }
        else {
            throw std::runtime_error("Failed to write to specified process");
        }
    }

    if (pFile)
        fclose(pFile);
}

void OutputFile()
{
    FILE* pFile = NULL;
    
    if (fopen_s(&pFile, FILE_NAME, "r")) {
        throw std::runtime_error("Failed to open filter pFile");
    }

    CHAR line[512] = { 0 };
    while (fscanf_s(pFile, "%s\n", line, 512) != EOF) {
        printf("%s\n", line);
    }
    
    if (pFile)
        fclose(pFile);
}
