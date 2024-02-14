/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkSocketCommunicationForWindows.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

/* standard include files */
#include <iostream>
#include <stdio.h>

#include "igstkConfigure.h"
#include "igstkSocketCommunicationForWindows.h"
#include "igstkPulseGenerator.h"

namespace igstk
{ 

SocketCommunicationForWindows::SocketCommunicationForWindows() : m_StateMachine(this)
{
    m_pGetInternalRead = NULL;
}

SocketCommunicationForWindows::~SocketCommunicationForWindows()
{
} 


SocketCommunicationForWindows::ResultType
SocketCommunicationForWindows::InternalOpenPort( void )
{
    cout << "InternalOpenPort( ) called..." << endl;
  ResultType eResult = FAILURE;
  const auto serial = GetSerialNumber();
  const auto port = GetPortNumber();
  cout << "S/N: " << serial << ", port: " << port << endl;
  init();

  
  if (!serial.empty() && !port.empty())
  {
	  eResult = connect(serial, port) ? SUCCESS : FAILURE;
  }

  unsigned int timeoutPeriod = this->GetTimeoutPeriod();

  igstkLogMacro( DEBUG, "InternalOpenPort succeeded...\n" );

  return eResult;
}


SocketCommunicationForWindows::ResultType
SocketCommunicationForWindows::InternalClosePort( void )
{
  disconnect();

  return SUCCESS;
}


void SocketCommunicationForWindows::InternalSleep(int milliseconds)
{
  PulseGenerator::Sleep(milliseconds);
}


SocketCommunicationForWindows::ResultType
SocketCommunicationForWindows::InternalWrite( const char *data,
                                              unsigned int n )
{
    int i = send(m_Socket, data, n, 0);
    cout << "InternalWrite(): " << "writting( " << n << " ), written( " << i << " )" << endl;
    return SUCCESS;
}


SocketCommunicationForWindows::ResultType
SocketCommunicationForWindows::InternalRead( char *data,
                                             unsigned int n,
                                             unsigned int &bytesRead )
{
    //cout << "SocketCommunicationForWindows::InternalRead() called..." << endl;
    char lastChar = '\0';
    std::string response;

#if 0
	while (lastChar != '\r')
	{
		read(&lastChar, 1);
		response += lastChar;
	}
#else
	char terminationCharacter = this->GetReadTerminationCharacter();
	bool useTerminationCharacter = this->GetUseReadTerminationCharacter();

	while (useTerminationCharacter && lastChar != terminationCharacter)
    {
        read(&lastChar, 1);
        response += lastChar;
    }
#endif
    strcpy(data, response.c_str());

    bytesRead = strlen(data);

    //cout << "SocketCommunicationForWindows::InternalRead() called... read = " << response << endl;

    if (m_pGetInternalRead)
        m_pGetInternalRead(response);

    return SUCCESS;
}


void SocketCommunicationForWindows::PrintSelf( std::ostream& os,
                                               itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

//  os << indent << "PortHandle: " << m_PortHandle << std::endl;
//  os << indent << "OldTimeoutPeriod: " << m_OldTimeoutPeriod << std::endl;
}

void SocketCommunicationForWindows::init()
{
    WSAStartup(MAKEWORD(2, 2), &m_sWSADATA);
    m_Socket = INVALID_SOCKET;
    m_bConnected = false;
    m_Ip4Address[0] = 0;
}

bool SocketCommunicationForWindows::connect(std::string serialNumber, std::string portNumber)
{
    // Define socket options in the 'addrinfo' block
    addrinfo addressInfo;
    memset(&addressInfo, 0, sizeof(addressInfo));
    addressInfo.ai_family = AF_INET;
    addressInfo.ai_socktype = SOCK_STREAM;
    addressInfo.ai_protocol = IPPROTO_TCP;
    // Setup a TCP socket using the given hostname and port
    addrinfo* aiPointer = NULL, * pai;
	int addrinforesult = getaddrinfo(serialNumber.c_str(), portNumber.c_str(), &addressInfo, &aiPointer);
    if (addrinforesult != 0)
    {
        std::cerr << "getaddrinfo Error code " << addrinforesult << " (" << gai_strerror(addrinforesult) << ")" << std::endl;
        return false;
    }

    for (pai = aiPointer; pai != NULL; pai = pai->ai_next)
    {
        //Initialize socket
        m_Socket = socket(pai->ai_family, pai->ai_socktype, pai->ai_protocol);
        // On Windows, SOCKET type is unsigned, so a negative number cannot indicate invalid socket
        // so "all bits set" indicates invalid sockets:  #define INVALID_SOCKET (SOCKET)(~0)
        if (m_Socket == INVALID_SOCKET)
        {
            continue;
        }

        //Initialize connection
        m_bConnected = ::connect(m_Socket, pai->ai_addr, (socklen_t)pai->ai_addrlen) >= 0;
        if (m_bConnected)
        {
            // Convert the IP address to a character array
            inet_ntop(AF_INET, (PVOID) & ((const sockaddr_in*)aiPointer->ai_addr)->sin_addr, m_Ip4Address, INET_ADDRSTRLEN);
            break;
        }
        disconnect();
    }

	return m_bConnected;
}

void SocketCommunicationForWindows::disconnect()
{
	if (m_Socket != INVALID_SOCKET)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
}

int SocketCommunicationForWindows::read(char* buffer, int length) const
{
    int result = recv(m_Socket, buffer, length, MSG_WAITALL);
    return result;
}

void SocketCommunicationForWindows::RegisterInternalRead(void(*cbFunc)(std::string))
{
    if (cbFunc)
    {
        m_pGetInternalRead = cbFunc;
    }
}

} // end namespace igstk