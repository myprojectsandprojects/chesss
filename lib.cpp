#include "lib.h"

bool ReadFile(const char *FileName, u8 **Contents, size_t *ContentsNumBytes)
{
	FILE *F = fopen(FileName, "rb"); // On Windows we need 'b', otherwise it doesnt read the whole file.
	if (!F)
	{
		fprintf(stderr, "Failed to open file: \"%s\"!\n", FileName);
		return false;
	}

	fseek(F, 0, SEEK_END);
	size_t FileSize = ftell(F);
	fseek(F, 0, SEEK_SET); // Back to the beginning...
	void *Memory = malloc(FileSize);
	if (!Memory)
	{
		fprintf(stderr, "Failed to allocate memory for file: \"%s\"!\n", FileName);
		if (fclose(F) == EOF)
		{
			fprintf(stderr, "Failed to close file: \"%s\"!\n", FileName);
		}
		return false;
	}

	// We expect file size to be the same it was when we last queried it (ftell()).
	size_t BytesRead = fread(Memory, sizeof(u8), FileSize, F);
	if (BytesRead != FileSize)
	{
		fprintf(stderr, "Failed to read file: \"%s\"!\n", FileName);
		if (fclose(F) == EOF)
		{
			fprintf(stderr, "Failed to close file: \"%s\"!\n", FileName);
		}
		free(Memory);
		return false;
	}

	if (fclose(F) == EOF)
	{
		fprintf(stderr, "Failed to close file: \"%s\"!\n", FileName);
		free(Memory);
		return false;
	}

	*Contents = (u8 *) Memory;
	*ContentsNumBytes = FileSize;

	return true;
}

bool WriteFile(const char *FileName, u8 *Contents, size_t NumBytes)
{
	FILE *F = fopen(FileName, "w");
	if (!F)
	{
		fprintf(stderr, "Failed to open file: \"%s\"!\n", FileName);
		return false;
	}

	size_t BytesWritten = fwrite((void *) Contents, sizeof(u8), NumBytes, F);
	if (BytesWritten != NumBytes)
	{
		fprintf(stderr, "Failed to write file: \"%s\"!\n", FileName);
		if (fclose(F) == EOF)
		{
			fprintf(stderr, "Failed to close file: \"%s\"!\n", FileName);
		}
		return false;
	}

	if (fclose(F) == EOF)
	{
		fprintf(stderr, "Failed to close file: \"%s\"!\n", FileName);
		return false;
	}

	printf("Successfully wrote %lu bytes to \"%s\"\n", BytesWritten, FileName);
	return true;
}