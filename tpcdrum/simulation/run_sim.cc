#include "LKG4RunManager.h"
#include "LKParameterContainer.h"

#include <time.h>
#include "globals.hh"
#include "Randomize.hh"

// Geant4 
#include "G4UImanager.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4LossTableManager.hh"
#include "G4EmConfigurator.hh"
#include "G4PAIModel.hh"
#include "G4Electron.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

// LILAK
#include "TPCDrumConstruction.h"
#include "LKPrimaryGeneratorAction.h"

// NPTool
#include "NPOptionManager.h"
#include "RootOutput.h"
#include "PrimaryGeneratorAction.hh"
#include "PhysicsList.hh"


// Just debugging
#include <TString.h>
#include "TSystem.h"
#include "TApplication.h"
#include "TROOT.h"


int main(int argc, char** argv)
{
//    ROOT::EnableThreadSafety();   

//    TApplication app("app", &argc, argv);


//    gSystem -> Load("libLILAK.so");
	
    auto runManager = new LKG4RunManager();
    cout << "1 " << endl;
    
    if (argv[1] == nullptr) cout << "No argv" << endl;

    runManager -> AddParameterContainer(argv[1]);
    cout << " 1" << endl;
    auto par = runManager -> GetParameterContainer();
   
  
    TString fName = "run_sim";
	
    lk_info << "middle" << endl; 

    TString reactionFile = par->GetParString("NPTool/ReactionFile");
    lk_info << "reactionFile : " << reactionFile << endl;

    NPOptionManager::getInstance()->SetIsSimulation();
    NPOptionManager::getInstance()->SetReactionFile(reactionFile.Data());

//    lk_info << NPOptionManager::getInstance()->GetReactionFile() << endl;

    gRandom->SetSeed(time(0));
    CLHEP::HepRandom::setTheSeed(time(0), 3);
    G4Random::setTheSeed(time(0));
    G4Random::setTheEngine(new CLHEP::RanecuEngine);
    
    TPCDrumConstruction* detector = new TPCDrumConstruction();
    runManager -> SetUserInitialization(detector);

    PhysicsList* NPPhysicsList = new PhysicsList();
    runManager -> SetUserInitialization(NPPhysicsList);
    lk_info << "stop before runManager Initialization." << endl;
    std::cin.get();

    runManager -> Initialize();
    lk_info << "isgood? " << endl;

    PrimaryGeneratorAction* primary = new PrimaryGeneratorAction(detector);
    primary->ReadEventGeneratorFile(reactionFile.Data());
    runManager->SetUserAction(primary);

    runManager -> Run(argc, argv);



    delete runManager;


    return 0;
}
