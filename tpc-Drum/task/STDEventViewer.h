#ifndef STDEventViewer_HH
#define STDEventViewer_HH

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

class STDEventViewer : public LKTask
{ 
    public:
        STDEventViewer();
        virtual ~STDEventViewer() {}

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
        TH2Poly* hPoly2 = nullptr;
        TH2Poly* hBoundary = nullptr;

        TH1D* hSumADC[4];// section
        TH1I* hHitNum = nullptr;

        TFile* outFile;
        TString runNum;




    ClassDef(STDEventViewer, 1)
};

#endif