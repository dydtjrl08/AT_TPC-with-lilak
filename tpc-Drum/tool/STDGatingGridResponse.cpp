#include "STDGatingGridResponse.h"

STDGatingGridResponse::STDGatingGridResponse()
{
}

bool STDGatingGridResponse::Init()
{
    cout << "STDGatingGridResponse::Init()" << endl;

    fRun = LKRun::GetRun();
    fPar = fRun -> GetParameterContainer();
    fDetector = (TPCDrum *) fRun -> GetDetector();
    fPadPlane = (STDPadPlane*) fDetector -> GetDetectorPlane();

    fTuneManager = STDSimTuningManager::GetSimTuningManager();
    fElectronStepSize = fTuneManager->GetElectronStepSize();

    if(fPar->CheckPar("TPCDrum/GatingGrid")){
        fOnGatingGrid = fPar -> GetParBool("TPCDrum/GatingGrid");
    }
    if(!fOnGatingGrid){return true;}

    InitGatingGridMap();
    InitGatingGridPoly();

    // Get Gating Grid height
    double geantX = 0.;
    double geantY = -1.*(fDetector->GetGatingGridGeantYPos()+fDetector->fPCBThickness/2.); // position Y on the Gating grid above surface
    double geantZ = 0.;
    fDetector->GetCoordinateGeantToPad(geantX, geantY, geantZ);
    fGatingGridHeight = geantZ;

    // Get Gating Grid different voltage
    fGatingGridVolt = 80; // default [V]
    if(fPar->CheckPar("TPCDrum/GatingGridVolt")){
        fGatingGridVolt = fPar->GetParDouble("TPCDrum/GatingGridVolt");
    }

    return true;
}

int STDGatingGridResponse::GetGatingGridIndex(double x, double y, double z)
{
    if(fOnGatingGrid){
        if(fGatingGridHeight-1. > z){return fPass;}
        if(fabs(fGatingGridHeight - z) < fElectronStepSize){
            int binIdx = fGatingGridPoly -> FindBin(x, y);
            if(binIdx == 1){return fActiveHit;}
            if(binIdx == 2 || binIdx == 3){return fPCBhit;}
        }
    }
    return fPass;
}

double STDGatingGridResponse::GetGatingGridFactor(double x, double y, double z)
{
    int gatinggridIdx = GetGatingGridIndex(x, y, z);
    if(gatinggridIdx == fPass){
        return 1.;
    } 
    if(gatinggridIdx == fPCBhit){
        return -1.;
    }

    // Make gating grid reducing factor
    const double testFactor = 0.15; // test !!!

    return testFactor;
}

void STDGatingGridResponse::InitGatingGridMap()
{
    // To be updated
}

void STDGatingGridResponse::InitGatingGridPoly()
{
    fGatingGridPoly = new TH2Poly();

    double pcbWidth = fDetector->fGatingGridActivePCBWidth; // [mm]
    double activeWidth = fDetector->fGatingGridActiveWidth; // [mm]
    double heightY = fDetector->fFieldCageLength; // [mm]

    double tmpX = 0.;
    double tmpY = 0;
    double tmpZ = 0.;
    fDetector->GetCoordinateGeantToPad(tmpX, tmpY, tmpZ);
    double padCenterY = tmpY;

    double x[5];
    double y[5];

    // make bin for gating grid active area
    for(int i=0; i<5; i++){
        double xSign = (i<2 || i==4)? -1. : +1.;
        double ySign = (i==1 || i==2)? -1. : +1.;
        x[i] = xSign*activeWidth/2.;
        y[i] = ySign*heightY/2.+padCenterY;
    }
    fGatingGridPoly -> AddBin(5, x, y);

    // make bin for gating grid pcb
    for(int vol=0; vol<2; vol++){
        double lrSign = (vol==0)? +1. : -1.;
        for(int i=0; i<5; i++){
            double xSign = (i<2 || i==4)? -1. : +1.;
            double ySign = (i==1 || i==2)? -1. : +1.;
            x[i] = lrSign*(activeWidth/2. + pcbWidth/2.) + xSign*pcbWidth/2.;
            y[i] = ySign*heightY/2.+padCenterY;
        }
        fGatingGridPoly -> AddBin(5, x, y);
    }
}