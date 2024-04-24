/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkNDITracker.cxx
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
// Warning about: identifier was truncated to '255' characters in the
// debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include "igstkNDITracker.h"

#include <iostream>
#include <fstream>

namespace igstk
{

/** Constructor: Initializes all internal variables. */
NDITracker::NDITracker(void):m_StateMachine(this)
{
  m_CommandInterpreter = CommandInterpreterType::New();
  m_SerialComm = 0;
  m_SocketComm = 0;

  this->SetThreadingEnabled( true );

  m_BaudRate = SerialCommunication::BaudRate115200; 
  //m_BufferLock = itk::MutexLock::New();
}

/** Destructor */
NDITracker::~NDITracker(void)
{
}


/** Helper function for reporting interpreter errors. */
NDITracker::ResultType
NDITracker::CheckError(CommandInterpreterType *interpreter) const
{
  const int errnum = interpreter->GetError();

  if (errnum)
    {
    // convert errnum to a hexidecimal string
    std::ostringstream os;
    os << "0x";
    os.width(2);
    os.fill('0');
    os << std::hex << std::uppercase << errnum;
    igstkLogMacro( WARNING, "NDI Error " << os.str() << ": " <<
                   interpreter->ErrorString(errnum) << "\n");

    return FAILURE;
    }

  return SUCCESS;
}

/** Get method for the command interpreter 
*  This will method will be used by the derived classes */
NDITracker::CommandInterpreterType::Pointer 
NDITracker::GetCommandInterpreter() const
{
  igstkLogMacro( DEBUG, 
    "igstk::NDITracker:: GetCommandInterpreter ...\n");

  return m_CommandInterpreter;  
}

/** Set the communication object, it will be initialized as necessary
  * for use with the NDI */
void NDITracker::SetCommunication( SerialCommunication *communication )
{
  igstkLogMacro( DEBUG, 
    "igstk::NDITracker:: Entered SetCommunication ...\n");
  m_SerialComm = communication;
  m_BaudRate = communication->GetBaudRate();
  m_CommandInterpreter->SetCommunication( communication );

  // data records are of variable length and end with a carriage return
  if( communication )
    {
    communication->SetUseReadTerminationCharacter( true );
    communication->SetReadTerminationCharacter( '\r' );
    }

  igstkLogMacro( DEBUG, 
    "igstk::NDITracker:: Exiting SetCommunication ...\n"); 
}

void NDITracker::SetCommunication( SocketCommunication *communication )
{
	igstkLogMacro(DEBUG,
		"igstk::NDITracker:: Entered SetCommunication ...\n");
	m_SocketComm = communication;
	m_CommandInterpreter->SetCommunication(communication);

	// data records are of variable length and end with a carriage return
	if (communication)
	{
		communication->SetUseReadTerminationCharacter(true);
		communication->SetReadTerminationCharacter('\r');
	}

	igstkLogMacro(DEBUG,
		"igstk::NDITracker:: Exiting SetCommunication ...\n");
}

/** Open communication with the tracking device. */
NDITracker::ResultType NDITracker::InternalOpen( void )
{
  igstkLogMacro( DEBUG, "igstk::NDITracker::InternalOpen called ...\n");

  ResultType result = SUCCESS;

  if (m_SerialComm.IsNull() && m_SocketComm.IsNull())
    {
    igstkLogMacro( CRITICAL, "NDITracker: AttemptToOpen before "
                   "Communication is set.\n");
    result = FAILURE;
    }

  if (result == SUCCESS)
    {
    m_CommandInterpreter->RESET();

    result = this->CheckError(m_CommandInterpreter);
    }

  if (result == SUCCESS)
    {
    // log information about the device
    m_CommandInterpreter->VER(CommandInterpreterType::NDI_CONTROL_FIRMWARE);
    result = this->CheckError(m_CommandInterpreter);
    }

  if (result == SUCCESS)
    {
	  if (m_SerialComm)
	  {
		  // increase the baud rate to the initial baud rate that was set for
		  // serial communication, since the baud rate is set to 9600 by 
		  // NDICommandInterpreter::SetCommunication() in order to communicate
		  // with the just-turned-on device which has a default baud rate of 9600
		  CommandInterpreterType::COMMBaudType baudRateForCOMM =
			  CommandInterpreterType::NDI_115200;

		  switch (m_BaudRate)
		  {
		  case SerialCommunication::BaudRate9600:
			  baudRateForCOMM = CommandInterpreterType::NDI_9600;
			  break;
		  case SerialCommunication::BaudRate19200:
			  baudRateForCOMM = CommandInterpreterType::NDI_19200;
			  break;
		  case SerialCommunication::BaudRate38400:
			  baudRateForCOMM = CommandInterpreterType::NDI_38400;
			  break;
		  case SerialCommunication::BaudRate57600:
			  baudRateForCOMM = CommandInterpreterType::NDI_57600;
			  break;
		  case SerialCommunication::BaudRate115200:
			  baudRateForCOMM = CommandInterpreterType::NDI_115200;
			  break;
		  }

		  m_CommandInterpreter->COMM(baudRateForCOMM,
			  CommandInterpreterType::NDI_8N1,
			  CommandInterpreterType::NDI_NOHANDSHAKE);
	  }
	  Sleep(1000);
    m_CommandInterpreter->INIT();
    result = this->CheckError(m_CommandInterpreter);
    }

  return result;
}

/** Close communication with the tracking device. */
NDITracker::ResultType NDITracker::InternalClose( void )
{
  igstkLogMacro( DEBUG, "igstk::NDITracker::InternalClose called ...\n");

  if (m_SerialComm)
  {
	  // return the device back to its initial comm setttings
	  m_CommandInterpreter->COMM(CommandInterpreterType::NDI_9600,
		  CommandInterpreterType::NDI_8N1,
		  CommandInterpreterType::NDI_NOHANDSHAKE);
  }
  return this->CheckError(m_CommandInterpreter);
}

/** Put the tracking device into tracking mode. */
NDITracker::ResultType NDITracker::InternalStartTracking( void )
{
  igstkLogMacro( DEBUG, 
    "igstk::NDITracker::InternalStartTracking called ...\n");  

  m_CommandInterpreter->TSTART();

  return this->CheckError(m_CommandInterpreter);
}

/** Take the tracking device out of tracking mode. */
NDITracker::ResultType NDITracker::InternalStopTracking( void )
{
  igstkLogMacro( DEBUG, 
    "igstk::NDITracker::InternalStopTracking called ...\n");

  m_CommandInterpreter->TSTOP();

  return this->CheckError(m_CommandInterpreter);
}

/** Reset the tracking device to put it back to its original state. */
NDITracker::ResultType NDITracker::InternalReset( void )
{
  igstkLogMacro( DEBUG, "igstk::NDITracker::InternalReset called ...\n");

  m_CommandInterpreter->RESET();
  m_CommandInterpreter->INIT();

  ResultType result = this->CheckError(m_CommandInterpreter);

  if (result == SUCCESS)
    {
    // log information about the device
    m_CommandInterpreter->VER(CommandInterpreterType::NDI_CONTROL_FIRMWARE);
    result = this->CheckError(m_CommandInterpreter);
    }

  return result;
}


/** Update the status and the transforms for all TrackerTools. */
NDITracker::ResultType NDITracker::InternalUpdateStatus()
{
  igstkLogMacro( DEBUG, 
    "igstk::NDITracker::InternalUpdateStatus called ...\n");

  // these flags are set for tools that can be used for tracking
  const unsigned long mflags = (CommandInterpreterType::NDI_TOOL_IN_PORT |
                                CommandInterpreterType::NDI_INITIALIZED |
                                CommandInterpreterType::NDI_ENABLED);

  typedef PortHandleContainerType::const_iterator  ConstIteratorType;

  ConstIteratorType inputItr = m_PortHandleContainer.begin();
  ConstIteratorType inputEnd = m_PortHandleContainer.end();

  TrackerToolsContainerType trackerToolContainer = 
    this->GetTrackerToolContainer();

  while( inputItr != inputEnd )
    {
    const int portStatus = m_ToolStatusContainer[inputItr->first];

    // only report tools that are enabled
    if ((portStatus & mflags) != mflags) 
      {
      igstkLogMacro( DEBUG, "igstk::NDITracker::InternalUpdateStatus: " <<
                     "tool " << inputItr->first << " is not available \n");
      ++inputItr;
      continue;
      }

    // only report tools that are in view
    if (m_ToolAbsentStatusContainer[inputItr->first])
      {
      // there should be a method to set that the tool is not in view
      igstkLogMacro( DEBUG, "igstk::NDITracker::InternalUpdateStatus: " <<
                     "tool " << inputItr->first << " is not in view\n");

      // report to the tracker tool that the tracker is not available 
      this->ReportTrackingToolNotAvailable( 
        trackerToolContainer[inputItr->first] );

      ++inputItr;
      continue;
      }

    const PortIdentifierType portId = inputItr->first;

    // report to the tracker tool that the tracker is Visible 
    this->ReportTrackingToolVisible( trackerToolContainer[portId] );
 
    // create the transform
    TransformType transform;

    typedef TransformType::VectorType TranslationType;
    TranslationType translation;

    translation[0] = (m_ToolTransformBuffer[portId])[4];
    translation[1] = (m_ToolTransformBuffer[portId])[5];
    translation[2] = (m_ToolTransformBuffer[portId])[6];

    typedef TransformType::VersorType RotationType;
    RotationType rotation;
    const double normsquared = 
      m_ToolTransformBuffer[portId][0]*m_ToolTransformBuffer[portId][0] +
      m_ToolTransformBuffer[portId][1]*m_ToolTransformBuffer[portId][1] +
      m_ToolTransformBuffer[portId][2]*m_ToolTransformBuffer[portId][2] +
      m_ToolTransformBuffer[portId][3]*m_ToolTransformBuffer[portId][3];

    // don't allow null quaternions
    if (normsquared < 1e-6)
      {
      rotation.Set(0.0, 0.0, 0.0, 1.0);
      igstkLogMacro( WARNING, "igstk::NDITracker::InternUpdateStatus: bad "
                     "quaternion, norm=" << sqrt(normsquared) << "\n");
      }
    else
      {
      // ITK quaternions are in xyzw order, not wxyz order
      rotation.Set(m_ToolTransformBuffer[portId][1],
                   m_ToolTransformBuffer[portId][2],
                   m_ToolTransformBuffer[portId][3],
                   m_ToolTransformBuffer[portId][0]);
      }

    // retool NDI error value
    typedef TransformType::ErrorType  ErrorType;
    ErrorType errorValue = m_ToolTransformBuffer[portId][7];

    transform.SetToIdentity(this->GetValidityTime());
    transform.SetTranslationAndRotation(translation, rotation, errorValue,
                                        this->GetValidityTime());
  
    //m_BufferLock->Lock();
    m_BufferLock.lock();
    // set the raw transform
    this->SetTrackerToolRawTransform( trackerToolContainer[portId], transform );
    this->SetTrackerToolTransformUpdate( trackerToolContainer[portId], true );
    //m_BufferLock->Unlock();
    m_BufferLock.unlock();

    ++inputItr;
    }
  
  return SUCCESS;
}

NDITracker::ResultType NDITracker::InternalThreadedCheckGPIOStatus(void)
{
	igstkLogMacro(DEBUG, "igstk::NDITracker::InternalThreadedCheckGPIOStatus"
		"called ...\n");

    m_CommandInterpreter->Command("GetInfo Param.GPIO.Configure.Value.4");

    ResultType result = this->CheckError(m_CommandInterpreter);

	return result;
}

/** Update the m_StatusBuffer and the transforms. 
    This function is called by a separate thread. */
NDITracker::ResultType NDITracker::InternalThreadedUpdateStatus( void )
{
  igstkLogMacro( DEBUG, "igstk::NDITracker::InternalThreadedUpdateStatus "
                 "called ...\n");

  // get the transforms for all tools from the NDI
  m_CommandInterpreter->TX(
	  CommandInterpreterType::NDI_XFORMS_AND_STATUS |
	  CommandInterpreterType::NDI_ADDITIONAL_INFO |
	  CommandInterpreterType::NDI_SINGLE_STRAY |
	  CommandInterpreterType::NDI_POSITION_MARKER |
	  CommandInterpreterType::NDI_INCLUDE_OUT_OF_VOLUME |
	  CommandInterpreterType::NDI_PASSIVE_STRAY);

  ResultType result = this->CheckError(m_CommandInterpreter);

  // lock the buffer
  //m_BufferLock->Lock();
  if (result == SUCCESS)
  {
      m_BufferLock.lock();

  // Initialize transformations to identity.
  // The NDI transform is 8 values:
  // the first 4 values are a quaternion
  // the next 3 values are an x,y,z position
  // the final value is an error estimate in the range [0,1]
  //
  std::vector < double > transform;
  transform.push_back ( 1.0 );
  transform.push_back ( 0.0 );
  transform.push_back ( 0.0 );
  transform.push_back ( 0.0 );
  transform.push_back ( 0.0 );
  transform.push_back ( 0.0 );
  transform.push_back ( 0.0 );
  transform.push_back ( 0.0 );

    typedef PortHandleContainerType::iterator  IteratorType;

    IteratorType inputItr = m_PortHandleContainer.begin();
    IteratorType inputEnd = m_PortHandleContainer.end();

    while( inputItr != inputEnd )
      {
      m_ToolAbsentStatusContainer[inputItr->first] = 0;
      m_ToolStatusContainer[inputItr->first] = 0;
 
      unsigned int ph = inputItr->second;
      if ( ph == 0 )
        {
        ++inputItr;
        continue;
        }

      double transformRecorded[8];

      const int tstatus = 
        m_CommandInterpreter->GetTXTransform(ph, transformRecorded);

      const int absent = (tstatus != CommandInterpreterType::NDI_VALID);
      const int status = m_CommandInterpreter->GetTXPortStatus(ph);

      if (!absent)
        {
        for( unsigned int i = 0; i < 8; i++ )
          {
          transform[i] = transformRecorded[i];
          }
        }

      m_ToolAbsentStatusContainer[inputItr->first] = absent;
      m_ToolStatusContainer[inputItr->first] = status;
      m_ToolTransformBuffer[inputItr->first] = transform;

      ++inputItr;
      }

    m_BufferLock.unlock();
    }

  // In the original vtkNDITracker code, there was a check at this
  // point in the code to see if any new tools had been plugged in

  // unlock the buffer
  //m_BufferLock->Unlock();

  igstk::PulseGenerator::Sleep(1);

  return result;
}

NDITracker::ResultType 
NDITracker::
AddTrackerToolToInternalDataContainers( const TrackerToolType * trackerTool ) 
{
  igstkLogMacro( DEBUG, 
    "igstk::NDITracker::AddTrackerToolToInternalDataContainers called ...\n");

  if ( trackerTool == nullptr )
    {
    return FAILURE;
    } 

  std::string trackerToolIdentifier = 
    trackerTool->GetTrackerToolIdentifier();

  // add it to the port handle container 
  this->m_PortHandleContainer[ trackerToolIdentifier ] = m_PortHandleToBeAdded;

  // add it to the tool absent status 
  this->m_ToolAbsentStatusContainer[ trackerToolIdentifier ] = 0;

  // add it to the tool status container
  this->m_ToolStatusContainer[ trackerToolIdentifier ] = 0;

  return SUCCESS;
}

NDITracker::ResultType 
NDITracker::
RemoveTrackerToolFromInternalDataContainers
( const TrackerToolType * trackerTool ) 
{
  igstkLogMacro( DEBUG, 
    "igstk::NDITracker::RemoveTrackerToolFromInternalDataContainers "
    "called ...\n");
 
  if ( trackerTool == nullptr )
    {
    return FAILURE;
    } 

  std::string trackerToolIdentifier = 
    trackerTool->GetTrackerToolIdentifier();

  m_CommandInterpreter->PHF( m_PortHandleContainer[trackerToolIdentifier] );
    
  // disable the port handle
  m_CommandInterpreter->PDIS( m_PortHandleContainer[ trackerToolIdentifier] );  

  // print warning if failed to disable
  this->CheckError(m_CommandInterpreter);

  // remove the tool from port handle container
  this->m_PortHandleContainer.erase( trackerToolIdentifier );

  // remove the tool from absent status container 
  this->m_ToolAbsentStatusContainer.erase( trackerToolIdentifier );

  // remove the tool from  tool status container
  this->m_ToolStatusContainer.erase( trackerToolIdentifier );

  // remove the tool from the Transform buffer container
  this->m_ToolTransformBuffer.erase( trackerToolIdentifier );

  return SUCCESS;
}

/**The "ValidateSpecifiedFrequency" method checks if the specified
  * frequency is valid for the tracking device that is being used. */
NDITracker::ResultType
NDITracker::ValidateSpecifiedFrequency( double frequencyInHz )
{
  if ( frequencyInHz < 0.0 )
    {
    return FAILURE;
    } 
  return SUCCESS;
}

/** Print Self function */
void NDITracker::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

}


} // end of namespace igstk
