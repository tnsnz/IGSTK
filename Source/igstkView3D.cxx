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
#include "vtkCellPicker.h"
#include "qdebug.h"

namespace igstk {

/** Constructor */
View3D::View3D( ) : m_StateMachine(this)
{
  vtkInteractorStyleTrackballCamera * interactorStyle = 
                                     vtkInteractorStyleTrackballCamera::New();
  this->SetInteractorStyle( interactorStyle );
  interactorStyle->Delete();
}

/** Destructor */
View3D::~View3D()
{
  this->SetInteractorStyle( nullptr );
}

/** Print object information */
void View3D::PrintSelf( std::ostream& os, ::itk::Indent indent ) const
{
  this->Superclass::PrintSelf(os,indent);
}

CoordinateSystem::Pointer View3D::GetPickerCoordSystemPublic() const
{
	return this->GetPickerCoordSystem();
}

const CoordinateSystem* View3D::GetCoordinateSystemPublic() const
{
	return this->GetCoordinateSystem();
}

void View3D::setPointRegMode(bool f)
{
	pointRegMode = f;
}

void View3D::SetPickedPointCoordinates(double x, double y)
{
	InitPickPointEvent initPickPointEvt;
	this->InvokeEvent(initPickPointEvt);

	igstkLogMacro(DEBUG, "igstkView::SetPickedPointCoordinates() called ...\n");

	auto cellPicker = vtkSmartPointer<vtkCellPicker>::New();
	cellPicker->SetTolerance(0.001);

	auto renderer = GetRenderer();

	if (cellPicker->Pick(x, y, 0, renderer))
	{
		double pickedPosition[3];
		cellPicker->GetPickPosition(pickedPosition);

		Transform::VectorType pickedPoint;
		pickedPoint[0] = pickedPosition[0];
		pickedPoint[1] = pickedPosition[1];
		pickedPoint[2] = pickedPosition[2];

		double validityTime = itk::NumericTraits<double>::max();
		double errorValue = cellPicker->GetPickPosition()[2];

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
	else
	{
		igstkLogMacro(DEBUG, "Pick failed - no object found at the specified coordinates.\n");
	}
}


} // end namespace igstk
