/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkView3D.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "igstkView3D.h"

#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkWorldPointPicker.h"
#include "qdebug.h"

namespace igstk {

/** Constructor */
View3D::View3D( ) : m_StateMachine(this)
{
  vtkInteractorStyleTrackballCamera * interactorStyle = 
                                     vtkInteractorStyleTrackballCamera::New();
  this->SetInteractorStyle( interactorStyle );
  interactorStyle->Delete();

  m_ptPicked[0] = -1;
  m_ptPicked[1] = -1;
  m_ptPicked[2] = -1;
}

/** Destructor */
View3D::~View3D()
{
  this->SetInteractorStyle( NULL );
}

/** Print object information */
void View3D::PrintSelf( std::ostream& os, ::itk::Indent indent ) const
{
  this->Superclass::PrintSelf(os,indent);
}

Transform::VectorType View3D::GetPickedPoint()
{
	return m_ptPicked;
}

void View3D::SetPickedPointCoordinates(double x, double y, QMouseEvent* e)
{
	if (e->button() != Qt::RightButton)
		return;

	igstkLogMacro(DEBUG, "igstkView::SetPickedPointCoordinates() called ...\n");

	auto pointPicker = GetPointPicker();
	auto renderer = GetRenderer();

	pointPicker->Pick(x, y, 0, renderer);

	double data[3];
	pointPicker->GetPickPosition(data);

	Transform::VectorType pickedPoint;
	m_ptPicked[0] = data[0];
	m_ptPicked[1] = data[1];
	m_ptPicked[2] = data[2];

	//qDebug() << "m_ptPicked[0] : " << m_ptPicked[0]
	//	<< "m_ptPicked[1] : " << m_ptPicked[1]
	//	<< "m_ptPicked[2] : " << m_ptPicked[2];

	double validityTime = itk::NumericTraits<double>::max();
	double errorValue = 1.0; // this should be obtained from 
							 // the picked object.

	Transform transform;
	transform.SetTranslation(pickedPoint, errorValue, validityTime);

	CoordinateSystemTransformToResult transformCarrier;

	transformCarrier.Initialize(transform,
		this->GetPickerCoordSystem(),
		this->GetCoordinateSystem());

	CoordinateSystemTransformToEvent  transformEvent;
	transformEvent.Set(transformCarrier);

	this->InvokeEvent(transformEvent);
}


} // end namespace igstk
