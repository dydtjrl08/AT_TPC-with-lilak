#ifndef STDGatingGridResponse_HH
#define STDGatingGridResponse_HH

#include "LKRun.h"
#include "LKParameterContainer.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"

#include "STDSimTuningManager.h"

#include "TCanvas.h"
#include "TH2Poly.h"

class STDGatingGridResponse
{
    enum GatingGridPar{
        fPCBhit = 0,
        fActiveHit = 1,
        fPass = 2
    };

    public:
        STDGatingGridResponse();
        virtual ~STDGatingGridResponse(){}

        bool Init();

        // Note: x, y, z must set to local coordinate with respect to pad plane
        double GetGatingGridFactor(double x, double y, double z);
        int GetGatingGridIndex(double x, double y, double z);

    private:
        void InitGatingGridMap();
        void InitGatingGridPoly();

        LKRun* fRun;
        LKParameterContainer* fPar;
        TPCDrum *fDetector;
        STDPadPlane *fPadPlane;

        STDSimTuningManager* fTuneManager;
        TH2Poly* fGatingGridPoly;

        double fElectronStepSize;

        bool fOnGatingGrid;
        double fGatingGridHeight; // local coor.
        double fGatingGridVolt;
};

#endif