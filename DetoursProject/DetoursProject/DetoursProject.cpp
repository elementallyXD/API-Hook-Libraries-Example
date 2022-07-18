#include <windows.h>
#include <stdio.h>

int main()
{
	printf("DetoursProject.exe: Starting.\n");

	HANDLE hFile = CreateFileW(
		L"file.txt",
		GENERIC_WRITE,
		NULL,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile != INVALID_HANDLE_VALUE && hFile != NULL)
	{
		printf("Created file with handle: 0x%p\n", hFile);
	}

	CloseHandle(hFile); // Calling to the hooked function
	CloseHandle(hFile); // Calling to the hooked function

	printf("DetoursProject.exe: Done.\n");

	return 0;
}
