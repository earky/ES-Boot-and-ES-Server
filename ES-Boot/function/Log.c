#include "Log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "Serial.h"

char Log_Buffer[LOG_BUFFER_SIZE];
char header1 = 0xFE;
char header2 = 0xCD;

/**
 * @brief ���˷�����־
 * @param �ɱ����
 */
void Log(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	// ʹ��vsnprintf��ȫ��ʽ���ַ���
	vsnprintf(Log_Buffer, LOG_BUFFER_SIZE, format, args);

	va_end(args);
	
	SendPacketHeader(0x04, strlen(Log_Buffer));
	Serial_SendString(Log_Buffer);
}

