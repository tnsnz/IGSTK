/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkSerialCommunicationForWindows.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISIS Georgetown University. All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __igstkSerialCommunicationForWindows_h
#define __igstkSerialCommunicationForWindows_h

#include <windows.h>

#include "igstkSerialCommunication.h"

namespace igstk
{

/** \class SerialCommunicationForWindows
 * 
 * \brief This class implements Windows specific methods for 32-bit 
 *        communication over a Serial Port(RS-232 connection).
 * \ingroup Communication
 * \ingroup SerialCommunication
 */

class SerialCommunicationForWindows : public SerialCommunication
{
public:

  /** Return value type for interface functions */ 
  typedef SerialCommunication::ResultType ResultType;

  typedef SerialCommunicationForWindows  Self;
  typedef itk::SmartPointer<Self>        Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /**  Run-time type information (and related methods). */
  igstkTypeMacro(SerialCommunicationForWindows, SerialCommunication);

  /** Method for creation of a reference counted object. */
  igstkNewMacro(Self);  

protected:

  /** Windows file handle type */
  typedef HANDLE HandleType;

  /** Constructor */
  SerialCommunicationForWindows();

  /** Destructor */
  ~SerialCommunicationForWindows();

  /** Opens serial port for communication; */
  virtual ResultType InternalOpenPort( void );

  /** Sets up communication on the open port as per the communication
      parameters. */
  virtual ResultType InternalSetTransferParameters( void );

  /** Closes serial port  */
  virtual ResultType InternalClosePort( void );

  /** Send a serial break */
  virtual void InternalSendBreak( void );

  /** Sleep for the number of milliseconds stored in m_SleepPeriod */
  virtual void InternalSleep( void );

  /** Purge the input and output buffers */
  virtual void InternalPurgeBuffers( void );

  /** Write data */
  virtual void InternalWrite( void );

  /** Read data */
  virtual void InternalRead( void );

  /** Print object information. */
  virtual void PrintSelf( std::ostream& os, itk::Indent indent ) const; 

private:

  HandleType      m_PortHandle;     // com port handle

  COMMTIMEOUTS    m_SaveTimeout;

  DCB             m_SaveDCB;
};

} // end namespace igstk

#endif // __igstkSerialCommunicationForWindows_h