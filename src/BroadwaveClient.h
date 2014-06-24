#pragma once

#include <fcntl.h>  
#include <signal.h>
#include <time.h>
#include <errno.h> 
#include <ctime>
#include <cstdlib>

#include <string>  
#include <list>  
#include <map>  
#include <set>  
#include <vector>  
#include <stdexcept>  
#include <fstream>  
#include <iostream>  
#include <sstream>  
#include <ios>  
#include <iterator>
#include <algorithm>

using namespace std;

#if defined(_MSC_VER) && (_MSC_VER >= 1400) // Visual Studio 2005 and up
	#define VISUAL_STUDIO
#elif defined(__GNUC__) && (__GNUC__ >= 3) // GCC Version 3 and up
	#define LINUX_GCC 
#endif

#if defined VISUAL_STUDIO
	#include <winsock2.h>
#elif defined LINUX_GCC
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <unistd.h>
#define INVALID_SOCKET (int)(~0)
#endif

static const int NO_SOCKET = -1; 

// Don't change these two!
#define BROADWAVE_IP "149.106.192.52" 
#define BROADWAVE_PORT 4242

#define BROADWAVE_USERNAME "Atonement"
#define BROADWAVE_PASSWORD "testing1234"

#define THIS_REALM "Atonement"

/*
#define TELOPT_MXP        	'\x5B'
#define SE                  '\xF0'
#define GA                 	'\xF9'
#define SB                 	'\xFA'
#define WILL               	'\xFB'
#define WONT               	'\xFC'
#define DO                 	'\xFD'
#define DONT               	'\xFE'
#define IAC                	'\xFF'
#define ESC                 '\x1B'
#define COMPRESS2 			 86
*/
class tBroadwave
{
protected:
#if defined VISUAL_STUDIO
	WSADATA wsaData;
	SOCKET m_socket;
#elif defined LINUX_GCC
	int m_socket;
#endif
	sockaddr_in clientService;

	bool initialised;

public:



	tBroadwave () : initialised (false) {this->initialiseBroadwave();}
	~tBroadwave () {
#if defined VISUAL_STUDIO
		closesocket(this->m_socket);
#elif defined LINUX_GCC
	close(this->m_socket);
#endif
	}
	
	void initialiseBroadwave ();
	void listenBroadwave ();
	void broadcastBroadwave (string actor, string subwave, string message);
	void loginBroadwave ();
	void systemBroadwave (string password, string remainder);
};
