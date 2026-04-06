#ifndef STDPulseAnalyser_HH
#define STDPulseAnalyser_HH

#include "LKRun.h"
#include "LKTask.h"
#include "LKContainer.h"
#include "LKDetectorSystem.h"
#include "LKDetector.h"

#include "TProfile.h"
#include "TSystem.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"
#include "GETChannel.h"
#include "LKHit.h"
#include "TClonesArray.h"

class STDPulseAnalyser : public LKTask
{ 
    public:
        STDPulseAnalyser();
        virtual ~STDPulseAnalyser() {}

        bool Init();
        void Exec(Option_t*);
        bool EndOfRun();

        void SetMethod(int opt);

        void MakePulseShape(){fMakePulseShapeMode = true;} // pulse template analysis

    private:
        void GetMaxPoint(int* adcArray, double& maxADC, double& maxTB);
        vector<pair<double, double>> GetMultiPoint(int* adcArray);

        void InitPulseTemplate();
        void FillPulseShape();
        void PulseShapeAnalyser();

        Double_t SinglePulseFitter(Double_t* x, Double_t* par);
        Double_t MultiPulseFitter(Double_t* x, Double_t* par);

        TPCDrum *fDetector = nullptr;
        STDPadPlane *fDetectorPlane = nullptr;

        TClonesArray* fChannelArray = nullptr;
        GETChannel* fChannel = nullptr;

        TClonesArray* fHitArray = nullptr;
        LKHit* fHit = nullptr;

        int fMethodOpt; // [0 == Maximum Point, to be updated]

        double fThreshold = 50.;
        int fTBStartIdx = 30;
        int fTBEndIdx = 450;

        // Pulse shape template 
        bool fMakePulseShapeMode;
        TString fPulseTemplatePath;
        TH2D* fPulseSum = nullptr;

        double fMaxADCBase = 100.;
        static const int fPulseTemplateArrNum = 300;
        double fPulseTemplate[fPulseTemplateArrNum];

        TF1* fPulseFitter = nullptr;

    ClassDef(STDPulseAnalyser, 1)
};

#endif
