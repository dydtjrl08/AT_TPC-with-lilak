#ifndef STDChannelViewer_HH
#define STDChannelViewer_HH

#include "LKRun.h"
#include "LKTask.h"
#include "LKContainer.h"
#include "LKDetectorSystem.h"
#include "LKDetector.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"
#include "LKHit.h"
#include "TClonesArray.h"

#include "TFile.h"
#include "TH1D.h"
#include "TH1I.h"
#include "TH2Poly.h"
#include "TGraph.h"

class STDChannelViewer : public LKTask
{ 
    public:
        STDChannelViewer();
        virtual ~STDChannelViewer() {}

        bool Init();
        void Exec(Option_t*);
        bool EndOfRun();

        void SetRunNumber(TString run){runNum = run;}

    private:
        TPCDrum *fDetector = nullptr;
        STDPadPlane *fPadPlane = nullptr;

        TClonesArray* fHitArray = nullptr;
        LKHit* fHit = nullptr;

        // test
        TCanvas* cEvent = nullptr;
        TH2Poly* hPoly = nullptr;
        TH2Poly* hBoundary = nullptr;

        TGraph* gCluster = nullptr;
        TF1* hFitter = nullptr;
        TH1I* hHitNum = nullptr;
        TH1I* hRowHitNum = nullptr;
        TH1D* hSumADC = nullptr;
        TH2D* hHitADC = nullptr;

        TFile* outFile;
        TTree* outTree;
        double fit[3]; // [p0, p1, limit y pos]

        TString runNum;




    ClassDef(STDChannelViewer, 1)
};

#endif