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

#include "igstkView2D.h"
#include "igstkView3D.h"

#include "GLWidget.h"

#include <QTimer>
#include <QLayout>

namespace igstk
{
	GLWidget::GLWidget(QWidget* qparent, Qt::WindowFlags f) :
		QVTKOpenGLNativeWidget(qparent, f), m_StateMachine(this), proxyView(this)
	{
		setAttribute(Qt::WA_AcceptTouchEvents);

		this->m_Logger = nullptr;
		this->view = View::New();

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

		pinchGuideLabel = new QLabel(this);
		pinchGuidePixmap = QPixmap(":/Resources/icons/pinchGuide.png");
		pinchGuideLabel->setPixmap(pinchGuidePixmap);
		pinchGuideLabel->hide();
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
		renderWindowInteractor->SetRecognizeGestures(true);
	}

	vtkRenderWindowInteractor* GLWidget::GetRenderWindowInteractor() const
	{
		return this->renderWindowInteractor;
	}

	void GLWidget::RequestSetView(const View* view)
	{
		igstkLogMacro(DEBUG, "igstkGLWidget::RequestSetView called ...\n");

		if (view == nullptr)
		{
			igstkPushInputMacro(InValidView);
		}
		else
		{
			this->view = const_cast<View*>(view);
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
			lastPressedEventId = vtkCommand::LeftButtonPressEvent;
			break;

		case Qt::MiddleButton:
			interactor->InvokeEvent(vtkCommand::MiddleButtonPressEvent, e);
			lastPressedEventId = vtkCommand::MiddleButtonPressEvent;
			break;

		case Qt::RightButton:
			showPinchGuide(geometry());
			lastPressedEventId = vtkCommand::RightButtonPressEvent;
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
			if (doubleClicked)
			{
				interactor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, e);
				doubleClicked = false;
			}
			else
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
			}
			break;
		}
		case Qt::MiddleButton:
			interactor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, e);
			break;

		case Qt::RightButton:
		{
			pinchGuideLabel->hide();
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
		do
		{
			auto renWin = GetRenderWindow();
			if (!renWin)
			{
				break;
			}

			interactor = renWin->GetInteractor();

			if (!interactor || !interactor->GetEnabled() || !this->interactionHandling)
			{
				break;
			}

			if (touching)
			{
				ignorePinchGuide = true;
			}

			if (lastPressedEventId != vtkCommand::RightButtonPressEvent)
			{
				pinchGuideLabel->hide();
			}

			auto view3DCond = (dynamic_cast<View3D*>(view) &&
				(e->buttons() == Qt::RightButton || e->modifiers() == Qt::ShiftModifier));

			auto view2DCond = (dynamic_cast<View2D*>(view) && e->modifiers() != Qt::ControlModifier &&
				(e->modifiers() == Qt::ShiftModifier || e->buttons() == Qt::LeftButton));

			if (!doubleClicked && (view2DCond || view3DCond))
			{
				magneticPicking(e);
			}
			else
			{
				interactor->SetEventInformationFlipY(e->x(), e->y(),
					(e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
					(e->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0);

				interactor->InvokeEvent(vtkCommand::MouseMoveEvent, e);
			}

		} while (false);
	}

	void GLWidget::magneticPicking(QMouseEvent* e)
	{
		prevFocusedPoint = e->pos();

		if (0 == frameSkipWeight % 5)
		{
			this->proxyView.SetPickedPointCoordinates(this->view, e->x(),
				this->height() - e->y());

			lastFocusedPoint = e->pos();

			QTimer::singleShot
			(0, [&]() -> void
				{
					if (lastFocusedPoint != prevFocusedPoint)
					{
						this->proxyView.SetPickedPointCoordinates(this->view, prevFocusedPoint.x(),
							this->height() - prevFocusedPoint.y());
					}

					lastFocusedPoint = prevFocusedPoint;
					frameSkipWeight = 0;
				}
			);
		}
		frameSkipWeight = (frameSkipWeight % 5) + 1;
	}

	void GLWidget::mouseDoubleClickEvent(QMouseEvent* e)
	{
		if (Qt::LeftButton != e->button())
		{
			return;
		}

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

		interactor->InvokeEvent(vtkCommand::MiddleButtonPressEvent, e);
		doubleClicked = true;
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

	void GLWidget::showEvent(QShowEvent* evt)
	{
		if (!isValid())
		{
			qApp->processEvents();
		}

		QVTKOpenGLNativeWidget::showEvent(evt);
	}

	void GLWidget::showPinchGuide(QRect geo)
	{
		auto innerLayout = layout();
		if (innerLayout)
		{
			pinchGuideLabel->setGeometry(innerLayout->geometry());
		}
		else
		{
			// calc pixmap and label geometry to GLWidget size.
		}

		pinchGuideLabel->show();
	}

	bool GLWidget::event(QEvent* ev)
	{
		if (ev->type() == QEvent::TouchBegin)
		{
			touchBeginTimer.restart();
			touching = true;
			QTimer::singleShot(700, [&]() -> void
				{
					if (!touching || ignorePinchGuide)
					{
						return;
					}

					auto beginTime = touchBeginTimer.elapsed();
					auto endTime = touchEndTimer.elapsed();

					if (beginTime >= 690 && beginTime < endTime)
					{
						showPinchGuide(geometry());
					}
				});

			return true;
		}
		else if (ev->type() == QEvent::TouchEnd)
		{
			touchEndTimer.restart();
			ignorePinchGuide = false;
			touching = false;
			QTimer::singleShot(0, [&]() -> void
				{
					pinchGuideLabel->hide();
				});
			return true;
		}

		return Superclass::event(ev);
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
