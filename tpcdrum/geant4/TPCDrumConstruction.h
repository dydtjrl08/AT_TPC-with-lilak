#ifndef TPCDRUMCONSTRUCTION_HH
#define TPCDRUMCONSTRUCTION_HH

#include "LKG4RunManager.h"
#include "LKLogger.h"
#include "LKParameterContainer.h"

#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4FieldManager.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4TransportationManager.hh"
#include "G4Tubs.hh"
#include "G4UniformMagField.hh"
#include "G4UserLimits.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4FastSimulationManager.hh"
#include "G4VFastSimulationModel.hh"
#include "G4ProductionCuts.hh"
#include "globals.hh"

#include "TPCDrum.h"
#include "STDPadPlane.h"

#include "DetectorConstruction.hh"
#include "BeamReaction.hh"
#include "Decay.hh"


class TPCDrumConstruction : public DetectorConstruction
{
    public:
        TPCDrumConstruction();
        virtual ~TPCDrumConstruction(){}

        G4VPhysicalVolume* Construct();

        G4SubtractionSolid* GetGatingGridVolume();

    private:
        G4LogicalVolume* GetWorld();
        G4LogicalVolume* GetTPCDrumChamber();
        G4LogicalVolume* GetBeamWindow();
        G4LogicalVolume* GetReadoutPCB();
        G4LogicalVolume* GetReadoutPadPlane();
        G4LogicalVolume* GetFieldCageSideWall();
        G4LogicalVolume* GetFieldCageCathode();
        G4LogicalVolume* GetFieldCageUpStream();
        G4LogicalVolume* GetFieldCageDownStream();
        G4LogicalVolume* GetGatingGrid();
        G4LogicalVolume* GetSiDetector(TString name);

        G4Material* GetGasMaterial(TString gasName);
        G4Material* GetSolidMaterial(TString name);

        G4VisAttributes* GetColor(TString color, double transparency=0.2);
        
        LKParameterContainer* fPar;
        G4Region* fReactionRegion; 
        vector<G4VFastSimulationModel*> fReactionModel;

    private: 
        // World size 
        const double fWorldX = 2. *m;
        const double fWorldY = 2. *m;
        const double fWorldZ = 2. *m;
        const double fHalfUnit = 0.5;

        TPCDrum* fTPCDrum;
	TString fName = "TPCDrumConstruction";
    };

#endif
