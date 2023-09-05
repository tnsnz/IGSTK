/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkCylinderObject.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "igstkCylinderObject.h"
#include "igstkEvents.h"

namespace igstk
{
	/** Constructor */
	CylinderObject::CylinderObject() :m_StateMachine(this)
	{
		// We create the ellipse spatial object
		m_CylinderSpatialObject = CylinderSpatialObjectType::New();

		CylinderPointListType pointList;
		CylinderPointType p1, p2;
		p1.SetPositionInObjectSpace(0, 0, 0);
		p1.SetRadiusInObjectSpace(0);
		pointList.push_back(p1);
		p2.SetPositionInObjectSpace(0, 0, 0);
		p2.SetRadiusInObjectSpace(0);
		pointList.push_back(p2);
		m_CylinderSpatialObject->SetPoints(pointList);
		m_CylinderSpatialObject->Update();

		this->RequestSetInternalSpatialObject(m_CylinderSpatialObject);
	}

	/** Destructor */
	CylinderObject::~CylinderObject()
	{
	}


	void CylinderObject::SetRadius(double radius)
	{
		//m_CylinderSpatialObject->SetRadius(radius);
		assert(m_CylinderSpatialObject->GetPoints().size() == 2);

		for (auto &point : m_CylinderSpatialObject->GetPoints()) {
			point.SetRadiusInObjectSpace(radius);
		}
		m_CylinderSpatialObject->Update();
	}


	void CylinderObject::SetHeight(double height)
	{
		//m_CylinderSpatialObject->SetHeight(height);
		auto &points = m_CylinderSpatialObject->GetPoints();
		assert(points.size() == 2);
		
		auto position = points[1].GetPositionInObjectSpace();
		assert(position.size() == 3);

		position[2] = height;
		points[1].SetPositionInObjectSpace(position);

		m_CylinderSpatialObject->Update();
	}

	double CylinderObject::GetRadius() const
	{
		//return m_CylinderSpatialObject->GetRadius();

		assert(m_CylinderSpatialObject->GetPoints().size() == 2);
		double prevRadius = -1;
		double radius = 0;

		for (auto point : m_CylinderSpatialObject->GetPoints()) {
			assert(-1 == prevRadius || prevRadius == radius);

			radius = point.GetRadiusInObjectSpace();
			prevRadius = radius;
		}

		return radius;
	}


	double CylinderObject::GetHeight() const
	{
		assert(m_CylinderSpatialObject->GetPoints().size() == 2);

		auto position = m_CylinderSpatialObject->GetPoints()[1].GetPositionInObjectSpace();
		assert(position.size() == 3);

		auto height = position[2];
		return height;
	}


	/** Print object information */
	void CylinderObject::PrintSelf(std::ostream& os, itk::Indent indent) const
	{
		Superclass::PrintSelf(os, indent);

		if (this->m_CylinderSpatialObject)
		{
			os << indent << this->m_CylinderSpatialObject << std::endl;
		}
	}


} // end namespace igstk
