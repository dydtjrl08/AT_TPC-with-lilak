#ifndef STDFieldDistortionMaker_HH
#define STDFieldDistortionMaker_HH

#include "LKRun.h"
#include "LKParameterContainer.h"

#include "TPCDrum.h"
#include "STDPadPlane.h"


class STDFieldDistortionMaker
{
    enum direction{
        dirX = 0,
        dirY = 1,
        dirZ = 2
    };

    public:
        STDFieldDistortionMaker();
        virtual ~STDFieldDistortionMaker(){}

        bool Init();

        double GetEField(double x, double y, double z, int direction=dirZ); // Get E-field value along axis 
        double GetEFieldMag(double x, double y, double z); // Get E-field total value

        double GetBField(double x, double y, double z, int direction=dirZ); // Get B-field component value along direction
        double GetBFieldMag(double x, double y, double z); // Get B-field magnitude

        bool GetElectronDirection(double x, double y, double z, double& dx, double& dy, double& dz);
        
    private:
        void InitEFieldMap();
        void InitBFieldMap();

        bool fInitEFieldFlag;
        bool fInitBFieldFlag;

        LKRun* fRun;
        LKParameterContainer* fPar;
        TPCDrum *fDetector;
        STDPadPlane *fPadPlane;

};

#endif