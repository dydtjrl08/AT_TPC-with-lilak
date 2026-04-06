#include "STDGainCalibrationMaker.h"

STDGainCalibrationMaker::STDGainCalibrationMaker() : fInitGainRatioFlag(false)
{
}

bool STDGainCalibrationMaker::Init()
{
    cout << "STDGainCalibrationMaker::Init()" << endl;

    fRun = LKRun::GetRun();
    fPar = fRun -> GetParameterContainer();

    bool onGainRatioMode = false;
    if(fPar->CheckPar("TPCDrum/GEMGainRatioMode")){
        onGainRatioMode = fPar->GetParBool("TPCDrum/GEMGainRatioMode");
    }
    if(onGainRatioMode){
        InitGainRatioMap();
    }

    return true;
}

double STDGainCalibrationMaker::GetGEMGainRatio(int pad)
{
    return 1.;
}

double STDGainCalibrationMaker::GetGEMGainRatio(int layer, int row)
{
    
    return 1.;
}

double STDGainCalibrationMaker::GetGEMGainRatio(double x, double y, double z)
{
    // To be updated

    return 1.;
}

void STDGainCalibrationMaker::InitGainRatioMap()
{
    // To be updated 
    fInitGainRatioFlag = true;
}