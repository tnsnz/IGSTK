/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkSocketCommunicationForWindows.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __igstkSocketCommunicationForWindows_h
#define __igstkSocketCommunicationForWindows_h

#include "igstkSocketCommunication.h"

#include <winsock2.h> // for Windows Socket API (WSA)
#include <ws2tcpip.h> // for getaddrinfo() etc...

namespace igstk
{

class SocketCommunicationForWindows : public SocketCommunication
{
public:
 
  /** Macro with standard traits declarations. */
  igstkStandardClassTraitsMacro( SocketCommunicationForWindows, 
                                 SocketCommunication )

public:

  /** Return value type for interface functions */ 
  typedef SocketCommunication::ResultType ResultType;

protected:
  /** Constructor */
  SocketCommunicationForWindows();

  /** Destructor */
  ~SocketCommunicationForWindows();

  /** Opens serial port for communication; */
  virtual ResultType InternalOpenPort( void );

//   /** Sets up communication on the open port as per the communication
//       parameters. */
//   virtual ResultType InternalUpdateParameters( void );

  /** Closes serial port  */
  virtual ResultType InternalClosePort( void );

  /** Sleep for the number of milliseconds specified */
  virtual void InternalSleep( int milliseconds );

  /** Write data */
  virtual ResultType InternalWrite( const char *message,
                                    unsigned int numberOfBytes );

  /** Read data */
  virtual ResultType InternalRead( char *data, unsigned int numberOfBytes,
                                   unsigned int &bytesRead );

  /** Print object information. */
  virtual void PrintSelf( std::ostream& os, itk::Indent indent ) const; 

  virtual void ClearBuffers();

  void init();

  bool connect(std::string serialNumber, std::string portNumber);

  void disconnect();

  int read(char* buffer, int length) const;

private:
    bool m_bConnected;
    char m_Ip4Address[INET_ADDRSTRLEN];
    WSADATA m_sWSADATA;
    SOCKET m_Socket;

public:
    void RegisterInternalRead(void(*cbFunc)(std::string)) override;

protected:
    void(*m_pGetInternalRead)(std::string);

};

} // end namespace igstk

#endif // __igstkSocketCommunicationForWindows_h