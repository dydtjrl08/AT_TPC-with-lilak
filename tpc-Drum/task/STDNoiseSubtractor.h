#ifndef STDNoiseSubtractor_HH
#define STDNoiseSubtractor_HH

#include "LKRun.h"
#include "LKTask.h"
#include "LKContainer.h"
#include "LKDetectorSystem.h"
#include "LKDetector.h"

#include "SejongDAQFlow.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"
#include "GETChannel.h"
#include "TClonesArray.h"

#include "TH1D.h"
#include "TProfile.h"


class STDNoiseSubtractor : public LKTask
{ 
    public:
        STDNoiseSubtractor();
        virtual ~STDNoiseSubtractor() {}

        bool Init();
        void Exec(Option_t*);
        bool EndOfRun();

        void DrawRawADCPad(){fOnDrawRawADC = true;}
        void MakeNoiseShape(){fMakeNoiseShapeMode = true;}

    private:
        void InitNoiseTemplateSaving();
        void SaveNoiseTemplate();

        TPCDrum *fDetector = nullptr;
        STDPadPlane *fDetectorPlane = nullptr;

        TClonesArray* fChannelArray = nullptr;
        GETChannel* fChannel = nullptr;

        TH1D* fADCTmp = nullptr;
        TH2D* fADCTemplate[ASADNUM];
        TProfile* fNoiseTemplate[ASADNUM];

        bool fMakeNoiseShapeMode;
        TFile* fNoiseFile;
        TTree* fNoiseTree;
        Double_t fNoiseData[TIMEBUCKET];

        bool fOnDrawRawADC;
        TCanvas* cRawPad = nullptr;
        TH2Poly* hBoundary = nullptr;
        TH2Poly* hPolyADC_subt = nullptr;
        TH2Poly* hPolyTime_subt = nullptr;
        TH2D* hPolyTime_Y = nullptr;
        TH2D* hTB_subt = nullptr;

        TH1D* hTestTB;
        TH1D* hTestRawTB;
        TCanvas* cTestCanvas;


        int fTBStartIdx = 30;
        int fTBEndIdx = 508;
        int fTBIdxNum = 40; // for remove the pulse-like-pad
        int fTBInterval = 10; // for remove the pulse-like-pad

        double fTmpADCOffset = 1000.;

    ClassDef(STDNoiseSubtractor, 1)
};

#endif