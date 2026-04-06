#include "STDSimTuningManager.h"

STDSimTuningManager* STDSimTuningManager::fInstance = nullptr;

STDSimTuningManager* STDSimTuningManager::GetSimTuningManager()
{
    if (fInstance != nullptr){return fInstance;}
    return new STDSimTuningManager();
}

STDSimTuningManager::STDSimTuningManager()
{
    fInstance = this;

    Init();
}

bool STDSimTuningManager::Init()
{
    cout << "STDSimTuningManager::Init()" << endl;

    fRun = LKRun::GetRun();
    fPar = fRun -> GetParameterContainer();

    fElectronStepSize = 2.; // Default[mm]
    if(fPar->CheckPar("TPCDrum/ElectronStepSize")){
        fElectronStepSize = fPar->GetParDouble("TPCDrum/ElectronStepSize");
    }

    InitGEMGain();

    return true;
}

double STDSimTuningManager::GetWValue(){return 41.;} // test !!! 

double STDSimTuningManager::GetElectronStepSize(){return fElectronStepSize;}

double STDSimTuningManager::GetDriftVelocity(double x, double y, double z){
    double efield = fFieldDistortion -> GetEFieldMag(x, y, z);
    double bfield = fFieldDistortion -> GetBFieldMag(x, y, z);

    return 5.4 * 0.01;// [mm/ns] test !!!
}

double STDSimTuningManager::GetDiffusionT(double x, double y, double z)
{
    double efield = fFieldDistortion -> GetEFieldMag(x, y, z);
    double bfield = fFieldDistortion -> GetBFieldMag(x, y, z);

    return 0.02 * 10./sqrt(10.);// [mm/sqrt{mm}] test !!!
}

double STDSimTuningManager::GetDiffusionL(double x, double y, double z)
{
    double efield = fFieldDistortion -> GetEFieldMag(x, y, z);
    double bfield = fFieldDistortion -> GetBFieldMag(x, y, z);

    return 0.018 * 10./sqrt(10.); // [mm/sqrt{mm}] test !!!
}

double STDSimTuningManager::GetGEMGainFactor(double x, double y, double z)
{
    double gainRatio = fGainCalibration -> GetGEMGainRatio(x, y, z);
    return gainRatio*GetGEMGain();
}

double STDSimTuningManager::GetGEMDiffusionT()
{
    return 0.02 * 10./sqrt(10.);// [mm/sqrt{mm}] test !!!
}

double STDSimTuningManager::GetGEMDiffusionL()
{
    return 0.018 * 10./sqrt(10.); // [mm/sqrt{mm}] test !!!
}

double STDSimTuningManager::GetGEMDriftVelocity()
{
    return 100.; // [mm/ns] test !!!
}

double STDSimTuningManager::GetExtraDiffusionT()
{
    return 0.;
}

void STDSimTuningManager::InitGarfieldData()
{

}

void STDSimTuningManager::InitGEMGain()
{
    // Gain fluctuation distribution based on Polya distribution
    fGEMGainDist = new TF1("function", this, &STDSimTuningManager::PolyaDistribution, 0., 50000., 2);
    fGEMGainDist -> SetParameter(0, 1.5); // M, See the STAR TPC gain fluctuation
    fGEMGainDist -> SetParameter(1, 6500); // Intrincsic gain
}

double STDSimTuningManager::GetGEMGain()
{
    double gain = fGEMGainDist -> GetRandom(); // test !!
    return gain;
}

Double_t STDSimTuningManager::PolyaDistribution(Double_t* x, Double_t* par)
{
    Double_t value = par[0]*pow(par[0]*(x[0]/par[1]), par[0]-1.)/TMath::Gamma(par[0]) * exp(-1. * par[0]*(x[0]/par[1]));
    return value;
}

