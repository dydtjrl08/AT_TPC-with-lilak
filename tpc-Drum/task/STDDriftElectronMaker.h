#ifndef STDDriftElectronMaker_HH
#define STDDriftElectronMaker_HH

#include <time.h>

#include "LKRun.h"
#include "LKTask.h"
#include "LKParameterContainer.h"
#include "LKDetector.h"
#include "LKMCStep.h"
#include "GETChannel.h"
#include "LKMCTag.h"

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

    private:
        void ConvertCoordinateGeantToPad();
        int GetElectronClusterNum();
        bool DriftElectron(double& x, double& y, double& z, double& t, double& w);
        bool AvalancheElectron(double& x, double& y, double& z, double& t, double& w);

        TPCDrum *fDetector;
        STDPadPlane *fPadPlane;

        TRandom3* fRandom;

        STDSimTuningManager* fTuneManager;
        STDGatingGridResponse* fGatingGrid;
        STDFieldDistortionMaker* fFieldDistortion;

        TClonesArray* fChannelArray;
        TClonesArray* fMCTagArray;
        TClonesArray* fTrackArray;
        TClonesArray* fStepArray;

        GETChannel* fChannel;
        LKMCTag* fMCTag;
        LKMCStep* fStep;

        double fElectronStepSize;
        bool fOnGatingGrid;

        TVector3 fElectronUnitVec;
        TVector3 fOthogonalUnitVec;
        double fTBTime;

    ClassDef(STDDriftElectronMaker, 1)
};

#endif