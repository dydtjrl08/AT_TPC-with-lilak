#ifndef STDTest2_HH
#define STDTest2_HH

#include <time.h>

#include "LKRun.h"
#include "LKTask.h"
#include "LKParameterContainer.h"
#include "LKDetector.h"
#include "LKMCTrack.h"
#include "LKMCStep.h"
#include "GETChannel.h"
#include "LKMCTag.h"

#include "TMath.h"
#include "TRandom3.h"
#include "TVector3.h"
#include "TClonesArray.h"
#include "TH2Poly.h"
#include "TH1D.h"
#include "TF1.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"

#include "STDSimTuningManager.h"
#include "STDGatingGridResponse.h"
#include "STDFieldDistortionMaker.h"


class STDTest2 : public LKTask
{ 
    enum kPID{
        kNe20 = 0,
        kC12 = 1,
        kAlpha = 2,
        kB8 = 3
    };

    public:
        STDTest2();
        virtual ~STDTest2() {}

        bool Init();
        void Exec(Option_t*);
        bool EndOfRun();

    private:
        TPCDrum *fDetector;
        STDPadPlane *fPadPlane;

        TClonesArray* fTrackArray;
        TClonesArray* fStepArray;

        static const int siDetNum = 8;
        TClonesArray* fSiStepArray[siDetNum];

        LKMCTrack* fMCTrack;
        LKMCTag* fMCTag;
        LKMCStep* fStep;

        int id_ne20 = 1000100200;
        int id_c12 = 1000060120;
        int id_c12_decay = 1000060129;
        int id_alpha = 1000020040;
        int id_B8 = 1000040080;

        TFile* file;
        TTree* tree;

        int outTrkNum;
        double outVertex[2][3]; // [reco, truth][vx, vy, vz]
        double outTPCDedx[10]; // [trkNum]
        double outTrk[10][4]; // [ux, uy, uz, e]
        double outTruthTrk[10][5]; // [px, py, pz, e, pid]


    ClassDef(STDTest2, 1)
};

#endif