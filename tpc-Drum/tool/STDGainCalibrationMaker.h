#ifndef STDGainCalibrationMaker_HH
#define STDGainCalibrationMaker_HH

#include "LKRun.h"
#include "LKParameterContainer.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"


class STDGainCalibrationMaker
{
    public:
        STDGainCalibrationMaker();
        virtual ~STDGainCalibrationMaker(){}

        bool Init();

        double GetGEMGainRatio(int pad);
        double GetGEMGainRatio(int layer, int row);

        double GetGEMGainRatio(double x, double y, double z);

    private:
        void InitGainRatioMap();

        bool fInitGainRatioFlag;

        LKRun* fRun;
        LKParameterContainer* fPar;
        TPCDrum *fDetector;
        STDPadPlane *fPadPlane;

};

#endif