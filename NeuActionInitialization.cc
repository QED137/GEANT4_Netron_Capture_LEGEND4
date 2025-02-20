#include "NeuActionInitialization.hh"
#include "NeuEventAction.hh"
#include "PrimaryGeneratorAction.hh"

#include "RunAction.hh"
#include "NeuSteppingAction.hh"
#include "NeuTrackingAction.hh"


NeuActionInitialization::NeuActionInitialization(NeuDetectorConstruction* detector) : fDetector(detector) {}




void NeuActionInitialization::BuildForMaster() const
{
    RunAction* runAction = new RunAction(fDetector, nullptr);
  SetUserAction(runAction);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void NeuActionInitialization::Build() const
{
  PrimaryGeneratorAction* primary = new PrimaryGeneratorAction();
  SetUserAction(primary);


  RunAction* runAction = new RunAction(fDetector, primary);
  SetUserAction(runAction);

  NeuEventAction* event = new NeuEventAction();
  SetUserAction(event);

  NeuTrackingAction* trackingAction = new NeuTrackingAction(event);
  SetUserAction(trackingAction);

  NeuSteppingAction* steppingAction = new NeuSteppingAction(event);
  SetUserAction(steppingAction);
}
