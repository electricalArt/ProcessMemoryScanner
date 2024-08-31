#include <windows.h>
#include <easylogging++.h>
#include <tclap/CmdLine.h>
#include "SharedStuffLib.h"

#define PRODUCT_NAME L"ProcessMemoryScanner"
#define FILE_NAME L"addresses.txt"
#define CHUNK_SIZE (SIZE_T)4096
#define USER_MODE_VM_MAX_ADDRESS (SIZE_T)0x7FFF'FFFFFFFF

INITIALIZE_EASYLOGGINGPP

DWORD64 ParseValue(std::string valueStr, std::string valueType, PDWORD pValueSize)
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
		value = (DWORD64)std::stof(valueStr);
		*pValueSize = sizeof(FLOAT);
	}
	else if (valueType == "double") {
		value = (DWORD64)std::stod(valueStr);
		*pValueSize = sizeof(DOUBLE);
	}
	else {
		throw TCLAP::ArgParseException("--value-type");
	}
	return value;
}

void SearchProcessAddresses(HANDLE process, DWORD64 value, DWORD valueSize, FILE* file)
{
	BYTE buff[CHUNK_SIZE] = { 0 };
	MEMORY_BASIC_INFORMATION memoryInfo = { 0 };
	for (SIZE_T i = 0x0; i < USER_MODE_VM_MAX_ADDRESS; i += CHUNK_SIZE) {
		if (ReadProcessMemory(
				process,
				(LPCVOID)i,
				&buff,
    			CHUNK_SIZE,
    			NULL)) {
			LOG_N_TIMES(5, INFO) << "buff contains (as DWORD64): " << *(PDWORD64)buff;
			for (SIZE_T buff_i = 0x0; buff_i < CHUNK_SIZE; buff_i += valueSize) {
				if (memcmp(buff + buff_i, &value, valueSize) == 0) {
					//LOG(INFO) << "Found: " << i + buff_i;
					printf("Found: %p", i + buff_i);
				}
			}
		}
		LOG_EVERY_N(500'000, INFO) << "Searching status: " << std::fixed << std::setprecision(2)
			<< (FLOAT)i / USER_MODE_VM_MAX_ADDRESS << "%";
	}
}

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Summary:  It initializes the logger, check customer validtion and,
			if everything is OK, starts `CheatLib` framework.

  Args:		ProcessMemoryScanner [<command>] <value> [<options>]

				The following commands are available:
					search
					filter
					write

				The following options are available:
					--process-id <process_id>
					--value-type   could be:
						int 
						long 
						float 
						double

  Returns:  0
				If success.
			non-zero valie
				If fail.
-----------------------------------------------------------------F-F*/
int main(int argc, char* argv[])
{
	DWORD result = 0;
	HANDLE hProcess = NULL;
	FILE* pFile = NULL;

	try {
#ifdef _DEBUG
		ConfigureLoggers(PRODUCT_NAME, true);
#else
		ConfigureLoggers(PRODUCT_NAME, false);
#endif

		TCLAP::CmdLine cmd(
			"ProcessMemoryScanner.exe scans specified process for specified value.",
			' ',
			"1.0");
		TCLAP::UnlabeledValueArg<std::string> commandArg(
			"command", "Command to perform", TRUE,
			"search | filter | write",
			"", cmd);
		TCLAP::UnlabeledValueArg<std::string> valueStrArg(
			"value", "Value", TRUE,
			"500", "", cmd );
		TCLAP::ValueArg<DWORD> processIdArg(
			"p", "process-id", "process id", TRUE,
			1224,
			"", cmd );
		TCLAP::ValueArg<std::string> valueTypeArg(
			"v", "value-type", "Value type", TRUE,
			"int32 | int64 | float | double",
			"", cmd);
		cmd.parse(argc, argv);

		std::string command = commandArg.getValue();
		DWORD processId = processIdArg.getValue();
		std::string valueType = valueTypeArg.getValue();
		DWORD valueSize = 0;
		// DWORD64 can hold every type value
		DWORD64 value = ParseValue(valueStrArg.getValue(), valueType, &valueSize);
		LOG(INFO) << "command: " << command;
		LOG(INFO) << "value as DWORD64: " << value;
		LOG(INFO) << "processIdArg: " << processId;
		LOG(INFO) << "valueType: " << valueType;
		LOG(INFO) << "valueSize: " << valueSize;

		hProcess = OpenProcess(
			PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION,
			FALSE,
			processId);
		if (hProcess == NULL) {
			throw std::runtime_error("Failed to open process");
		}
		LOG(INFO) << "hProcess: " << hProcess;

		if (_wfopen_s(&pFile, FILE_NAME, L"w+")) {
			throw std::runtime_error("Failed to open output pFile");
		}

		if (command == "search") {
			SearchProcessAddresses(hProcess, value, valueSize, pFile);
		}
		else if (command == "filter") {
			// WIP
		}
		else if (command == "write") {
			// WIP
		}
		else {
			throw TCLAP::ArgException("Invalid command");
		}

	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		result = 2;
	} catch (...) {
		result = 3;
	}

	printf("Press Enter to exit...");
	getchar();

	if (pFile) {
		fclose(pFile);
	}
	if (hProcess) {
		CloseHandle(hProcess);
	}

	return 0;
}
