#include "CSerialLinux.h"

#define BAUDRATE 9600            
#define MODEMDEVICE "/dev/ttyACM0"

int main(void)
{
    CSerialLinux *port = new CSerialLinux();
    port->InitPort(MODEMDEVICE, 9600);
    port->StartMonitoring();


    port->WriteChar('1');
    port->WriteChar('2');
    port->WriteChar('1');
    port->WriteChar('3');

    port->StopMonitoring();

    return 0;
}