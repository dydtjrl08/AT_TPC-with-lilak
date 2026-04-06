#include "STDDriftElectronMaker.h"

ClassImp(STDDriftElectronMaker);

using namespace std;

STDDriftElectronMaker::STDDriftElectronMaker()
{
    fName = "STDDriftElectronMaker";
}


// 히트 개수가 작은 애들이 압도적으로 많을 것이다. 0개 주변으로 많을 거고, 그 다음 12개 정도의 히트 개수를 가진 놈들. 분포를 봐라. 12개 정도에서 가우시안 분포가 만들어 질거다. 얘네들의 HT Efficiency를 1차적으로 보아라.

/*
bool STDDriftElectronMaker::BindInputBranches()
{
    auto* inputTree = fRun ? fRun->GetInputChain() : nullptr;
    if (inputTree == nullptr) {
        cout << "[STDDriftElectronMaker::BindInputBranches] Error: input tree is null!" << endl;
        return false;
    }

    fTrackArray = nullptr;
    fStepArray = nullptr;

    inputTree->SetBranchStatus("MCTrack", 1);
    inputTree->SetBranchStatus("MCStepTPCDrum", 1);

    int rc1 = inputTree->SetBranchAddress("MCTrack", &fTrackArray);
    int rc2 = inputTree->SetBranchAddress("MCStepTPCDrum", &fStepArray);

    if (rc1 < 0 || rc2 < 0) {
        cout << "[STDDriftElectronMaker::BindInputBranches] Error while binding input branches." << endl;
        cout << "  SetBranchAddress(MCTrack)       = " << rc1 << endl;
        cout << "  SetBranchAddress(MCStepTPCDrum) = " << rc2 << endl;
        return false;
    }

    fInputBranchesBound = true;
    return true;
}
*/
bool STDDriftElectronMaker::Init()
{
    fDetector = (TPCDrum*) fRun->GetDetector();
    if (fDetector == nullptr) {
        cout << "[STDDriftElectronMaker::Init] Error: fDetector is null!" << endl;
        return false;
    }

    fPadPlane = (STDPadPlane*) fDetector->GetDetectorPlane();
    if (fPadPlane == nullptr) {
        cout << "[STDDriftElectronMaker::Init] Error: fPadPlane is null!" << endl;
        return false;
    }

    fChannelArray = fRun->RegisterBranchA("RawPad", "GETChannel");
    fMCTagArray = fRun->RegisterBranchA("MCTag", "LKMCTag");
    fTrackArray = fRun -> KeepBranchA("MCTrack");
    fStepArray = fRun -> KeepBranchA("MCStepTPCDrum");
   /* if (!BindInputBranches()) {
        cout << "[STDDriftElectronMaker::Init] Error: failed to bind input branches directly." << endl;
        return false;
    }
*/
    if (fChannelArray == nullptr || fMCTagArray == nullptr) {
        cout << "[STDDriftElectronMaker::Init] Error: one or more output branches are null!" << endl;
        cout << "  fChannelArray        = " << fChannelArray << endl;
        cout << "  fMCTagArray          = " << fMCTagArray << endl;
        return false;
    }

    fTBTime = 10.; // [ns]
    if (fPar->CheckPar("TPCDrum/TBTime")) {
        fTBTime = fPar->GetParDouble("TPCDrum/TBTime");
    }

    // Runtime gating-grid switch can be controlled in three ways:
    // 1) default = true
    // 2) user macro: drift->SetUseGatingGrid(false);
    // 3) parameter file:
    //      STDDriftElectronMaker/UseGatingGrid false
    //    or
    //      TPCDrum/UseGatingGrid false
    if (fPar->CheckPar("STDDriftElectronMaker/UseGatingGrid")) {
        fUseGatingGrid = fPar->GetParBool("STDDriftElectronMaker/UseGatingGrid");
    }
    else if (fPar->CheckPar("TPCDrum/UseGatingGrid")) {
        fUseGatingGrid = fPar->GetParBool("TPCDrum/UseGatingGrid");
    }

    cout << "[STDDriftElectronMaker::Init] UseGatingGrid = "
         << (fUseGatingGrid ? "true" : "false") << endl;

    fRandom = new TRandom3(0);
    fRandom->SetSeed(time(0));

    fTuneManager = STDSimTuningManager::GetSimTuningManager();
    fFieldDistortion = new STDFieldDistortionMaker();
    fGatingGrid = new STDGatingGridResponse();

    if (fTuneManager == nullptr || fFieldDistortion == nullptr || fGatingGrid == nullptr) {
        cout << "[STDDriftElectronMaker::Init] Error: tuning/field/gating object is null!" << endl;
        return false;
    }

    fFieldDistortion->Init();
    fGatingGrid->Init();

    fTuneManager->SetFieldDostortionMaker(fFieldDistortion);
    fElectronStepSize = fTuneManager->GetElectronStepSize();

    if (!std::isfinite(fElectronStepSize) || fElectronStepSize <= 0) {
        cout << "[STDDriftElectronMaker::Init] Warning: invalid fElectronStepSize = "
             << fElectronStepSize << ", force set to 1.0 mm" << endl;
        fElectronStepSize = 1.0;
    }


    fGEMGainScale = 1.0;
    if (fPar -> CheckPar("TPCDrum/GEMGainScale")){
	fGEMGainScale = fPar -> GetParDouble("TPCDrum/GEMGainScale");

    }



    return true;
}

void STDDriftElectronMaker::Exec(Option_t* option)
{
/*    if (!fInputBranchesBound) {
        if (!BindInputBranches()) {
            cout << "[STDDriftElectronMaker::Exec] failed to re-bind input branches." << endl;
            return;
        }
    }
*/
    int tracks = fTrackArray ? fTrackArray->GetEntriesFast() : -1;
    int stepnum = fStepArray ? fStepArray->GetEntriesFast() : -1;

     

    if (tracks < 0 || tracks > 100000) {
        cout << "[STDDriftElectronMaker::Exec] invalid trackNum = " << tracks
             << " at event " << fRun->GetCurrentEventID() << endl;
        return;
    }

    

    if (stepnum < 0 || stepnum > 10000000) {
        cout << "[STDDriftElectronMaker::Exec] invalid stepNum = " << stepnum
             << " at event " << fRun->GetCurrentEventID() << endl;
        return;
    }

    if (fChannelArray == nullptr || fMCTagArray == nullptr ||
        fTrackArray == nullptr || fStepArray == nullptr ||
        fDetector == nullptr || fPadPlane == nullptr) {
        cout << "[STDDriftElectronMaker::Exec] critical null pointer detected. Skip event." << endl;
        return;
    }

    fChannelArray->Clear("C");
    fMCTagArray->Clear("C");

    lk_info << "Event : " << fRun->GetCurrentEventID() << endl;
  //  lk_info << " nTrack : " << tracks << endl;

    int stepNum = fStepArray->GetEntriesFast();

    for (int i = 0; i < stepNum; i++) {
        fStep = (LKMCStep*) fStepArray->At(i);
        if (fStep == nullptr) {
            cout << "[STDDriftElectronMaker::Exec] null step at i = " << i << endl;
            continue;
        }

        ConvertCoordinateGeantToPad();

        double sx = fStep->GetX();
        double sy = fStep->GetY();
        double sz = fStep->GetZ();

        if (!std::isfinite(sx) || !std::isfinite(sy) || !std::isfinite(sz)) {
            cout << "[STDDriftElectronMaker::Exec] non-finite step coordinate at i = " << i
                 << " : (" << sx << ", " << sy << ", " << sz << ")" << endl;
            continue;
        }

        bool isIn = fDetector->IsInBoundary(sx, sy, sz);
        if (!isIn) continue;

        double StartX = sx;
        double StartY = sy;
        double StartZ = sz;
        double StartT = fStep->GetTime();
        int trackId = fStep->GetTrackID();

        if (!std::isfinite(StartT)) {
            cout << "[STDDriftElectronMaker::Exec] non-finite time at step i = " << i << endl;
            continue;
        }


        // HT quality
        // Hit Cut Problem
        // adc가 1 이상인 패드 개수 분포 

        int clusterNum = (GetElectronClusterNum() / 5) + 1;
        if (clusterNum > 20) clusterNum = 20;
        if (clusterNum < 1) continue;

/*        if (clusterNum > 10000) {
            cout << "[STDDriftElectronMaker::Exec] suspiciously large clusterNum = "
                 << clusterNum << " at step i = " << i
                 << ", Edep = " << fStep->GetEdep() << endl;
            clusterNum = 10000;
        }
*/
        for (int e = 0; e < clusterNum; e++) {
            double x = StartX;
            double y = StartY;
            double z = StartZ;
            double t = StartT;
            double w = 5.0;
            
            bool driftFlag = DriftElectron(x, y, z, t, w);
            if (!driftFlag) continue;

            bool avalancheFlag = AvalancheElectron(x, y, z, t, w);
            if (!avalancheFlag) continue;

            if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
                !std::isfinite(t) || !std::isfinite(w)) {
                cout << "[STDDriftElectronMaker::Exec] non-finite post-drift/avalanche state"
                     << " at step i = " << i << endl;
                continue;
            }

            int padID = fPadPlane->FindPadID(x, y);
            if (padID < 0) continue;

            unsigned int tb = (unsigned int)(t / fTBTime);
            if (tb >= 512) continue;

            fChannel = (GETChannel*) fPadPlane->GetChannelFast(padID);
            if (fChannel == nullptr) {
                cout << "[STDDriftElectronMaker::Exec] null channel for padID = " << padID
                     << " at step i = " << i
                     << " (x,y,z)=(" << x << ", " << y << ", " << z << ")" << endl;
                continue;
            }

            fMCTag = (LKMCTag*) fPadPlane->GetMCTag(padID);
            if (fMCTag == nullptr) {
                cout << "[STDDriftElectronMaker::Exec] null MCTag for padID = " << padID
                     << " at step i = " << i << endl;
                continue;
            }

            fChannel->SetAsad(fPadPlane->GetAsAdID(padID));
            fChannel->SetAget(fPadPlane->GetAgetID(padID));
            fChannel->SetChan(fPadPlane->GetChanID(padID));
            fChannel->SetPadID(padID);
            fChannel->GetBufferArray()[tb] += w;

            fMCTag->AddMCTag(trackId, tb);
        }
    }

    TIter itChannel(fPadPlane->GetChannelArray());
    int idx = 0;
    while ((fChannel = (GETChannel*) itChannel.Next())) {
        if (fChannel == nullptr) {
            idx++;
            continue;
        }

        fMCTag = (LKMCTag*) fPadPlane->GetMCTag(idx);
        if (fMCTag == nullptr) {
            cout << "[STDDriftElectronMaker::Exec] null MCTag in save loop, idx = "
                 << idx << endl;
            idx++;
            continue;
        }

        GETChannel* outChannel = (GETChannel*) fChannelArray->ConstructedAt(idx);
        LKMCTag* outTag = (LKMCTag*) fMCTagArray->ConstructedAt(idx);

        if (outChannel == nullptr || outTag == nullptr) {
            cout << "[STDDriftElectronMaker::Exec] failed to construct output channel/tag at idx = "
                 << idx << endl;
            idx++;
            continue;
        }

        fChannel->Copy(*outChannel);
        fMCTag->Copy(*outTag);

        fChannel->Clear();
        fMCTag->Clear();

        idx++;
    }
}

bool STDDriftElectronMaker::EndOfRun()
{
    return true;
}

void STDDriftElectronMaker::ConvertCoordinateGeantToPad()
{
    if (fStep == nullptr) return;
    if (fDetector == nullptr) return;

    double x = fStep->GetX();
    double y = fStep->GetY();
    double z = fStep->GetZ();

    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
        return;
    }

    fDetector->GetCoordinateGeantToPad(x, y, z);

    fStep->SetX(x);
    fStep->SetY(y);
    fStep->SetZ(z);
}

int STDDriftElectronMaker::GetElectronClusterNum()
{
    if (fStep == nullptr) return 0;
    if (fTuneManager == nullptr) return 0;

    double e = fStep->GetEdep() * 1000000.; // [eV]
    double w = fTuneManager->GetWValue();

    if (!std::isfinite(e) || !std::isfinite(w) || w <= 0) {
        cout << "[STDDriftElectronMaker::GetElectronClusterNum] invalid e or w : "
             << "e = " << e << ", w = " << w << endl;
        return 0;
    }

    int clusterSize = int(e / w);

    if (clusterSize < 0) clusterSize = 0;
    if (clusterSize > 50000) {
        cout << "[STDDriftElectronMaker::GetElectronClusterNum] too large clusterSize = "
             << clusterSize << ", truncated to 50000" << endl;
        clusterSize = 50000;
    }

    return clusterSize;
}

bool STDDriftElectronMaker::DriftElectron(double& x, double& y, double& z, double& t, double& w)
{
    const double GEMSurfaceHeight = 3. * fDetector->fGEMSpacing; // [mm]

    
    const int maxIter = 100;
    int iter = 0;

    while (1) {
        ++iter;
/*        if (iter > maxIter) {
            cout << "[STDDriftElectronMaker::DriftElectron] maxIter reached!"
                 << " x=" << x
                 << " y=" << y
                 << " z=" << z
                 << " t=" << t
                 << " w=" << w
                 << endl;
            return false;
        }

        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
            !std::isfinite(t) || !std::isfinite(w)) {
            cout << "[STDDriftElectronMaker::DriftElectron] non-finite input state!" << endl;
            return false;
        }
*/
        if (z <= GEMSurfaceHeight) {
            return true;
        }

        double GatingGridFactor = 1.0;
        if (fUseGatingGrid) {
            GatingGridFactor = fGatingGrid->GetGatingGridFactor(x, y, z);
            if (!std::isfinite(GatingGridFactor)) {
                cout << "[STDDriftElectronMaker::DriftElectron] invalid GatingGridFactor!" << endl;
                return false;
            }
            if (GatingGridFactor < 0) {
                return false;
            }
        }

        double dirX, dirY, dirZ;
        fFieldDistortion->GetElectronDirection(x, y, z, dirX, dirY, dirZ);

        if (!std::isfinite(dirX) || !std::isfinite(dirY) || !std::isfinite(dirZ)) {
            cout << "[STDDriftElectronMaker::DriftElectron] invalid field direction!"
                 << " dir=(" << dirX << ", " << dirY << ", " << dirZ << ")"
                 << " at (x,y,z)=(" << x << ", " << y << ", " << z << ")" << endl;
            return false;
        }

        fElectronUnitVec.SetXYZ(dirX, dirY, dirZ);
        if (fElectronUnitVec.Mag() <= 0) {
            cout << "[STDDriftElectronMaker::DriftElectron] zero electron direction vector!" << endl;
            return false;
        }

        double sigmaT = fTuneManager->GetDiffusionT(x, y, z) * sqrt(fElectronStepSize);
        double sigmaL = fTuneManager->GetDiffusionL(x, y, z) * sqrt(fElectronStepSize);
        double velocityD = fTuneManager->GetDriftVelocity(x, y, z);

        if (!std::isfinite(sigmaT) || !std::isfinite(sigmaL) ||
            !std::isfinite(velocityD) || velocityD <= 0) {
            cout << "[STDDriftElectronMaker::DriftElectron] invalid diffusion/velocity!"
                 << " sigmaT=" << sigmaT
                 << " sigmaL=" << sigmaL
                 << " velocityD=" << velocityD << endl;
            return false;
        }

        double dr  = fRandom->Gaus(0., sigmaT);
        double dt  = fRandom->Gaus(0., sigmaL) / velocityD;
        double phi = fRandom->Uniform(2 * TMath::Pi());

        fOthogonalUnitVec = fElectronUnitVec.Orthogonal();
        if (fOthogonalUnitVec.Mag() > 0) {
            fOthogonalUnitVec.Rotate(phi, fElectronUnitVec);
            fOthogonalUnitVec.SetMag(dr);
        }
        else {
            fOthogonalUnitVec.SetXYZ(0., 0., 0.);
        }

        fElectronUnitVec.SetMag(fElectronStepSize);

        x = x + fElectronUnitVec.X() + fOthogonalUnitVec.X();
        y = y + fElectronUnitVec.Y() + fOthogonalUnitVec.Y();
        z = z + fElectronUnitVec.Z() + fOthogonalUnitVec.Z();
        t = t + fElectronStepSize / velocityD + dt;

        if (fUseGatingGrid) {
            w = (w < GatingGridFactor * w) ? w : GatingGridFactor * w;
        }
    }

    return false;
}

bool STDDriftElectronMaker::AvalancheElectron(double& x, double& y, double& z, double& t, double& w)
{
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
        !std::isfinite(t) || !std::isfinite(w)) {
        return false;
    }

    double extraGainFactor = 1.;
    if (-10. <= x && x <= 10.) {
        extraGainFactor = 0.5;
    }

    double zz = z;
    if (zz < 0) zz = 0;

    //double gain = fTuneManager->GetGEMGainFactor(x, y, z) * extraGainFactor;
    
    double gain = fTuneManager->GetGEMGainFactor(x,y,z) * extraGainFactor * fGEMGainScale;
    double sigmaT = fTuneManager->GetGEMDiffusionT() * sqrt(zz); // [mm]
    double sigmaL = fTuneManager->GetGEMDiffusionL() * sqrt(zz); // [mm]
    double velocityD = fTuneManager->GetGEMDriftVelocity(); // [mm/ns]

    if (!std::isfinite(gain) || !std::isfinite(sigmaT) ||
        !std::isfinite(sigmaL) || !std::isfinite(velocityD) || velocityD <= 0) {
        cout << "[STDDriftElectronMaker::AvalancheElectron] invalid GEM parameters!"
             << " gain=" << gain
             << " sigmaT=" << sigmaT
             << " sigmaL=" << sigmaL
             << " velocityD=" << velocityD << endl;
        return false;
    }

    double dr  = fRandom->Gaus(0., sigmaT);
    double dt  = fRandom->Gaus(0., sigmaL) / velocityD;
    double phi = fRandom->Uniform(2 * TMath::Pi());

    double dx = dr * cos(phi);
    double dy = dr * sin(phi);

    x = x + dx;
    y = y + dy;
    z = 0.;
    t = t + dt;
    w = w * gain;

    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(t) || !std::isfinite(w)) {
        return false;
    }

    if (w < 0) return false;
    return true;
}
