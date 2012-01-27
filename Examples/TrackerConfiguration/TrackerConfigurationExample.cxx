/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    TrackerConfigurationExample.cxx
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
//  Warning about: identifier was truncated to '255' characters 
//  in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4284 )
#endif

// BeginLatex
// 
// This example illustrates how to use xml readers and the TrackerController 
// class to initialize a specific tracker configuration described in an xml 
// configuration file. To use the tracker controller we need to include the
// following files:
//
// EndLatex
// BeginCodeSnippet
#include "igstkTrackerConfiguration.h"
#include "igstkTrackerController.h"
// EndCodeSnippet

// BeginLatex
// 
// To enable reading of all supported trackers we need to include the 
// corresponding xml file readers:
//
// EndLatex
// BeginCodeSnippet
#include "igstkTrackerConfigurationFileReader.h"
#include "igstkPolarisVicraConfigurationXMLFileReader.h"
#include "igstkPolarisSpectraConfigurationXMLFileReader.h"
#include "igstkPolarisHybridConfigurationXMLFileReader.h"
#include "igstkAuroraConfigurationXMLFileReader.h"
#include "igstkCertusConfigurationXMLFileReader.h"
#include "igstkMicronConfigurationXMLFileReader.h"
#include "igstkAscensionConfigurationXMLFileReader.h"
#include "igstkAscension3DGConfigurationXMLFileReader.h"
// EndCodeSnippet

/**
 * Observer for the event generated by 
 * TrackerConfigurationFileReader->RequestGetData() method.
 */
igstkObserverObjectMacro( 
  TrackerConfiguration, 
  igstk::TrackerConfigurationFileReader::TrackerConfigurationDataEvent,
  igstk::TrackerConfiguration )

/**
 * Observer for the TrackerController->RequestInitialize() failure.
 */
igstkObserverMacro( InitializeError, 
                    igstk::TrackerController::InitializeErrorEvent, 
                    std::string )
                   
/**
 * This program receives a tracker's xml configuration file, and attempts to
 * read it. If it is one of the supported trackers and the xml file is valid
 * a tracker controller is initialized (tracker is implicitly initialized too).
 */
int main( int argc, char *argv[] )
{
  if( argc != 2 )
    {
      std::cerr<<"Wrong number of input arguments.\n";
      std::cerr<<"Usage: "<<argv[0]<<" tracker_configuration_file_name\n";
      return EXIT_FAILURE;
    }

// BeginLatex
// 
// Instantiate all xml readers:
//
// EndLatex
// BeginCodeSnippet
  const unsigned int NUM_TRACKER_TYPES = 8;
  igstk::TrackerConfigurationXMLFileReaderBase::Pointer 
    trackerConfigurationXMLReaders[NUM_TRACKER_TYPES];
  trackerConfigurationXMLReaders[0] = 
    igstk::PolarisVicraConfigurationXMLFileReader::New();
  trackerConfigurationXMLReaders[1] = 
    igstk::PolarisSpectraConfigurationXMLFileReader::New();
  trackerConfigurationXMLReaders[2] = 
    igstk::PolarisHybridConfigurationXMLFileReader::New();
  trackerConfigurationXMLReaders[3] = 
    igstk::AuroraConfigurationXMLFileReader::New();
  trackerConfigurationXMLReaders[4] = 
    igstk::MicronConfigurationXMLFileReader::New();
  trackerConfigurationXMLReaders[5] =
       igstk::AscensionConfigurationXMLFileReader::New();
  trackerConfigurationXMLReaders[6] = 
    igstk::Ascension3DGConfigurationXMLFileReader::New();
  trackerConfigurationXMLReaders[7] = 
    igstk::CertusConfigurationXMLFileReader::New();



// EndCodeSnippet

// BeginLatex
// 
// We then instantiate the configuration file reader which is parameterized
// using the xml readers.
//
// EndLatex
// BeginCodeSnippet
  igstk::TrackerConfigurationFileReader::Pointer trackerConfigReader = 
    igstk::TrackerConfigurationFileReader::New();
// EndCodeSnippet

                //need to observe if the request read succeeds or fails
                //there is a third option that the read is invalid, if the
                //file name or xml reader weren't set
  igstk::TrackerConfigurationFileReader::ReadFailSuccessObserver::Pointer rfso = 
    igstk::TrackerConfigurationFileReader::ReadFailSuccessObserver::New();
  trackerConfigReader->AddObserver( 
    igstk::TrackerConfigurationFileReader::ReadSuccessEvent(),
    rfso );
  trackerConfigReader->AddObserver( 
    igstk::TrackerConfigurationFileReader::ReadFailureEvent(),
    rfso );
  trackerConfigReader->AddObserver( 
    igstk::TrackerConfigurationFileReader::UnexpectedTrackerTypeEvent(),
    rfso );

             //setting the file name and reader always succeeds so I don't
             //observe for success event

// BeginLatex
// 
// Set the file name we want to read.
//
// EndLatex
// BeginCodeSnippet
  trackerConfigReader->RequestSetFileName( argv[1] );
//EndCodeSnippet

  TrackerConfigurationObserver::Pointer tco = 
    TrackerConfigurationObserver::New();
  const igstk::TrackerConfiguration *trackerConfiguration = NULL;


           //attempt reading with all readers till one succeeds
// BeginLatex
// 
// Try to read the given file by using each of the xml readers till one of them
// succeeds.
//
// EndLatex
// BeginCodeSnippet
  for( unsigned int i=0; 
       trackerConfiguration == NULL && i<NUM_TRACKER_TYPES; 
       i++ )
    {
    //setting the xml reader always succeeds so I don't
    //observe the success event
    trackerConfigReader->RequestSetReader( trackerConfigurationXMLReaders[i] );  
 
    //try to read using the current xml reader
    trackerConfigReader->RequestRead();

    //xml file doesn't match current reader
    if( rfso->GotUnexpectedTrackerType() )
    {
      rfso->Reset();
    } //xml file matches current reader, but there 
      //is a problem with the xml data
    else if( rfso->GotFailure() && !rfso->GotUnexpectedTrackerType() )
    {
      std::cerr<<"Failed reading configuration file (";
      std::cerr<<rfso->GetFailureMessage()<<").\n";
      return EXIT_FAILURE;
    } //we read the xml data and everything is fine
    else if( rfso->GotSuccess() )
      {
      //get the configuration data from the reader
      trackerConfigReader->AddObserver( 
        igstk::TrackerConfigurationFileReader::TrackerConfigurationDataEvent(),
        tco );
      trackerConfigReader->RequestGetData();

      if( tco->GotTrackerConfiguration() )
        {
        trackerConfiguration = tco->GetTrackerConfiguration();        
        }
      }
    }
//EndCodeSnippet

  if( trackerConfiguration == NULL )   
    {
    std::cerr<<"Unsupported tracker type.\n";
    return EXIT_FAILURE;
    }
  
  
  //initialize the tracker controller

// BeginLatex
// 
// Instantiate TrackerController.
//
// EndLatex
// BeginCodeSnippet
  igstk::TrackerController::Pointer trackerController = 
    igstk::TrackerController::New();
// EndCodeSnippet  
  InitializeErrorObserver::Pointer ieo = 
    InitializeErrorObserver::New();
  trackerController->AddObserver( 
     igstk::TrackerController::InitializeErrorEvent(), ieo );

// BeginLatex
// 
// Initialize TrackerController, initializes the specific tracker.
//
// EndLatex
// BeginCodeSnippet
  trackerController->RequestInitialize( trackerConfiguration );
//EndCodeSnippet

  if( ieo->GotInitializeError() )
    {
    std::cout<<ieo->GetInitializeError()<<"\n";
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

