/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    HelloWorld.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISIS Georgetown University. All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
//Warning about: identifier was truncated to '255' characters 
//in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4284 )
#endif



// BeginLatex
//
// This example illustrates the minimal applications that can be written using IGSTK.
// The application uses three main components. They are the View, the
// SpatialObjects and the Tracker. The View is the visualization window that is
// presented to the user in the graphical user interface (GUI) of the
// application. The SpatialObjects are used for representing geometrical shapes
// in the scene of the surgical room. In this simplified example, we only use
// them to represent a cylinder and a sphere. The Tracker is the device that
// provides position and orientation information about some of the objects in
// the scene. A Tracker can track multiple objects, and each one of them is
// referred as a TrackerTool. In this minimal example we use a MouseTracker
// that is a class intended mainly for demonstration and debugging purposes.
// This tracker get the values of positions from the position of the mouse on
// the screen. The position values are then passed to the sphere object in the
// scene. The MouseTracker is not intended to be used in a real image guided
// surgery application.
// 
// EndLatex

// BeginLatex
//
// Given that this application has a graphical user interface, we use FLTK in
// order to define a very simple GUI. FLTK stores the description of the
// interface in files with extension .fl. The FLTK tool \emph{fluid} takes this
// file and uses it for generating C++ code in two files. One header file with
// extension .h, and an implementation file with extension .cxx. In order to
// use that GUI from the main program of our application we must include the
// header file generated by fluid. This is done in the following line.
// 
// EndLatex

// BeginCodeSnippet
#include "HelloWorldGUI.h"
// EndCodeSnippet




// BeginLatex
//
// The geometrical description of the Cylinder and the Sphere in the scene are
// managed by SpatialObjects. For this purpose we need the two classes
// \doxygen{EllipsoidObject} and \doxygen{CylinderObject}. Their two header
// files are included below.
// 
// EndLatex

// BeginCodeSnippet
#include "igstkEllipsoidObject.h"
#include "igstkCylinderObject.h"
// EndCodeSnippet



// BeginLatex
//
// The visual representation of SpatialObjects in the visualization window is
// done with Representation objects. Every SpatialObject has one or several
// representation objects associated with it. We include now the header files
// of the \doxygen{EllipsoidObjectRepresentation} and
// \doxygen{CylinderObjectRepresentation}.
// 
// EndLatex

// BeginCodeSnippet
#include "igstkEllipsoidObjectRepresentation.h"
#include "igstkCylinderObjectRepresentation.h"
// EndCodeSnippet



// BeginLatex
//
// As we mention earlier, the tracker in this minimal application is
// represented by a \doxygen{MouseTracker}. This class provides the same
// interface of a real tracking device but with the convenience of running
// based on the movement of the mouse in the screen. The header file of this
// class is included below.
// 
// EndLatex

// BeginCodeSnippet
#include "igstkMouseTracker.h"
// EndCodeSnippet




// BeginLatex
//
// Since image guided surgery applications are used in a critical environment,
// it is quite important to be able to trace the behavior of the application
// during the intervention. For this purpose IGSTK uses a \doxygen{Logger}
// class and some helpers. The logger is a class that receives messages from
// IGSTK classes and forward those messages to LoggerOutput classes. Typical
// logger output classes are the standard output, a file and a popup window.
// The Logger classes and their helpers are taken from the Insight Toolkit (ITK).
// 
// EndLatex

// BeginCodeSnippet
#include "itkLogger.h"
#include "itkStdStreamLogOutput.h"
// EndCodeSnippet





// BeginLatex
//
// We are now ready for writing the code of the actual application. Of couse we
// start with the classical \code{main()} function.
// 
// EndLatex

// BeginCodeSnippet
int main(int , char** )
{ 
// EndCodeSnippet


// BeginLatex
// First, we instantiate the GUI application.
// EndLatex
// 
// BeginCodeSnippet
   HelloWorldGUI * m_GUI = new HelloWorldGUI();
// EndCodeSnippet
// 
  m_GUI->MainWindow->show();

// BeginLatex
// Next, we instantiate the ellipsoidal spatial object that we will be 
// attaching to the tracker.
// EndLatex

// Create the ellipsoid 
// BeginCodeSnippet
  igstk::EllipsoidObject::Pointer ellipsoid = igstk::EllipsoidObject::New();
// EndCodeSnippet
// 
// BeginLatex
// The ellipsoid radius can be set to one in all dimensions ( X,Y and Z ) 
// using the SetRadius member function as follows.
// EndLatex
// BeginCodeSnippet
  ellipsoid->SetRadius(1,1,1);
// EndCodeSnippet

// BeginLatex
// To visualize the ellipsoid spatial object, an object representation class is created and the 
// ellipsoid spatial object is added to it. 
//
// EndLatex
 
 // BeginCodeSnippet 
  igstk::EllipsoidObjectRepresentation::Pointer ellipsoidRepresentation = igstk::EllipsoidObjectRepresentation::New();
  ellipsoidRepresentation->RequestSetEllipsoidObject( ellipsoid );
  ellipsoidRepresentation->SetColor(0.0,1.0,0.0);
  ellipsoidRepresentation->SetOpacity(1.0);
// EndCodeSnippet
  
  // BeginLatex
  // Similarly, a cylinder spatial object and cylinder spatial object representation
  // object are instantiated as follows.
  // EndLatex

  
  // BeginCodeSnippet
  igstk::CylinderObject::Pointer cylinder = igstk::CylinderObject::New();
  cylinder->SetRadius(0.1);
  cylinder->SetHeight(3);

  igstk::CylinderObjectRepresentation::Pointer cylinderRepresentation = igstk::CylinderObjectRepresentation::New();
  cylinderRepresentation->RequestSetCylinderObject( cylinder );
  cylinderRepresentation->SetColor(1.0,0.0,0.0);
  cylinderRepresentation->SetOpacity(1.0);
  // EndCodeSnippet
// 
  // BeginLatex
  // Next, the spatial objects are added to the view
  // EndLatex
  m_GUI->Display->RequestAddObject( ellipsoidRepresentation );
  m_GUI->Display->RequestAddObject( cylinderRepresentation );
  m_GUI->Display->RequestResetCamera();
  m_GUI->Display->Update();

  // Enable interactions
  m_GUI->Display->RequestEnableInteractions();

  // Create a tracker
  igstk::MouseTracker::Pointer tracker = igstk::MouseTracker::New();

  // Initialize the tracker
  tracker->Open();
  tracker->Initialize();
  tracker->SetScaleFactor( 100.0 );

  const unsigned int toolPort = 0;
  const unsigned int toolNumber = 0;
  tracker->AttachObjectToTrackerTool( toolPort, toolNumber, ellipsoid );

  m_GUI->SetTracker( tracker );

  // Setup the logging system
  itk::Logger::Pointer logger = itk::Logger::New();
  itk::StdStreamLogOutput::Pointer logOutput = itk::StdStreamLogOutput::New();
  itk::StdStreamLogOutput::Pointer fileOutput = itk::StdStreamLogOutput::New();
  
  logOutput->SetStream( std::cout );
  logger->AddLogOutput( logOutput );
  logger->SetPriorityLevel( itk::Logger::DEBUG );

  std::ofstream ofs( "log.txt" );
  fileOutput->SetStream( ofs );
  logger->AddLogOutput( logOutput );
  
  m_GUI->Display->SetLogger( logger ); 
  tracker->SetLogger( logger );


  // Setup the frequency for refreshing the view
  m_GUI->Display->RequestSetRefreshRate( 60 ); // 60 Hz
  m_GUI->Display->RequestStart();


  while( !m_GUI->HasQuitted() )
    {
    Fl::wait(0.001);
    igstk::PulseGenerator::CheckTimeouts();
    }


  tracker->StopTracking();
  tracker->Close();

  delete m_GUI;

  ofs.close();

  return EXIT_SUCCESS;
  
}

