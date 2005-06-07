/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkView2D.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISIS Georgetown University. All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _igstkView2D_h
#define _igstkView2D_h

#include "igstkView.h"

namespace igstk {


  /** \class View2D
   *
   * \brief View2D provies the functionality of rendering a scene in a 2D
   * window. This class derives from the View class, and represents the
   * abstraction of a window in a GUI in which 3D objects will be displayed but
   * from a point of view in which the camera never changes the orientation of
   * the field of view. Zooming and Panning operations are possible in this
   * window.
   *
   */
class View2D : public View 
{
public:
  typedef View2D    Self;
  typedef View      Superclass;

  igstkTypeMacro( View2D, View );
   
  /** Constructor. The parameters of this constructor are related to the FLTK
   * box class. They include the screen coordinates of the upper left
   * coordinate, its width and height, and a string associated to the label */
  View2D( int x, int y, int w, int h, const char *l="");

  /** Destructor */
  ~View2D( void );

  /** Print the object information in a stream. */
  void PrintSelf( std::ostream& os, ::itk::Indent indent ); 

protected:

  /** This method implements the user interactions with the view. It is an
   * overload of a virtual medthod defined for FLTK classes. */
  int  handle( int event );

};

} // end namespace igstk

#endif

