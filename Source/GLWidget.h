#ifndef __igstkGLWidget_h
#define __igstkGLWidget_h

#define QT_NO_DEBUG 1

#include "itkCommand.h"
#include "itkLogger.h"

#include "igstkMacros.h"
#include "igstkStateMachine.h"
#include "igstkViewProxy.h"

#include "QVTKOpenGLNativeWidget.h"
#include <QLabel>
#include <QElapsedTimer>
#include <QPoint>

namespace igstk {

	class GLWidget : public QVTKOpenGLNativeWidget
	{
	public:
		typedef GLWidget Self;
		typedef QVTKOpenGLNativeWidget Superclass;

		igstkTypeMacro(GLWidget, QVTKOpenGLNativeWidget);
		igstkStateMachineMacro();

		void Print(std::ostream&, ::itk::Indent indent = 0) const;

		igstkLoggerMacro();

		GLWidget(QWidget* parent = nullptr, Qt::WindowFlags f = 0);
		virtual ~GLWidget(void);

		void RequestSetView(const View*);
		typedef ViewProxy< GLWidget > ProxyType;
		friend class ViewProxy< GLWidget >;
		void RequestDisableInteractions();
		void RequestEnableInteractions();

		vtkRenderWindowInteractor* GetRenderWindowInteractor() const;

	protected:
		virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

		void mouseReleaseEvent(QMouseEvent* e);
		void mouseMoveEvent(QMouseEvent* e);
		void magneticPicking(QMouseEvent* e);
		void mouseDoubleClickEvent(QMouseEvent*);
		void mousePressEvent(QMouseEvent* e);
		void wheelEvent(QWheelEvent* e);
		void showEvent(QShowEvent*);
		bool event(QEvent*) override;

		vtkCommand::EventIds lastPressedEventId;
		bool doubleClicked = false;
		bool touching = false;
		bool ignorePinchGuide = false;
		QElapsedTimer touchBeginTimer;
		QElapsedTimer touchEndTimer;

		void showPinchGuide(QRect);
		QLabel* pinchGuideLabel = nullptr;
		QPixmap pinchGuidePixmap;

	private:
		void ReportInvalidRequestProcessing();
		void ReportInvalidViewConnectedProcessing();
		void ConnectViewProcessing();
		void DisableInteractionsProcessing();
		void EnableInteractionsProcessing();
		void SetRenderer(vtkRenderer* renderer);
		void SetRenderWindowInteractor(vtkRenderWindowInteractor* interactor);

	private:
		igstk::View* view = nullptr;;
		ProxyType proxyView;
		vtkRenderer* renderer = nullptr;
		vtkRenderWindowInteractor* renderWindowInteractor = nullptr;
		bool interactionHandling = true;

		int frameSkipWeight = 0;

		QPoint prevFocusedPoint;
		QPoint lastFocusedPoint;

		igstkDeclareStateMacro(Idle);
		igstkDeclareStateMacro(ViewConnected);

		igstkDeclareInputMacro(ValidView);
		igstkDeclareInputMacro(InValidView);
		igstkDeclareInputMacro(EnableInteractions);
		igstkDeclareInputMacro(DisableInteractions);

	};

	std::ostream& operator<<(std::ostream& os, const GLWidget& o);
}

#endif
