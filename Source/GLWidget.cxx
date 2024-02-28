#include <QCoreApplication>
#include <qnamespace.h>

#include "vtkCommand.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

#if defined(__APPLE__) && defined(VTK_USE_CARBON)
#include "vtkCarbonRenderWindow.h"
#endif

#include "qevent.h"

#include "itksys/SystemTools.hxx"

#include "igstkRenderWindowInteractor.h"   
#include "igstkEvents.h"

#include "GLWidget.h"

namespace igstk
{
	GLWidget::GLWidget(QWidget* qparent, Qt::WindowFlags f) :
		QVTKOpenGLNativeWidget(qparent, f), m_StateMachine(this), proxyView(this)
	{
		this->m_Logger = nullptr;
		this->view = ViewType::New();

		igstkAddInputMacro(ValidView);
		igstkAddInputMacro(InValidView);
		igstkAddInputMacro(EnableInteractions);
		igstkAddInputMacro(DisableInteractions);

		igstkAddStateMacro(Idle);
		igstkAddStateMacro(ViewConnected);

		igstkAddTransitionMacro(Idle, ValidView, ViewConnected, ConnectView);

		igstkAddTransitionMacro(Idle, EnableInteractions,
			Idle, ReportInvalidRequest);
		igstkAddTransitionMacro(Idle, DisableInteractions,
			Idle, ReportInvalidRequest);

		igstkAddTransitionMacro(ViewConnected, EnableInteractions,
			ViewConnected, EnableInteractions);
		igstkAddTransitionMacro(ViewConnected, DisableInteractions,
			ViewConnected, DisableInteractions);

		igstkSetInitialStateMacro(Idle);
		m_StateMachine.SetReadyToRun();
	}

	GLWidget::~GLWidget()
	{
		igstkLogMacro(DEBUG, "igstkGLWidget::Destructor called ...\n");
	}

	void GLWidget::SetRenderer(vtkRenderer* renderer)
	{
		this->renderer = renderer;
	}

	void GLWidget::SetRenderWindowInteractor(vtkRenderWindowInteractor* interactor)
	{
		this->renderWindowInteractor = interactor;
	}

	vtkRenderWindowInteractor* GLWidget::GetRenderWindowInteractor() const
	{
		return this->renderWindowInteractor;
	}

	void GLWidget::RequestSetView(const ViewType* view)
	{
		igstkLogMacro(DEBUG, "igstkGLWidget::RequestSetView called ...\n");

		if (view == NULL)
		{
			igstkPushInputMacro(InValidView);
		}
		else
		{
			this->view = const_cast<ViewType*>(view);
			igstkPushInputMacro(ValidView);
		}

		m_StateMachine.ProcessInputs();
	}

	void GLWidget::ConnectViewProcessing()
	{
		igstkLogMacro(DEBUG, "igstkGLWidget::ConnectViewProcessing called ...\n");

		this->proxyView.Connect(this->view);
		SetRenderWindow(this->renderer->GetRenderWindow());
	}

	void GLWidget::RequestEnableInteractions()
	{
		igstkLogMacro(DEBUG, "igstkGLWidget::RequestEnableInteractions() called ...\n");
		igstkPushInputMacro(EnableInteractions);
		m_StateMachine.ProcessInputs();
	}

	void GLWidget::RequestDisableInteractions()
	{
		igstkLogMacro(DEBUG, "igstkGLWidget::RequestDisableInteractions() called ...\n");
		igstkPushInputMacro(DisableInteractions);
		m_StateMachine.ProcessInputs();
	}

	void GLWidget::EnableInteractionsProcessing()
	{
		igstkLogMacro(DEBUG, "igstkGLWidget::EnableInteractionsProcessing() called ...\n");
		this->interactionHandling = true;
	}

	void GLWidget::DisableInteractionsProcessing()
	{
		igstkLogMacro(DEBUG, "igstkGLWidget::DisableInteractionsProcessing() called ...\n");
		this->interactionHandling = false;
	}

	void GLWidget::mousePressEvent(QMouseEvent* e)
	{
		vtkRenderWindowInteractor* interactor = nullptr;
		auto renWin = GetRenderWindow();
		if (renWin)
		{
			interactor = renWin->GetInteractor();
		}

		if (!interactor || !interactor->GetEnabled() || !this->interactionHandling)
		{
			return;
		}

		interactor->SetEventInformationFlipY(
			e->x(), e->y(),
			(e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
			(e->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0,
			0,
			e->type() == QEvent::MouseButtonDblClick ? 1 : 0
		);

		switch (e->button())
		{
		case Qt::LeftButton:
			interactor->InvokeEvent(vtkCommand::LeftButtonPressEvent, e);
			break;

		case Qt::MidButton:
			interactor->InvokeEvent(vtkCommand::MiddleButtonPressEvent, e);
			break;

		case Qt::RightButton:
			interactor->InvokeEvent(vtkCommand::RightButtonPressEvent, e);
			break;

		default:
			break;
		}
	}

	void GLWidget::mouseReleaseEvent(QMouseEvent* e)
	{
		vtkRenderWindowInteractor* interactor = nullptr;
		auto renWin = GetRenderWindow();
		if (renWin)
		{
			interactor = renWin->GetInteractor();
		}

		if (!interactor || !interactor->GetEnabled() || !this->interactionHandling)
		{
			return;
		}

		interactor->SetEventInformationFlipY(e->x(), e->y(),
			(e->modifiers() & Qt::ControlModifier),
			(e->modifiers() & Qt::ShiftModifier));

		switch (e->button())
		{
		case Qt::LeftButton:
		{
			interactor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, e);

			if (!view->isPointRegMode())
			{
				double position[2];
				position[0] = e->x();
				position[1] = height() - e->y();

				this->proxyView.SetPickedPointCoordinates(
					this->view, position[0], position[1]);
			}
			break;
		}
		case Qt::MidButton:
			interactor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, e);
			break;

		case Qt::RightButton:
		{
			interactor->InvokeEvent(vtkCommand::RightButtonReleaseEvent, e);

			if (view->isPointRegMode())
			{
				double position[2];
				position[0] = e->x();
				position[1] = this->height() - e->y();

				this->proxyView.SetPickedPointCoordinates(
					this->view, position[0], position[1]);
			}
			break;
		}
		default:
			break;
		}
	}

	void GLWidget::mouseMoveEvent(QMouseEvent* e)
	{
		vtkRenderWindowInteractor* interactor = nullptr;
		auto renWin = GetRenderWindow();
		if (renWin)
		{
			interactor = renWin->GetInteractor();
		}

		if (!interactor || !interactor->GetEnabled() || !this->interactionHandling)
		{
			return;
		}

		interactor->SetEventInformationFlipY(e->x(), e->y(),
			(e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
			(e->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0);

		interactor->InvokeEvent(vtkCommand::MouseMoveEvent, e);

		if (e->buttons() == Qt::LeftButton)
		{

		}

		if (e->modifiers() & Qt::ShiftModifier) {
			this->proxyView.SetPickedPointCoordinates(this->view, e->x(),
				this->height() - e->y());
		}
	}

#ifndef QT_NO_WHEELEVENT
	void GLWidget::wheelEvent(QWheelEvent* e)
	{
		vtkRenderWindowInteractor* interactor = nullptr;
		auto renWin = GetRenderWindow();
		if (renWin)
		{
			interactor = renWin->GetInteractor();
		}

		if (!interactor || !interactor->GetEnabled() || !this->interactionHandling)
		{
			return;
		}

		interactor->SetEventInformationFlipY(e->x(), e->y(),
			(e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
			(e->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0);

		if (e->delta() > 0)
		{
			interactor->InvokeEvent(vtkCommand::MouseWheelForwardEvent, e);
		}
		else
		{
			interactor->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, e);
		}
	}
#endif

	void GLWidget::showEvent(QShowEvent*)
	{
		if (!isValid())
		{
			qApp->processEvents();
		}
	}

	void GLWidget::ReportInvalidRequestProcessing()
	{
		igstkLogMacro(WARNING, "ReportInvalidRequestProcessing() called ...\n");
	}

	void GLWidget::ReportInvalidViewConnectedProcessing()
	{
		igstkLogMacro(WARNING, "ReportInvalidViewConnectedProcessing() called ...\n");
	}

	void GLWidget::Print(std::ostream& os, ::itk::Indent indent) const
	{
		this->PrintSelf(os, indent);
	}

	std::ostream& operator<<(std::ostream& os, const GLWidget& o)
	{
		o.Print(os);
		return os;
	}

	void GLWidget::PrintSelf(std::ostream& os, itk::Indent indent) const
	{
		os << indent << "RTTI typeinfo:   " << typeid(*this).name() << std::endl;
	}

}
