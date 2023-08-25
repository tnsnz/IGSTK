/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkSocketCommunication.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
// Warning about: identifier was truncated to '255' characters in
// the debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
// Turn off deprecation warnings in MSVC 8+
#if (_MSC_VER >= 1400)
#pragma warning( disable : 4996 ) 
#endif
#endif

#include <iostream>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <time.h>

#include "igstkBinaryData.h"
#include "igstkSocketCommunication.h"
#if defined(WIN32) || defined(_WIN32)
#include "igstkSocketCommunicationForWindows.h"
#else
#include "igstkSocketCommunicationForPosix.h"
#endif


namespace igstk
{ 

SocketCommunication::Pointer SocketCommunication::New(void)
{ 
  Pointer smartPtr;
  #if defined(WIN32) || defined(_WIN32)
  smartPtr = SocketCommunicationForWindows::New();
  #else
  smartPtr = SocketCommunicationForPosix::New();
  #endif
  return smartPtr;
} 


/** Constructor */
SocketCommunication::SocketCommunication() :  m_StateMachine( this )
{
  // default to 1 second timeout
  this->SetTimeoutPeriod(1000);

  m_InputData = 0;
  m_OutputData = 0;
  m_BytesToWrite = 0;
  m_BytesToRead = 0;
  m_BytesRead = 0;

  m_CaptureFileName = "";
  m_Capture = false;
  m_CaptureMessageNumber = 0;
  
  m_ResultInputMap[SUCCESS] = m_SuccessInput;
  m_ResultInputMap[FAILURE] = m_FailureInput;
  m_ResultInputMap[TIMEOUT] = m_TimeoutInput;

  // Set the state descriptors
  igstkAddStateMacro( Idle ); 
  igstkAddStateMacro( AttemptingToOpenPort ); 
  igstkAddStateMacro( PortOpen ); 
  igstkAddStateMacro( AttemptingToUpdateParameters ); 
  igstkAddStateMacro( ReadyForCommunication ); 
  igstkAddStateMacro( AttemptingToClosePort ); 
  igstkAddStateMacro( AttemptingToRead ); 
  igstkAddStateMacro( AttemptingToWrite ); 
  igstkAddStateMacro( Sleep );

  // Set the input descriptors
  igstkAddInputMacro( Success ); 
  igstkAddInputMacro( Failure ); 
  igstkAddInputMacro( Timeout ); 
  igstkAddInputMacro( OpenPort ); 
  igstkAddInputMacro( UpdateParameters ); 
  igstkAddInputMacro( ClosePort ); 
  igstkAddInputMacro( Sleep ); 
  igstkAddInputMacro( Write ); 
  igstkAddInputMacro( Read ); 

  // ----------------------------------------
  // Programming the state machine transitions


  // Idle
  igstkAddTransitionMacro( Idle,
                           OpenPort,
                           AttemptingToOpenPort,
                           AttemptToOpenPort);

  igstkAddTransitionMacro( Idle,
                           ClosePort,
                           Idle,
                           No );

  igstkAddTransitionMacro( Idle,
                           UpdateParameters,
                           Idle,
                           No );

  // AttemptingToOpenPort
  igstkAddTransitionMacro( AttemptingToOpenPort,
                           Success,
                           PortOpen,
                           OpenPortSuccess );

  igstkAddTransitionMacro( AttemptingToOpenPort,
                           Failure,
                           Idle,
                           OpenPortFailure );

  // PortOpen
  igstkAddTransitionMacro( PortOpen,
                           UpdateParameters,
                           AttemptingToUpdateParameters,
                           AttemptToUpdateParameters);

  igstkAddTransitionMacro( PortOpen,
                           ClosePort,
                           AttemptingToClosePort,
                           AttemptToClosePort );

  // AttemptingToUpdateParameters
  igstkAddTransitionMacro( AttemptingToUpdateParameters,
                           Success,
                           ReadyForCommunication,
                           No );

  igstkAddTransitionMacro( AttemptingToUpdateParameters,
                           Failure,
                           PortOpen,
                           No );

  igstkAddTransitionMacro( ReadyForCommunication,
                           Sleep,
                           Sleep,
                           Sleep );

  igstkAddTransitionMacro( ReadyForCommunication,
                           Write,
                           AttemptingToWrite,
                           AttemptToWrite);
                                
  igstkAddTransitionMacro( ReadyForCommunication,
                           Read,
                           AttemptingToRead,
                           AttemptToRead);

  igstkAddTransitionMacro( ReadyForCommunication,
                           ClosePort,
                           AttemptingToClosePort,
                           AttemptToClosePort);

  igstkAddTransitionMacro( ReadyForCommunication,
                           UpdateParameters,
                           AttemptingToUpdateParameters,
                           AttemptToUpdateParameters);

  // AttemptingToRead
  igstkAddTransitionMacro( AttemptingToRead,
                           Success,
                           ReadyForCommunication,
                           Success );
                                
  igstkAddTransitionMacro( AttemptingToRead,
                           Failure,
                           ReadyForCommunication,
                           Failure );
                                
  igstkAddTransitionMacro( AttemptingToRead,
                           Timeout,
                           ReadyForCommunication,
                           Timeout );

  // AttemptingToWrite
  igstkAddTransitionMacro( AttemptingToWrite,
                           Success,
                           ReadyForCommunication,
                           Success );
                                
  igstkAddTransitionMacro( AttemptingToWrite,
                           Failure,
                           ReadyForCommunication,
                           Failure );
                                
  igstkAddTransitionMacro( AttemptingToWrite,
                           Timeout,
                           ReadyForCommunication,
                           Timeout );

  // AttemptingToClosePort
  igstkAddTransitionMacro( AttemptingToClosePort,
                           Success,
                           Idle,
                           No );

  igstkAddTransitionMacro( AttemptingToClosePort,
                           Failure,
                           PortOpen,
                           ClosePortFailure );

  // Sleep
  igstkAddTransitionMacro( Sleep,
                           Success,
                           ReadyForCommunication,
                           No );

  // Set the initial state of the state machine
  igstkSetInitialStateMacro( Idle );

  // Finish the programming and get ready to run
  m_StateMachine.SetReadyToRun();

} 


// Destructor 
SocketCommunication::~SocketCommunication()  
{
  // Close communication, if any
  this->SetLogger(0); 
  this->CloseCommunication();
}


void SocketCommunication::SetCaptureFileName(const char* filename)
{
  m_CaptureFileName = filename;
}


const char *SocketCommunication::GetCaptureFileName() const
{
  return m_CaptureFileName.c_str();
}


SocketCommunication::ResultType
SocketCommunication::OpenCommunication( void )
{
  igstkLogMacro( DEBUG, "SocketCommunication::OpenCommunication called ...\n");

  // Attempt to open communication port
  igstkPushInputMacro( OpenPort );
  m_StateMachine.ProcessInputs();

  return m_ReturnValue;
}


SocketCommunication::ResultType
SocketCommunication::CloseCommunication( void )
{
  igstkLogMacro( DEBUG, "SocketCommunication::CloseCommunication"
                 " called ...\n" );

  igstkPushInputMacro( ClosePort );
  m_StateMachine.ProcessInputs();

  m_CaptureFileStream.close();

  return m_ReturnValue;
}


SocketCommunication::ResultType
SocketCommunication::UpdateParameters( void )
{
  igstkLogMacro( DEBUG, "UpdateParameters called ...\n" );

  igstkPushInputMacro( UpdateParameters );
  m_StateMachine.ProcessInputs();

  return m_ReturnValue;
}


void SocketCommunication::Sleep( unsigned int milliseconds )
{
  igstkLogMacro( DEBUG, "SocketCommunication::Sleep(" << milliseconds
                 << ") called ...\n" );

  m_SleepPeriod = milliseconds;
  igstkPushInputMacro( Sleep );
  m_StateMachine.ProcessInputs();
}


SocketCommunication::ResultType 
SocketCommunication::Write( const char *data, unsigned int numberOfBytes )
{
  // In case the data contains nulls or non-graphical characters,
  // encode it before logging it
  std::string encodedString;
  igstk::BinaryData::Encode(encodedString, (unsigned char *)data,
                            numberOfBytes);
  igstkLogMacro( DEBUG, "SocketCommunication::Write(" 
                 << encodedString << ", " << numberOfBytes
                 << ") called...\n" );

  m_OutputData = data;
  m_BytesToWrite = numberOfBytes;

  // Recording for data sent
  if( m_Capture && m_CaptureFileStream.is_open() )
    {
    m_CaptureMessageNumber++;

    igstkLogMacro2( m_Recorder, INFO, m_CaptureMessageNumber
                    << ". command[" << numberOfBytes << "] "
                    << encodedString << std::endl );
    }

  igstkPushInputMacro( Write );
  m_StateMachine.ProcessInputs();

  return m_ReturnValue;
}


SocketCommunication::ResultType 
SocketCommunication::Read( char *data, unsigned int numberOfBytes,
                           unsigned int &bytesRead )
{
  m_BytesRead = 0;
  m_BytesToRead = numberOfBytes;
  m_InputData = data; 

  igstkPushInputMacro( Read );
  m_StateMachine.ProcessInputs();

  bytesRead = m_BytesRead;

  // terminate the string
  data[bytesRead] = '\0';


  // In case the data contains nulls or non-graphical characters,
  // encode it before logging it
  std::string encodedString;
  BinaryData::Encode(encodedString, (unsigned char*)data, bytesRead);

  // Recording for data received
  if( m_Capture && m_CaptureFileStream.is_open() )
    {
    igstkLogMacro2( m_Recorder, INFO, m_CaptureMessageNumber
                    << ". receive[" << bytesRead << "] "
                    << encodedString << std::endl );
    }

  igstkLogMacro( DEBUG, "SocketCommunication::Read(" << encodedString << ", "
                 << numberOfBytes << ", " << bytesRead << ") called...\n" );

  return m_ReturnValue;
}


void SocketCommunication::OpenPortSuccessProcessing( void )
{
  igstkLogMacro( DEBUG, "SocketCommunication::OpenPortSuccessProcessing"
                 " called ...\n");

  m_ReturnValue = SUCCESS;

  // Open a file for writing data stream.
  if( m_Capture )
    {
    time_t ti;
    time(&ti);

    igstkLogMacro( DEBUG, "Capture is on. Filename: "
                   << m_CaptureFileName << "\n" );

    if( m_Recorder.IsNull() )
      {
      m_Recorder = igstk::Object::LoggerType::New();
      m_Recorder->SetPriorityLevel(itk::Logger::DEBUG);
      m_Recorder->SetLevelForFlushing(itk::Logger::DEBUG);
      }
    if( m_CaptureFileOutput.IsNull() )
      {
      m_CaptureFileOutput = itk::StdStreamLogOutput::New();
      }
    m_CaptureFileStream.open(m_CaptureFileName.c_str());
    if( !m_CaptureFileStream.is_open() )
      {
      igstkLogMacro( CRITICAL,
                     "failed to open a file for writing data stream.\n" );
      }
    else
      {
      m_CaptureFileOutput->SetStream( m_CaptureFileStream );
      m_Recorder->AddLogOutput( m_CaptureFileOutput );
      igstkLogMacro2( m_Recorder, DEBUG, "# recorded " 
                      << asctime(localtime(&ti))
                          << "\n" );
      }
    }

  // if the port was opened successfully, then set transfer parameters next
  igstkPushInputMacro( UpdateParameters );
}

const SocketCommunication::InputType &
SocketCommunication::MapResultToInput( int condition )
{  
  IntegerInputMapType& conditionInputMap = m_ResultInputMap;
  const InputType *input = &m_FailureInput;
  IntegerInputMapType::const_iterator itr;

  itr = conditionInputMap.find(condition);
  if ( itr != conditionInputMap.end() )
    {
    input = & (itr->second);
    }
  else
    {
    igstkLogMacro( WARNING, " Default Input is pushed because the"
      << " condition value didn't have a corresponding input."
      << " Please make sure the condition value is correct."
      << " Condition: \n" );
    }

  return *input;
}
  

void SocketCommunication::AttemptToWriteProcessing()
{
  int condition = this->InternalWrite( m_OutputData,
                                       m_BytesToWrite );

  this->m_StateMachine.PushInput( this->MapResultToInput(condition) );
}


void SocketCommunication::AttemptToReadProcessing()
{
  int condition = this->InternalRead( m_InputData,
                                      m_BytesToRead,
                                      m_BytesRead );

  this->m_StateMachine.PushInput( this->MapResultToInput(condition) );
}


void SocketCommunication::NoProcessing()
{
}


void SocketCommunication::AttemptToOpenPortProcessing()
{
  m_StateMachine.PushInputBoolean( (bool)this->InternalOpenPort(), 
                                   m_SuccessInput,
                                   m_FailureInput );
}


void SocketCommunication::AttemptToUpdateParametersProcessing()
{
  m_StateMachine.PushInputBoolean( (bool)this->InternalUpdateParameters(),
                                   m_SuccessInput,
                                   m_FailureInput );
}


void SocketCommunication::AttemptToClosePortProcessing()
{
  m_StateMachine.PushInputBoolean( (bool)this->InternalClosePort(),
                                   m_SuccessInput,
                                   m_FailureInput );
}


void SocketCommunication::SleepProcessing()
{
  this->InternalSleep(m_SleepPeriod);

  igstkPushInputMacro( Success );
}


void SocketCommunication::SuccessProcessing()
{
  m_ReturnValue = SUCCESS;
  this->InvokeEvent( CompletedEvent() );
}


void SocketCommunication::FailureProcessing()
{
  m_ReturnValue = FAILURE;
  this->InvokeEvent( InputOutputErrorEvent() );
}


void SocketCommunication::TimeoutProcessing()
{
  m_ReturnValue = TIMEOUT;
  this->InvokeEvent( InputOutputTimeoutEvent() );
}


void SocketCommunication::OpenPortFailureProcessing()
{
  m_ReturnValue = FAILURE;
  this->InvokeEvent( OpenPortErrorEvent() );
}


void SocketCommunication::ClosePortFailureProcessing()
{
  m_ReturnValue = FAILURE;
  this->InvokeEvent( ClosePortErrorEvent() );
}


void SocketCommunication::PrintSelf( std::ostream& os,
                                     itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "SerialNumber: " << m_SerialNumber << std::endl;
  os << indent << "PortNumber: " << m_PortNumber << std::endl;

  os << indent << "Capture: " << m_Capture << std::endl;
  os << indent << "CaptureFileName: " << m_CaptureFileName << std::endl;
  os << indent << "CaptureMessageNumber: " << m_CaptureMessageNumber
     << std::endl;
}

} // end namespace igstk
