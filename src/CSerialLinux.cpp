/*
**	FILENAME			CSerialLinux.cpp
**
**	PURPOSE				This class can read, write and watch one serial port.
**						It sends messages to its owner when something happends on the port
**						The class creates a thread for reading and writing so the main
**						program is not blocked.
**
**	CREATION DATE		15-09-1997
**	LAST MODIFICATION	12-11-1997
**	YIFAN MODIFICATION	12-27-2016
**
**	AUTHOR				Remon Spekreijse
**	EDITOR				Yifan Hou
**
**
*/

#include "CSerialLinux.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

volatile bool threadAlive;



//
// Constructor
//
CSerialLinux::CSerialLinux()
{
	m_nPortID = -1;
}

//
// Delete dynamic memory
//
CSerialLinux::~CSerialLinux()
{
}

//
// Initialize the port.
//
bool CSerialLinux::InitPort(const char *portname,	// port name, e.g. "/dev/ttyACM0"
						   uint  baud,			    // baudrate
						   uint  writebuffersize,	// size to the writebuffer
						   uint  readbuffersize)	// size of the read buffer
{
	threadAlive          = false;
	m_nBaudRate          = baud;
	m_nWriteBufferSize   = writebuffersize;
	m_nReceiveBufferSize = readbuffersize;
	m_nReceived          = 0;

	if (m_sPortName != NULL)
		delete [] m_sPortName;
	m_sPortName = new char[(unsigned)strlen(portname)+1];
	strcpy(m_sPortName, portname);

	if (m_sWriteBuffer != NULL)
		delete [] m_sWriteBuffer;
	m_sWriteBuffer = new char[writebuffersize];

	if (m_sReceiveBuffer != NULL)
		delete [] m_sReceiveBuffer;
	m_sReceiveBuffer = new char[m_nReceiveBufferSize];

	// // if the port is already opened: close it
	// if (m_nPortID >= 0)

	// initialize the port settings
    struct termios newtio;

	// open the port
    // Open modem device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    m_nPortID = open(portname, O_RDWR | O_NOCTTY ); 
    if (m_nPortID < 0) {
        printf("Error opening %s: %d\n", portname, errno);
        return false;
    }
    
    /* clear struct for new port settings */
    bzero(&newtio, sizeof(newtio)); 


	int BAUDRATE = 0;
	switch (m_nBaudRate)
	{
        case 9600 : 
        	BAUDRATE = B9600;
        	break;
        case 19200 : 
        	BAUDRATE = B19200;
        	break;
        case 38400 : 
        	BAUDRATE = B38400;
        	break;
        case 57600 : 
        	BAUDRATE = B57600;
        	break;
        case 115200 : 
        	BAUDRATE = B115200;
        	break;
        case 230400 : 
        	BAUDRATE = B230400;
        	break;
    	default:
    		printf("Wrong format in Baudrate: %d\n", m_nBaudRate);
	}
	/* 
	  BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	  CRTSCTS : output hardware flow control (only used if the cable has
	            all necessary lines. See sect. 7 of Serial-HOWTO)
	  CS8     : 8n1 (8bit,no parity,1 stopbit)
	  CLOCAL  : local connection, no modem contol
	  CREAD   : enable receiving characters
	*/
 	newtio.c_cflag = ((speed_t)BAUDRATE) | CRTSCTS | CS8 | CLOCAL | CREAD;
 	/*
 	  IGNPAR  : ignore bytes with parity errors
 	  ICRNL   : map CR to NL (otherwise a CR input on the other computer
 	            will not terminate input)
 	  otherwise make device raw (no other input processing)
 	*/
 	newtio.c_iflag = IGNPAR | ICRNL;
 	/*
 	 Raw output.
 	*/
 	newtio.c_oflag = 0;
 	/*
 	  ICANON  : enable canonical input
 	  disable all echo functionality, and don't send signals to calling program
 	*/
 	newtio.c_lflag = ICANON;

 	/* 
 	  initialize all control characters 
 	  default values can be found in /usr/include/termios.h, and are given
 	  in the comments, but we don't need them here
 	*/
 	newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
 	newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
 	newtio.c_cc[VERASE]   = 0;     /* del */
 	newtio.c_cc[VKILL]    = 0;     /* @ */
 	newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
 	newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
 	newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
 	newtio.c_cc[VSWTC]    = 0;     /* '\0' */
 	newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
 	newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
 	newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
 	newtio.c_cc[VEOL]     = 0;     /* '\0' */
 	newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
 	newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
 	newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
 	newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
 	newtio.c_cc[VEOL2]    = 0;     /* '\0' */

 	/* 
 	  now clean the modem line and activate the settings for the port
 	*/
 	 tcflush(m_nPortID, TCIFLUSH);
 	 tcsetattr(m_nPortID,TCSANOW,&newtio);


	printf("Serial port is opened at %s.\nStartmonitor is ready to use.\n", m_sPortName);

	return true;
}

//
//  The CommThread Function.
//
void* Monitor(void* pParam)
{
	// // Cast the void pointer passed to the thread back to
	// // a pointer of CSerialLinux class
	CSerialLinux *port = (CSerialLinux*)pParam;
	// // Set the status variable in the dialog class to
	// // TRUE to indicate the thread is running.
	// threadAlive = TRUE;	
		
	threadAlive = true;
	printf("Waiting...\n");

	 while (threadAlive == true) {     
	 /* read blocks program execution until a line terminating character is 
	    input, even if more than 255 chars are input. If the number
	    of characters read is smaller than the number of chars available,
	    subsequent reads will return the remaining chars. res will be set
	    to the actual number of characters actually read */
	    port->m_nReceived = read(port->m_nPortID,port->m_sReceiveBuffer,port->m_nReceiveBufferSize); 
	    port->m_sReceiveBuffer[port->m_nReceived] = 0;             /* set end of string, so we can printf */
	    printf(":%s:%d\n", port->m_sReceiveBuffer, port->m_nReceived);
	 }
	printf("Monitor Stopped.\n");
}

//
// start comm watching
//
bool CSerialLinux::StartMonitoring()
{
	if (threadAlive==true)
		return true;

	int rc = pthread_create(&m_Thread, NULL, Monitor, this);
	if (rc){
		printf("Error:unable to create thread.\n");
		return false;
	}

	return true;	
}

//
// Suspend the comm thread
//
bool CSerialLinux::StopMonitoring()
{
	if (threadAlive == false)
		return true;

	threadAlive == false;
	return true;	
}


//
// Write a character.
//
void CSerialLinux::WriteChar(char c)
{
    int wlen = write(m_nPortID, &c, 1);
    if (wlen != 1) {
        printf("Error from write: %d, %d\n", wlen, errno);
    }
    tcdrain(m_nPortID);    /* delay for output */
}
 
//
// Write a string to the port
//
void CSerialLinux::WriteString(char* str)
{		
	int len = strlen(str);
	int wlen = write(m_nPortID, str, len);
	if (wlen != len) {
	    printf("Error from write: %d, %d\n", wlen, errno);
	}
	tcdrain(m_nPortID);    /* delay for output */
}