#ifndef STDDriftElectronMaker_HH
#define STDDriftElectronMaker_HH

#include <time.h>
#include <iostream>
#include <cmath>

#include "LKRun.h"
#include "LKTask.h"
#include "LKParameterContainer.h"
#include "LKDetector.h"
#include "LKMCStep.h"
#include "GETChannel.h"
#include "LKMCTag.h"
#include "LKMCTrack.h"

#include "TMath.h"
#include "TRandom3.h"
#include "TVector3.h"
#include "TClonesArray.h"
#include "TH2Poly.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"

#include "STDSimTuningManager.h"
#include "STDGatingGridResponse.h"
#include "STDFieldDistortionMaker.h"

class STDDriftElectronMaker : public LKTask
{
  public:
    STDDriftElectronMaker();
    virtual ~STDDriftElectronMaker() {}

    bool Init();
    void Exec(Option_t*);
    bool EndOfRun();

    // Runtime switch:
    //   true  -> use gating grid attenuation
    //   false -> disable gating grid effect
    void SetUseGatingGrid(bool val = true) { fUseGatingGrid = val; }
    bool GetUseGatingGrid() const { return fUseGatingGrid; }

  private:
    bool BindInputBranches();
    void ConvertCoordinateGeantToPad();
    int GetElectronClusterNum();
    bool DriftElectron(double& x, double& y, double& z, double& t, double& w);
    bool AvalancheElectron(double& x, double& y, double& z, double& t, double& w);

    TPCDrum* fDetector = nullptr;
    STDPadPlane* fPadPlane = nullptr;

    TRandom3* fRandom = nullptr;

    STDSimTuningManager* fTuneManager = nullptr;
    STDGatingGridResponse* fGatingGrid = nullptr;
    STDFieldDistortionMaker* fFieldDistortion = nullptr;

    TClonesArray* fChannelArray = nullptr;
    TClonesArray* fMCTagArray = nullptr;

    // Input branches are bound directly to the input TChain/TTree instead of
    // relying on cached branch pointers from LKRun.
    TClonesArray* fTrackArray = nullptr;
    TClonesArray* fStepArray = nullptr;
    bool fInputBranchesBound = false;

    GETChannel* fChannel = nullptr;
    LKMCTag* fMCTag = nullptr;
    LKMCStep* fStep = nullptr;

    double fElectronStepSize = 0.;
    bool fOnGatingGrid = false;
    bool fUseGatingGrid = true; // runtime configurable

    TVector3 fElectronUnitVec;
    TVector3 fOthogonalUnitVec;
    double fTBTime = 10.;


    double fGEMGainScale = 1.0;


    ClassDef(STDDriftElectronMaker, 1)
};

#endif
