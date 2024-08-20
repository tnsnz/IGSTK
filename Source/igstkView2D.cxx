/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkView2D.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "igstkView2D.h"
#include "vtkRenderer.h"
#include "vtkWorldPointPicker.h"
#include "vtkInteractorStyleImage.h"
#include "vtkCellPicker.h"

namespace igstk {

	/** Constructor */
	View2D::View2D() : m_StateMachine(this)
	{
		vtkInteractorStyleImage* interactorStyle = vtkInteractorStyleImage::New();
		this->SetInteractorStyle(interactorStyle);
		interactorStyle->Delete();

		// initialize the orientation to be axial
		this->m_Orientation = Axial;
	}

	/** Destructor */
	View2D::~View2D()
	{
		this->SetInteractorStyle(nullptr);
	}

	/** Print object information */
	void View2D::PrintSelf(std::ostream& os, ::itk::Indent indent) const
	{
		this->Superclass::PrintSelf(os, indent);
		os << indent << "Orientation type " << m_Orientation << std::endl;
	}

	void View2D::SetPickedPoint(igstk::Transform::VectorType pickedPoint)
	{
		prevPickedPoint = pickedPoint;
	}

	void View2D::zoomAutomatically()
	{
		auto camera = GetCamera();
		auto renderer = GetRenderer();

		RequestStop();

		double maxVal = 1.0;
		auto* bounds = renderer->ComputeVisiblePropBounds();
		auto parallelScale = camera->GetParallelScale();

		maxVal = std::max(abs(bounds[0]), abs(bounds[1]));
		maxVal = std::max(maxVal, std::max(abs(bounds[2]), abs(bounds[3])));
		maxVal = std::max(maxVal, std::max(abs(bounds[4]), abs(bounds[5])));

		camera->Zoom(parallelScale / maxVal);

		RequestStart();
	}

	void View2D::SetPickedPointCoordinates(double x, double y)
	{
		InitPickPointEvent initPickPointEvt;
		this->InvokeEvent(initPickPointEvt);

		igstkLogMacro(DEBUG, "igstkView::SetPickedPointCoordinates() called ...\n");

		auto cellPicker = vtkSmartPointer<vtkCellPicker>::New();

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

	void View2D::syncPick(Transform::VectorType& pickedPoint)
	{
		if (Axial == m_Orientation && prevPickedPoint[2] != pickedPoint[2])
		{
			pickedPoint[2] = prevPickedPoint[2];
		}
		else if (Sagittal == m_Orientation && prevPickedPoint[0] != pickedPoint[0])
		{
			pickedPoint[0] = prevPickedPoint[0];
		}
		else if (Coronal == m_Orientation && prevPickedPoint[1] != pickedPoint[1])
		{
			pickedPoint[1] = prevPickedPoint[1];
		}

		//finishSyncPick(pickedPoint);
	}

	void View2D::finishSyncPick(Transform::VectorType pickedPoint)
	{
		prevPickedPoint = pickedPoint;
	}

	/** Select the orientation of the View */
	void View2D::RequestSetOrientation(const OrientationType& orientation)
	{
		igstkLogMacro(DEBUG, "igstk::View2D::RequestSetOrientation called ....\n");

		m_Orientation = orientation;

		double focalPoint[3];
		double position[3];

		for (unsigned int cc = 0; cc < 3; cc++)
		{
			focalPoint[cc] = 0.0;
			position[cc] = focalPoint[cc];
		}

		const double distanceToFocalPoint = 1000;

		switch (m_Orientation)
		{
		case Sagittal:
		{
			position[0] += distanceToFocalPoint;
			this->SetCameraViewUp(0, 0, 1);
			break;
		}
		case Coronal:
		{
			position[1] -= distanceToFocalPoint;
			this->SetCameraViewUp(0, 0, 1);
			break;
		}
		case Axial:
		{
			position[2] -= distanceToFocalPoint;
			this->SetCameraViewUp(0, -1, 0);
			break;
		}
		}

		this->SetCameraPosition(position[0], position[1], position[2]);
		this->SetCameraFocalPoint(focalPoint[0], focalPoint[1], focalPoint[2]);
		this->SetCameraClippingRange(0.1, 100000);
		this->SetCameraParallelProjection(true);
	}


} // end namespace igstk
