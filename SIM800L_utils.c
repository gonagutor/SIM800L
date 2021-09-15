#include "SIM800L.h"
#include <windows.h>

HANDLE hComm;
DCB dcbSerialParams = {0};	 // Initializing DCB structure
COMMTIMEOUTS timeouts = {0}; //Initializing timeouts structure

int initialize_local_port()
{

	hComm = CreateFileA("\\\\.\\COM7",				  //port name
						GENERIC_READ | GENERIC_WRITE, //Read/Write
						0,							  // No Sharing
						NULL,						  // No Security
						OPEN_EXISTING,				  // Open existing port only
						0,							  // Non Overlapped I/O
						NULL);						  // Null for Comm Devices
	if (hComm == INVALID_HANDLE_VALUE)
	{
		printf("[ERROR] Serial port could not be opened\n");
		return (1);
	}
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hComm, &dcbSerialParams))
	{
		printf_s("[ERROR] Error to Get the Com state\n");
		return (1);
	}
	dcbSerialParams.BaudRate = CBR_9600;   //BaudRate = 9600
	dcbSerialParams.ByteSize = 8;		   //ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT; //StopBits = 1
	dcbSerialParams.Parity = NOPARITY;	   //Parity = None
	if (!SetCommState(hComm, &dcbSerialParams))
	{
		printf_s("\n[ERROR] Error to Setting DCB Structure\n");
		return (1);
	}
	//Setting Timeouts
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(hComm, &timeouts))
	{
		printf_s("\nError to Setting Time outs");
		return (1);
	}
	return (0);
}

void destroy_local_port()
{
	CloseHandle(hComm);
}

/**
 * Placeholder function to send data to console
 * @param data String to send
 **/
void send_data(const char *data)
{
	printf("[SENDING] %s\n", data);
	unsigned long written = 0;
	BOOL status = WriteFile(hComm,		  // Handle to the Serial port
							data,		  // Data to be written to the port
							strlen(data), //No of bytes to write
							&written,	  //Bytes written
							NULL);
	if (!status)
		printf("[ERROR] Failed to write to port\n");
	if (written != strlen(data))
		printf("[ERROR] Failed to write all bytes to port\n");
}

/**
 * Placeholder function to read data from console
 **/
unsigned long read_data(char **data)
{
	DWORD dwEventMask;
	char ReadData;
	DWORD NoBytesRead;
	unsigned long loop = 0;
	char SerialBuffer[1024] = {0};
	if (!SetCommMask(hComm, EV_RXCHAR))
	{
		printf_s("\nError to in Setting CommMask\n\n");
		return (0);
	}
	if (!WaitCommEvent(hComm, &dwEventMask, NULL))
	{
		printf_s("\nError! in Setting WaitCommEvent()\n\n");
		return (0);
	}

	do
	{
		ReadFile(hComm, &ReadData, sizeof(ReadData), &NoBytesRead, NULL);
		SerialBuffer[loop] = ReadData;
		++loop;
	} while (NoBytesRead > 0);
	SerialBuffer[loop] = '\0';
	--loop;
	printf("[INFO] Number of bytes received = %lu\n", loop);
	*data = SerialBuffer;
	return (loop);
}

int strlocatechar(const char *str, const char chr)
{
	for (int i = 0; i < strlen(str); i++)
		if (str[i] == chr)
			return (i);
	return (-1);
}

int strstartswith(const char *str, const char *search)
{
	size_t lenstr;
	size_t lensearch;

	lenstr = strlen(str);
	lensearch = strlen(search);
	return lenstr < lensearch ? 0 : memcmp(search, str, lensearch) == 0;
}

void strcut(char **str, int upto)
{
	char *cpy;

	cpy = malloc(upto + 1);
	memcpy(cpy, *str, upto);
	cpy[upto + 1] = 0;
	free(*str);
	*str = cpy;
}

void strtrim(char **str, int characters)
{
	char *cpy;

	cpy = malloc(strlen(*str) - characters + 1);
	printf("%s", *str);
	memcpy(cpy, *str + characters, strlen(*str) - characters);
	cpy[(int)strlen(*str) - characters + 1] = 0;
	free(str);
	*str = cpy;
}

/**
 * Checks wether given url is http or https
 * @param url Url full string
 * @return true if url is https or http. If now string given defaults to https.
 **/
int is_url_ssl(char *url)
{
	if (strstr(url, "https://") != NULL)
		return (1);
	if (strstr(url, "http://") != NULL)
		return (0);
	return (1);
}

char *trim_url(char *url)
{
	char *ret_url;
	int i;

	i = 0;
	ret_url = malloc(strlen(url));
	memcpy(ret_url, url, strlen(url));
	ret_url[strlen(url)] = 0;
	if (strstartswith(url, "https://"))
		strtrim(&ret_url, 8);
	else if (strstartswith(url, "http://"))
		strtrim(&ret_url, 7);
	if (strstartswith(ret_url, "www."))
		strtrim(&ret_url, 4);
	if (strlocatechar(ret_url, '\\') != -1)
		strcut(&ret_url, strlocatechar(ret_url, '\\'));
	return (ret_url);
}

char *get_url_location(char *url)
{
	char *ret_url;

	ret_url = malloc(strlen(url) + 1);
	memcpy(ret_url, url, strlen(url));
	ret_url[strlen(url) + 1] = 0;
	strtrim(&ret_url, strlocatechar(ret_url, '\\'));
	return (ret_url);
}