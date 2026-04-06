#include "STDDriftElectronMaker.h"

ClassImp(STDDriftElectronMaker);

STDDriftElectronMaker::STDDriftElectronMaker()
{
    fName = "STDDriftElectronMaker";
}

bool STDDriftElectronMaker::Init()
{
    fDetector = (TPCDrum *) fRun -> GetDetector();
    fPadPlane = (STDPadPlane*) fDetector -> GetDetectorPlane();

    fChannelArray = fRun -> RegisterBranchA("RawPad", "GETChannel");
    fMCTagArray = fRun -> RegisterBranchA("MCTag", "LKMCTag");
    
    fTrackArray = fRun -> KeepBranchA("MCTrack");
    fStepArray = fRun -> KeepBranchA("MCStepTPCDrum");

    fTBTime = 10.; // [ns]
    if(fPar -> CheckPar("TPCDrum/TBTime")){
        fTBTime = fPar -> GetParDouble("TPCDrum/TBTime");
    }

    fRandom = new TRandom3(0);

    fTuneManager = STDSimTuningManager::GetSimTuningManager();
    fFieldDistortion = new STDFieldDistortionMaker();
    fGatingGrid = new STDGatingGridResponse();

    fFieldDistortion -> Init();
    fGatingGrid -> Init();

    fTuneManager -> SetFieldDostortionMaker(fFieldDistortion);
    fElectronStepSize = fTuneManager->GetElectronStepSize();

    return true;
}

void STDDriftElectronMaker::Exec(Option_t *option)
{
    fChannelArray -> Clear("C");
    fMCTagArray -> Clear("C");

    fRandom -> SetSeed(time(0));

    int stepNum = fStepArray -> GetEntries();
    for(int i=0; i<stepNum; i++){
        fStep = (LKMCStep*)fStepArray -> UncheckedAt(i);

        // Convert coordinate from Geant system to local pad system
        ConvertCoordinateGeantToPad();

        // Checking for TPC-Drum active area
        bool isIn = fDetector -> IsInBoundary(fStep->GetX(), fStep->GetY(), fStep->GetZ());
        if(!isIn){continue;}

        double StartX = fStep -> GetX();
        double StartY = fStep -> GetY();
        double StartZ = fStep -> GetZ();
        double StartT = fStep -> GetTime();
        int trackId = fStep -> GetTrackID(); // Note: Truth track Id for each step will be saved into LKMCTag format.

        int clusterNum = (GetElectronClusterNum()/5.)+1;
        for(int e=0; e<clusterNum; e++){
            double x = StartX;
            double y = StartY;
            double z = StartZ;
            double t = StartT;
            double w = 5.;

            // Make Drift Electron data
            bool driftFlag = DriftElectron(x, y, z, t, w);
            if(!driftFlag){continue;}

            // Avalanch electrons by GEM
            bool avalanchFlag = AvalancheElectron(x, y, z, t, w);
            if(!avalanchFlag){continue;}

            int padID = fPadPlane -> FindPadID(x, y);
            if(padID < 0){continue;}

            unsigned int tb = t/fTBTime;
            if(tb >= 512){continue;}

            fChannel = (GETChannel*)fPadPlane -> GetChannelFast(padID);
            fChannel -> SetAsad(fPadPlane->GetAsAdID(padID));
            fChannel -> SetAget(fPadPlane->GetAgetID(padID));
            fChannel -> SetChan(fPadPlane->GetChanID(padID));
            fChannel -> SetPadID(padID);
            fChannel -> GetBufferArray()[tb] += w;

            fMCTag = (LKMCTag*)fPadPlane -> GetMCTag(padID);
            fMCTag -> AddMCTag(trackId, tb);
        }
    }

    // Save data
    TIter itChannel(fPadPlane -> GetChannelArray());
    int idx = 0;
    while ((fChannel = (GETChannel*)itChannel.Next())) {    
        fMCTag = (LKMCTag*)fPadPlane -> GetMCTag(idx);
        fChannel -> Copy(*(GETChannel*)fChannelArray -> ConstructedAt(idx));
        fMCTag -> Copy(*(LKMCTag*)fMCTagArray -> ConstructedAt(idx));

        fChannel -> Clear();
        fMCTag -> Clear();

        idx++;
    }
}

bool STDDriftElectronMaker::EndOfRun()
{
    return true;
}

void STDDriftElectronMaker::ConvertCoordinateGeantToPad()
{
    double x = fStep -> GetX();
    double y = fStep -> GetY();
    double z = fStep -> GetZ();

    fDetector -> GetCoordinateGeantToPad(x, y, z);

    fStep -> SetX(x);
    fStep -> SetY(y);
    fStep -> SetZ(z);
}

int STDDriftElectronMaker::GetElectronClusterNum()
{
    double e = fStep -> GetEdep() * 1000000.; // [eV]
    double w = fTuneManager->GetWValue();
    int clusterSize = int(e/w);
    return clusterSize;
}

bool STDDriftElectronMaker::DriftElectron(double& x, double& y, double& z, double& t, double& w)
{
    const double GEMSurfaceHeight = 3.* fDetector->fGEMSpacing; // [mm]

    while(1){
        if(z <= GEMSurfaceHeight){return true;} // break if z lower then triple GEM height

        // Step1: Calculate the Gating Grid effect 
        double GatingGridFactor = fGatingGrid -> GetGatingGridFactor(x, y, z);
        if(GatingGridFactor < 0){return false;} // break if electron deposit in Gating Grid PCB

        // Step2: Get the electron direction using Field map
        double dirX, dirY, dirZ;
        fFieldDistortion -> GetElectronDirection(x, y, z, dirX, dirY, dirZ);
        fElectronUnitVec.SetXYZ(dirX, dirY, dirZ);

        // Step3: Generate random diffusion with respect to electron direction vector
        double sigmaT = fTuneManager -> GetDiffusionT(x, y, z) * sqrt(fElectronStepSize); // [mm]
        double sigmaL = fTuneManager -> GetDiffusionL(x, y, z) * sqrt(fElectronStepSize); // [mm]
        double velocityD = fTuneManager -> GetDriftVelocity(x, y, z); // [mm/ns]

        double dr = fRandom -> Gaus(0., sigmaT);
        double dt = fRandom -> Gaus(0, sigmaL)/velocityD;
        double phi = fRandom -> Uniform(2*TMath::Pi());

        fOthogonalUnitVec = fElectronUnitVec.Orthogonal(); // Get the transverse plane with respect to electron direction vector
        fOthogonalUnitVec.Rotate(phi, fElectronUnitVec); // Rotate random angle along electron direction
        fOthogonalUnitVec.SetMag(dr); // Set diffusion R

        // Step4: re-scale electron step size and summed vector
        fElectronUnitVec.SetMag(fElectronStepSize);

        x = x + fElectronUnitVec.X() + fOthogonalUnitVec.X();
        y = y + fElectronUnitVec.Y() + fOthogonalUnitVec.Y();
        z = z + fElectronUnitVec.Z() + fOthogonalUnitVec.Z();
        t = t + fElectronStepSize/velocityD + dt;
        w = (w < GatingGridFactor*w)? w : GatingGridFactor*w;
    }

    return false;
}

bool STDDriftElectronMaker::AvalancheElectron(double& x, double& y, double& z, double& t, double& w)
{
    double extraGainFactor = 1.; // for beamline
    if(-10. <= x && x <= 10.){extraGainFactor = 0.5;}
    double gain = fTuneManager -> GetGEMGainFactor(x, y, z) * extraGainFactor;
    double sigmaT = fTuneManager -> GetGEMDiffusionT() * sqrt(z); // [mm]
    double sigmaL = fTuneManager -> GetGEMDiffusionL() * sqrt(z); // [mm]
    double velocityD = fTuneManager -> GetGEMDriftVelocity(); // [mm/ns]

    double dr = fRandom -> Gaus(0., sigmaT);
    double dt = fRandom -> Gaus(0, sigmaL)/velocityD;
    double phi = fRandom -> Uniform(2*TMath::Pi());

    double dx = dr * cos(phi);
    double dy = dr * sin(phi);

    x = x + dx;
    y = y + dy;
    z = 0.;
    t = t + dt;
    w = w * gain;

    if(w < 0){return false;}
    return true;
}
