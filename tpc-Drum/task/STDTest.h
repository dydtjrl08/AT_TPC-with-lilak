#ifndef STDTest_HH
#define STDTest_HH

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


class STDTest : public LKTask
{ 
    enum kPID{
        kNe20 = 0,
        kC12 = 1,
        kAlpha = 2,
        kB8 = 3
    };

    public:
        STDTest();
        virtual ~STDTest() {}

        bool Init();
        void Exec(Option_t*);
        bool EndOfRun();

    private:
        TPCDrum *fDetector;
        STDPadPlane *fPadPlane;

        TClonesArray* fMCTagArray;
        TClonesArray* fTrackArray;
        TClonesArray* fStepArray;

        LKMCTrack* fMCTrack;
        LKMCTag* fMCTag;
        LKMCStep* fStep;


        TRandom3* fRandom;

        STDSimTuningManager* fTuneManager;
        STDGatingGridResponse* fGatingGrid;
        STDFieldDistortionMaker* fFieldDistortion;


        int id_ne20 = 1000100200;
        int id_c12 = 1000060120;
        int id_c12_decay = 1000060129;
        int id_alpha = 1000020040;
        int id_B8 = 1000040080;

        const double siPlaneY = 370.; // [mm]
        // const int 
        TH2D* hPlane[4];
        TH2D* hPlaneAlngpe[4];
        TH1D* hTrkEnergy[4][2];

        static const int siDetNum = 8;
        TClonesArray* fSiStepArray[siDetNum];

        TH1D* hSiEnergy[siDetNum][5];
        TH1D* hSiEnergyByNum[siDetNum][5][4];

        TH2Poly* hBoundaryXY;
        TH2Poly* hBoundaryYZ;
        TH2Poly* fPadPoly;

        Double_t fDynamicRange;
        Double_t fEChargeToADC;
        const Double_t fElectronCharge = 1.6021773349e-19; // [C]
        const Double_t fADCMaxAmp = 4096; // maximum ADC


        double tpcPad[10][12][64][3];
        TGraph* fTrack[10];
        TGraph* fTrackTime[10];
        TF1* fFitTrk[10];
        TF1* fFitTrkTime[10];

        bool fEventFigSave;
        bool fSmaredData;
        TCanvas* cTrk;

        TH1D* hVertex;
        TH2D* hDedx[2];
        TH2D* hECorr[2];
        TH1D* hMomemtum;


        // Reconstructed Data 
        vector<int> fTruthTrkID;
        double fRecoVertex[3];
        double fTruthVretex[3];
        vector<double> fTPCTrkUnit[3]; // [x, y, z] unit vector
        vector<double> fTPCTrkInfo[2]; // [trk length, sum of E]
        vector<double> fTruthTrkInfo[7]; // [px, py, pz, e, length, siE, pid]
        vector<double> fMatchedTrkE[2]; // [si Energy, tpcTrkidx, siTruthID]
        vector<double> fSiInfo[4]; //[x, z, e, truthTrkId]


        bool fIsSaveData;
        TFile* file;
        TTree* tree;

        int outTrkNum;
        double outVertex[2][3]; // [reco, truth][vx, vy, vz]
        double outTPCDedx[10]; // [trkNum]
        double outTrk[10][4]; // [ux, uy, uz, e]
        double outTruthTrk[10][5]; // [px, py, pz, e, pid]

    ClassDef(STDTest, 1)
};

#endif