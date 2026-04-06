#ifndef STDMCViewer_HH
#define STDMCViewer_HH

#include "LKRun.h"
#include "LKTask.h"
#include "LKContainer.h"
#include "LKDetectorSystem.h"
#include "LKDetector.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"
#include "LKHit.h"
#include "LKMCStep.h"
#include "LKMCTrack.h"
#include "TClonesArray.h"

#include "TFile.h"
#include "TH1D.h"
#include "TH1I.h"
#include "TH2Poly.h"
#include "TGraph.h"

class STDMCViewer : public LKTask
{ 
    public:
        STDMCViewer();
        virtual ~STDMCViewer() {}

        bool Init();
        void Exec(Option_t*);
        bool EndOfRun();

        void SetRunNumber(TString run){runNum = run;}

    private:
        TPCDrum *fDetector = nullptr;
        STDPadPlane *fPadPlane = nullptr;

        TClonesArray* fHitArray = nullptr;
        TClonesArray* fStepArray = nullptr;
        TClonesArray* fTrackArray = nullptr;
        LKHit* fHit = nullptr;
        LKMCStep* fStep = nullptr;
        LKMCTrack* fTrack = nullptr;

        // test
        TCanvas* cEvent = nullptr;
        TH2Poly* hPoly = nullptr;
        TH2Poly* hBoundary = nullptr;

        vector<TGraph*> gTrack;
        TF1* hFitter = nullptr;
        TH1I* hHitNum = nullptr;
        TH1I* hRowHitNum = nullptr;
        TH1D* hSumADC = nullptr;

        TString runNum;




    ClassDef(STDMCViewer, 1)
};

#endif