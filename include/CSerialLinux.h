#ifndef __CSERIALLINUX_H__
#define __CSERIALLINUX_H__

#include <pthread.h>

typedef unsigned int uint;

class CSerialLinux
{														 
public:
	// contruction and destruction
	CSerialLinux();
	virtual		~CSerialLinux();

	// port initialisation											
	bool		InitPort(const char *portname, uint baud = 19200, uint nBufferSize = 512, uint  readbuffersize = 512);

	// write to port
	void		WriteChar(char c);
	void		WriteString(char* string);

	// start/stop comm watching
	bool		StartMonitoring();
	bool		StopMonitoring();

	// data received
	char*				m_sReceiveBuffer;
	uint				m_nReceiveBufferSize;
	uint				m_nReceived;

	// port
	int 				m_nPortID;
	char*				m_sPortName;
	uint				m_nBaudRate;
	char*				m_sWriteBuffer;
	uint				m_nWriteBufferSize;
	
protected:
	
	// thread
	pthread_t			m_Thread;
};

#endif __CSERIALLINUX_H__
