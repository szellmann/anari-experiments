#include "vtkColorTransferFunction.h"
#include "vtkDataSetTriangleFilter.h"
#include "vtkImageCast.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkNew.h"
#include "vtkPiecewiseFunction.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkTesting.h"
#include "vtkThreshold.h"
#include "vtkUnstructuredGridVolumeRayCastMapper.h"
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"
#include "vtkArrayCalculator.h"

#include "vtkAnariPass.h"
#include "vtkAnariRendererNode.h"

struct IdleCallback : vtkCommand
{
  vtkTypeMacro(IdleCallback, vtkCommand);

  static IdleCallback *New() {
    return new IdleCallback;
  }

  void Execute(vtkObject * vtkNotUsed(caller),
               unsigned long vtkNotUsed(eventId),
               void * vtkNotUsed(callData))
  {
    renderWindow->Render();
  }

  vtkRenderWindow *renderWindow;
};

int main(int argc, char *argv[]) {
  vtkNew<vtkUnstructuredGridReader> reader;
  reader->SetFileName(argv[1]);
  reader->Update();
  reader->Print(cout);

  double range[] = { 242616.,259745. };

  // Create transfer mapping scalar value to opacity
  vtkNew<vtkPiecewiseFunction> opacityTransferFunction;
  opacityTransferFunction->AddPoint(242616., 0.0);
  opacityTransferFunction->AddPoint(249616., 0.0);
  opacityTransferFunction->AddPoint(250745., 0.1);
  opacityTransferFunction->AddPoint(251745., 0.0);
  opacityTransferFunction->AddPoint(259745., 0.0);
  opacityTransferFunction->AdjustRange(range);

  // Create transfer mapping scalar value to color
  vtkNew<vtkColorTransferFunction> colorTransferFunction;
  colorTransferFunction->AddRGBPoint(242616., 0.0, 0.0, 0.0);
  colorTransferFunction->AddRGBPoint(249916., 1.0, 0.0, 0.0);
  colorTransferFunction->AddRGBPoint(250745., 0.0, 1.0, 0.0);
  colorTransferFunction->AddRGBPoint(251245., 0.0, 0.0, 1.0);
  colorTransferFunction->AddRGBPoint(259745., 0.0, 0.0, 0.0);
  colorTransferFunction->AdjustRange(range);

  // The property describes how the data will look
  vtkNew<vtkVolumeProperty> volumeProperty;
  volumeProperty->SetColor(colorTransferFunction);
  volumeProperty->SetScalarOpacity(opacityTransferFunction);
  volumeProperty->ShadeOff();
  volumeProperty->SetInterpolationTypeToLinear();

  // The mapper / ray cast function know how to render the data
  vtkNew<vtkUnstructuredGridVolumeRayCastMapper> volumeMapper;
  volumeMapper->SetInputConnection(reader->GetOutputPort());

  // The volume holds the mapper and the property and
  // can be used to position/orient the volume
  vtkNew<vtkVolume> volume;
  volume->SetMapper(volumeMapper);
  volume->SetProperty(volumeProperty);

  vtkNew<vtkRenderer> ren1;
  ren1->AddVolume(volume);

  // Create the renderwindow, interactor and renderer
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetMultiSamples(0);
  renderWindow->SetSize(401, 399); // NPOT size
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renderWindow);
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  iren->SetInteractorStyle(style);
  ren1->SetBackground(0.3, 0.3, 0.4);
  renderWindow->AddRenderer(ren1);

  ren1->ResetCamera();

  // Attach ANARI render pass
  vtkNew<vtkAnariPass> anariPass;
  ren1->SetPass(anariPass);

  vtkAnariRendererNode::SetLibraryName("environment", ren1);
  vtkAnariRendererNode::SetSamplesPerPixel(6, ren1);
  vtkAnariRendererNode::SetLightFalloff(.5, ren1);
  vtkAnariRendererNode::SetUseDenoiser(1, ren1);
  vtkAnariRendererNode::SetCompositeOnGL(1, ren1);

  vtkNew<IdleCallback> idleCallback;
  idleCallback->renderWindow = renderWindow;
  iren->CreateRepeatingTimer(1);
  iren->AddObserver(vtkCommand::TimerEvent, idleCallback);
  iren->Start();

  return 0;
}


