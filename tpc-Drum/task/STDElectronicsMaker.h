#ifndef STDElectronicsMaker_HH
#define STDElectronicsMaker_HH

#include <time.h>
#include <vector>
#include <tuple>

#include "LKRun.h"
#include "LKTask.h"
#include "LKParameterContainer.h"
#include "LKDetector.h"
#include "GETChannel.h"
#include "LKMCTag.h"

#include "TRandom3.h"
#include "TClonesArray.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"

#include "STDSimTuningManager.h"

class STDElectronicsMaker : public LKTask
{ 
    public:
        STDElectronicsMaker();
        virtual ~STDElectronicsMaker() {}

        bool Init();
        void Exec(Option_t*);
        bool EndOfRun();

    private:
        TPCDrum *fDetector;
        STDPadPlane *fPadPlane;

        TRandom3* fRandom;

        STDSimTuningManager* fTuneManager;

        TClonesArray* fChannelArray;
        TClonesArray* fMCTagArray;

        GETChannel* fChannel;
        LKMCTag* fMCTag;

        Double_t fDynamicRange;
        Double_t fEChargeToADC;
        const Double_t fElectronCharge = 1.6021773349e-19; // [C]
        const Double_t fADCMaxAmp = 4096; // maximum ADC

    ClassDef(STDElectronicsMaker, 1)
};

#endif