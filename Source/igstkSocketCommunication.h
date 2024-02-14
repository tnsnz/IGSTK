/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkSerialCommunication.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __igstkSocketCommunication_h
#define __igstkSocketCommunication_h

// Disabling warning C4355: 'this' : used in base member initializer list
#if defined(_MSC_VER)
#pragma warning ( disable : 4355 )
#endif

#include "itkObject.h"
#include "itkEventObject.h"
#include "igstkLogger.h"
#include "itkStdStreamLogOutput.h"

#include "igstkMacros.h"
#include "igstkEvents.h"
#include "igstkCommunication.h"
#include "igstkStateMachine.h"


namespace igstk
{

/** \class SocketCommunication
 * 
 * \brief This class implements communication over a socket
 *        via tcp/ip
 *
 * This class provides a common interface for the interactions with a serial
 * port independently of the platform. The actual interactions are implemented
 * on derived classes that are platform specific. This class is not intended
 * for being instantiated directly, instead, the derived classes should be
 * used.
 * 
 * \image html  igstkSocketCommunication.png 
 *               "Socket Communication State Machine Diagram"
 *
 * \image latex igstkSocketCommunication.eps 
 *               "Socket Communication State Machine Diagram"
 * 
 *
 *
 *
 * \ingroup Communication
 */

class SocketCommunication : public Communication
{
public:

  typedef Communication::ResultType      ResultType;

  /** Standard traits of a basic class */
  igstkStandardClassBasicTraitsMacro( SocketCommunication, Communication );

  /** Customized New method that will return the implementation of
   * SocketCommunication that is appropriate for this platform.  
   * \sa SocketCommunicationForWindows 
   * \sa SocketCommunicationForPosix
   */
  static Pointer New(void);

  igstkSetMacro( SerialNumber, std::string );
  igstkGetMacro( SerialNumber, std::string );

  igstkSetMacro( PortNumber, std::string );
  igstkGetMacro( PortNumber, std::string );

  /** Set the name of the file into which the data stream is recorded. */
  void SetCaptureFileName(const char* filename);
  /** Get the filename into which the data stream is recorded. */
  const char* GetCaptureFileName() const;

  /** Set whether to record the data. */
  igstkSetMacro( Capture, bool );
  /** Get whether the data is being recorded. */
  igstkGetMacro( Capture, bool );

  /** Update the communication parameters, in case you need to change
   *  the baud rate, handshaking, timeout, etc. after opening the port */
  ResultType UpdateParameters( void );

  /** The method OpenCommunication sets up communication as per the data
   *  provided. */
  ResultType OpenCommunication( void );

  /** The method CloseCommunication closes the communication. */
  ResultType CloseCommunication( void );

  /** Write method sends the string via the communication link. */
  ResultType Write( const char *message, unsigned int numberOfBytes );

  /** Read method receives the string via the communication link. The
   *  data will always be null-terminated, so ensure that 'data' is at
   *  least numberOfBytes+1 in size. */
  ResultType Read( char *data, unsigned int numberOfBytes,
                   unsigned int &bytesRead );

  /** Sleep for the specified number of milliseconds. This is useful
   *  after a reset of a device on the other end of the serial port,
   *  if the device is known to take a certain amount of time to
   *  initialize. */
  void Sleep( unsigned int milliseconds );

  /** Declarations related to the State Machine. */
  igstkStateMachineMacro();

  /** Declarations related to the Logger. */
  igstkLoggerMacro();

protected:

  SocketCommunication();

  ~SocketCommunication();

  // These methods are the interface to the derived classes.

  /** Opens serial port for communication; */
  virtual ResultType InternalOpenPort( void ) { return SUCCESS; }

  /** Set communication parameters on the open port. */
  virtual ResultType InternalUpdateParameters( void ) { return SUCCESS; }

  /** Closes serial port. */
  virtual ResultType InternalClosePort( void ) { return SUCCESS; }

  /** write the data to the serial port. */
  virtual ResultType InternalWrite( const char *, unsigned int ) {
    return TIMEOUT; }

  /** read the data from the serial port. */
  virtual ResultType InternalRead( char *, unsigned int, unsigned int &) {
    return TIMEOUT; }

  /** Sleep for the period of time specified, in milliseconds. */
  virtual void InternalSleep( unsigned int ) {};

  /** Print object information. */
  virtual void PrintSelf( std::ostream& os, itk::Indent indent ) const; 

private:

  // Communication Parameters
  /**  Serial Number */
  std::string m_SerialNumber;
  
  /** Port Number */
  std::string m_PortNumber;

  /** Time to sleep for, in milliseconds. */
  unsigned int m_SleepPeriod;

  /** Input data */
  char *m_InputData;

  /** Output data */
  const char *m_OutputData;

  /** The number of bytes to write. */
  unsigned int m_BytesToWrite;

  /** Number of bytes to try to read. */
  unsigned int m_BytesToRead;

  /** Actual number of bytes read. */
  unsigned int m_BytesRead;

  /** Type used to map between integer values and inputs */
  typedef std::map<int, InputType>     IntegerInputMapType;
  
  /** Record file name */
  std::string              m_CaptureFileName;

  /** File output stream for recording stream into a file */
  std::ofstream            m_CaptureFileStream;

  /** Current message number */
  unsigned int             m_CaptureMessageNumber;

  /** Recording flag */
  bool                     m_Capture;
  
  /** Logger for recording */
  igstk::Object::LoggerType::Pointer     m_Recorder;
  
  /** LogOutput for File output stream */
  itk::StdStreamLogOutput::Pointer  m_CaptureFileOutput;
  
  /** For storing return values */
  ResultType                m_ReturnValue;
  
  /** For mapping return values to state machine inputs */
  IntegerInputMapType       m_ResultInputMap;
  
  // List of States 
  igstkDeclareStateMacro( Idle );
  igstkDeclareStateMacro( AttemptingToOpenPort );
  igstkDeclareStateMacro( PortOpen );
  igstkDeclareStateMacro( AttemptingToUpdateParameters );
  igstkDeclareStateMacro( ReadyForCommunication );
  igstkDeclareStateMacro( AttemptingToClosePort );
  igstkDeclareStateMacro( AttemptingToRead );
  igstkDeclareStateMacro( AttemptingToWrite );
  igstkDeclareStateMacro( Sleep );

  // List of Inputs
  igstkDeclareInputMacro( Success );
  igstkDeclareInputMacro( Failure );
  igstkDeclareInputMacro( Timeout );
  igstkDeclareInputMacro( OpenPort );
  igstkDeclareInputMacro( ClosePort );
  igstkDeclareInputMacro( UpdateParameters );
  igstkDeclareInputMacro( Read );
  igstkDeclareInputMacro( Write );
  igstkDeclareInputMacro( Sleep );

  /** called by state machine serial port is successfully opened */
  void OpenPortSuccessProcessing( void );

  /** called by state machine when serial port fails to open */
  void OpenPortFailureProcessing( void );

  /** called by state machine serial port is successfully closed */
  void ClosePortSuccessProcessing( void );

  /** called by state machine when serial port fails to close */
  void ClosePortFailureProcessing( void );
  
  /** called by state machine when writing succeeded */
  void SuccessProcessing( void );
  
  /** called by state machine when writing failed */
  void FailureProcessing( void );
  
  /** called by state machine when writing was timed out */
  void TimeoutProcessing( void );
  
  /** Null operation for a state machine transition */
  void NoProcessing();

  /** Called by the state machine when communication is to be opened */
  void AttemptToOpenPortProcessing( void );

  /** Called by the state machine when transfer parameters are to be set */
  void AttemptToUpdateParametersProcessing( void );

  /** Called by the state machine when communication is to be closed */
  void AttemptToClosePortProcessing( void );
    
  /** Called by the state machine when writing is to be done */
  void AttemptToWriteProcessing( void );
  
  /** Called by the state machine when reading is to be done */
  void AttemptToReadProcessing( void );

  /** Called by the state machine to purge the buffers */
  void SleepProcessing( void );

  /** Helper function to map a return value to an input */
  const InputType &MapResultToInput( int condition );

public:
    virtual void RegisterInternalRead(void(*cbFunc)(std::string)) = 0;
};

} // end namespace igstk

#endif // __igstkSocketCommunication_h
