#include "BroadwaveClient.h"
#include "decl.h"
#include "protos.h"
#include "structs.h"



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

void tBroadwave::initialiseBroadwave()
{
#if defined VISUAL_STUDIO
  // Initialize Winsock.
  int iResult = WSAStartup(MAKEWORD(2,2), &this->wsaData);

  if (iResult != NO_ERROR)
  {
	cerr << "Error with WSAStartup while initialising Broadwave.\n";
  }
  else
  {
	cout << "Broadwave: WSAStarup() okay.\n";
  }
#endif

  // Create a socket.
  this->m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (this->m_socket == INVALID_SOCKET)
  {
#if defined VISUAL_STUDIO
	cerr << "Client: socket() - Error at socket(): " << WSAGetLastError() << "\n";
	WSACleanup();
#endif
	return;
  }
  else
  {
	cout << "Broadwave: socket() okay.\n";
  }

#if defined VISUAL_STUDIO
  u_long nonBlocking = 1;
  ioctlsocket(this->m_socket, FIONBIO, &nonBlocking);
#elif defined LINUX_GCC
  // make sure socket doesn't block  
  if (fcntl( this->m_socket, F_SETFL, FNDELAY ) == -1)  
	throw runtime_error ("fcntl on control socket");
#endif

  struct linger ld = linger ();  // zero it  

  // Don't allow closed sockets to linger  
  if (setsockopt(this->m_socket, SOL_SOCKET, SO_LINGER,  
                 (char *) &ld, sizeof ld ) == -1)  
	throw runtime_error ("setsockopt (SO_LINGER)");  

  int x = 1;  

  // Allow address reuse   
  if (setsockopt( this->m_socket, SOL_SOCKET, SO_REUSEADDR,  
                 (char *) &x, sizeof x ) == -1)  
	throw runtime_error ("setsockopt (SO_REUSEADDR)");  

  // Connect to a server.
  this->clientService.sin_family = AF_INET;
  this->clientService.sin_addr.s_addr = inet_addr(BROADWAVE_IP);
  this->clientService.sin_port = htons(BROADWAVE_PORT);
#if defined VISUAL_STUDIO
  if (connect(this->m_socket, (SOCKADDR*)&this->clientService, sizeof(this->clientService)) == SOCKET_ERROR)
  {
	cerr << "Client: connect() - Failed to connect.\n";
	WSACleanup();
#elif defined LINUX_GCC
	if (connect(this->m_socket, (sockaddr*)&this->clientService, sizeof(this->clientService)) == 0)
	  //if (connect(this->m_socket, &this->clientService, sizeof(this->clientService)) == 0)
	{
	  cerr << "Client: connect() - Failed to connect.\n";
#endif
		return;
	}

	this->initialised = true;

	// Login
	tBroadwave::loginBroadwave ();	
  }

  void tBroadwave::listenBroadwave ()
  {
	if (!this->initialised)
	{
	  return;
	}

	static vector<char> inbuf (1000); 

#if defined VISUAL_STUDIO
	int nRead = recv(this->m_socket, &inbuf [0], (int) inbuf.size(), 0);

	if (nRead == -1)
	{
	  if (errno != WSAEWOULDBLOCK)
	  {
		perror("read from broadwave");
	  }
	  return;
	}

	if (nRead <= 0)
	{
	  closesocket(this->m_socket);
	  cerr << "Connection to Broadwave [" << this->m_socket << "] closed " << endl;
	  this->m_socket = NO_SOCKET;
	  return;
	}
#elif defined LINUX_GCC
	int nRead = read(this->m_socket, &inbuf [0], (int)inbuf.size());

	if (nRead == -1) { 
	  if (errno != EWOULDBLOCK) { 
		perror ("read from broadwave");
	  }
	  return;  
	}

	if (nRead <= 0) {
	  close(this->m_socket); 
	  cout << "Connection " << this->m_socket << " closed" << endl;  
	  this->m_socket = NO_SOCKET;  
	  return; 
	}
#endif

	string input;

	input += string(&inbuf[0], nRead);

	while (true)
	{
	  string::size_type i = input.find("\n");
	  if (i == string::npos)
	  {
		break;
	  }

	  string sLine = input.substr(0, i);
	  input = input.substr(i + 1, string::npos);

	  // This is where telnet negotiations occur
	  if (sLine[0] == IAC)
	  {
		string MCCP;
		MCCP.push_back(IAC);
		MCCP.push_back(WILL);
		MCCP.push_back(COMPRESS2);
		if (sLine.compare(0, 3, MCCP))
		{
		  // This assumes that MCCP is not available.

		  // Send IAC WONT COMPRESS2 here
		}
		sLine.erase(0, 3);
	  }
	  else if (sLine[0] == '~') // System Command from Broadwave
	  {
		sLine.erase(0, 1);
		if (sLine == "loginok")
		{
		  cout << "Successfully logged in to Broadwave Server." << endl;
		}
		else if (sLine == "shutdown")
		{
		  string outstring = "The Broadwave Server has been remotely shut down.";

		  // PLACEHOLDER - Server Shutdown
		  // If we've entered this else if, then the broadwave server has been shut down.
		  // The string outstring contains that message - you'll need to use your MUD's
		  // code to broadcast it.

		  send_to_gods(outstring.c_str());


		}
		continue;
	  }

	  if (!sLine.empty())
	  {
		istringstream is (sLine);
		string subwave, actor, realm, message;

		is >> subwave >> actor >> ws;
		string remnants;
		getline(is, remnants);

		realm = remnants.substr(0, remnants.find('*'));
		message = remnants.substr(remnants.find('*')+1, string::npos);

		string outstring;
		if (realm == THIS_REALM)
		{
		  outstring = "[" + subwave + ":" + actor + "] " + message;
		}
		else
		{
		  outstring = "[" + subwave + ":" + actor + "(" + realm + ")] " + message;
		}
		send_to_gods(outstring.c_str());

	  }
	}
  }

  void tBroadwave::broadcastBroadwave (string actor, string subwave, string message)
  {
	if (!this->initialised)
	{
	  return;
	}

	string output = subwave + " " + actor + " " + message + "\n";
	while (this->m_socket != NO_SOCKET && !output.empty())
	{
	  int iLength = min<int> ((int) output.size(), 512);

#if defined VISUAL_STUDIO

	  int nWrite = send(this->m_socket, output.c_str(), iLength, 0);

	  if (nWrite < 0)
	  {
		if (errno != WSAEWOULDBLOCK)
		{
		  perror("send to broadwave");
		}
		return;
	  }

#elif defined LINUX_GCC
	  int nWrite = write(this->m_socket, output.c_str(), iLength);

	  if (nWrite < 0) 
	  {
		if (errno != EWOULDBLOCK) 
		{
		  perror ("send to broadwave");
		}  
		return;  
	  }
#endif

	  output.erase(0, nWrite);

	  if (nWrite < iLength)
	  {
		break;
	  }
	}
  }

  void tBroadwave::loginBroadwave ()
  {
	if (!this->initialised)
	{
	  return;
	}

	ostringstream outstream;
	outstream << "#" << BROADWAVE_USERNAME << " " << BROADWAVE_PASSWORD << "\n";
	string output = outstream.str();

	int iLength = min<int> ((int) output.size(), 512);

#if defined VISUAL_STUDIO

	int nWrite = send(this->m_socket, output.c_str(), iLength, 0);

	if (nWrite < 0)
	{
	  if (errno != WSAEWOULDBLOCK)
	  {
		perror("send to broadwave");
	  }
	  return;
	}

#elif defined LINUX_GCC
	int nWrite = write(this->m_socket, output.c_str(), iLength);

	if (nWrite < 0) 
	{
	  if (errno != EWOULDBLOCK) 
	  {
		perror ("send to broadwave");
	  }  
	  return;  
	}
#endif
  }

  void tBroadwave::systemBroadwave (string password, string remainder)
  {
	if (!this->initialised)
	{
	  return;
	}

	string output = "~" + password + " " + remainder + "\n";
	while (this->m_socket != NO_SOCKET && !output.empty())
	{
	  int iLength = min<int> ((int) output.size(), 512);

#if defined VISUAL_STUDIO

	  int nWrite = send(this->m_socket, output.c_str(), iLength, 0);

	  if (nWrite < 0)
	  {
		if (errno != WSAEWOULDBLOCK)
		{
		  perror("send to broadwave");
		}
		return;
	  }

#elif defined LINUX_GCC
	  int nWrite = write(this->m_socket, output.c_str(), iLength);

	  if (nWrite < 0) 
	  {
		if (errno != EWOULDBLOCK) 
		{
		  perror ("send to broadwave");
		}  
		return;  
	  }
#endif

	  output.erase(0, nWrite);

	  if (nWrite < iLength)
	  {
		break;
	  }
	}
  }
  