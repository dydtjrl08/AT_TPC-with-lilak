#include "STDTest2.h"

#include <vector>
#include <tuple>

ClassImp(STDTest2);

STDTest2::STDTest2()
{

    fName = "STDTest2";
}

bool STDTest2::Init()
{
    fDetector = (TPCDrum *) fRun -> GetDetector();
    fPadPlane = (STDPadPlane*) fDetector -> GetDetectorPlane();
    fTrackArray = fRun -> KeepBranchA("MCTrack");
    for(int i=0; i<siDetNum; i++){
        fSiStepArray[i] = fRun -> KeepBranchA(Form("MCStep%s", fDetector->fSiDetectorName[i].Data()));
    }

    TString name = fRun -> GetOutputFileName();
    file = new TFile(name, "recreate");
    tree = new TTree("event", "event");
    tree -> Branch("trkNum", &outTrkNum, "TrkNum/I");
    tree -> Branch("vertex", outVertex, "vertex[2][3]/D");
    tree -> Branch("TPCdedx", outTPCDedx, "TPCdedx[10]/D");
    tree -> Branch("trk", outTrk, "trk[10][4]/D");
    tree -> Branch("truthTrk", outTruthTrk, "truthTrk[10][5]/D");

    return true;
}

void STDTest2::Exec(Option_t *option)
{
    outTrkNum = 0;
    memset(outVertex, 0., sizeof(outVertex));
    memset(outTPCDedx, 0., sizeof(outTPCDedx));
    memset(outTrk, 0., sizeof(outTrk));
    memset(outTruthTrk, 0., sizeof(outTruthTrk));

    // double siESum[siDetNum][5];
    // memset(siESum, 0., sizeof(siESum));

    // for(int si=0; si<siDetNum; si++){
    //     vector<int> tmpTrackId;
    //     int stepNum = fSiStepArray[si] -> GetEntries();
    //     for(int step=0; step<stepNum; step++){
    //         fStep = (LKMCStep*)fSiStepArray[si] -> UncheckedAt(step);

    //         int trackId = fStep -> GetTrackID();
    //         double x = fStep -> GetX();
    //         double y = fStep -> GetY();
    //         double z = fStep -> GetZ();
    //         double t = fStep -> GetTime();
    //         double e = fStep -> GetEdep();
    //         fDetector -> GetCoordinateGeantToPad(x, y, z);

    //         fMCTrack  = (LKMCTrack*)fTrackArray -> UncheckedAt(trackId-1);
    //         double startE = fMCTrack->GetEnergy();
    //         int pid = fMCTrack -> GetPDG();
    //         int pidIdx = -1;
    //         if(pid == id_ne20){pidIdx = kNe20;}
    //         if(pid == id_c12 || pid == id_c12_decay){pidIdx = kC12;}
    //         if(pid == id_alpha){pidIdx = kAlpha;}
    //         if(pid == id_B8){pidIdx = kB8;}
    //         if(pidIdx == -1){continue;}

    //         siESum[si][pidIdx] += e;
    //         siESum[si][4] += e;
    //     }
    // }
    // // =================================== trigger  =======================================
    // const double threshold[4] = {0.5, 12., 40., 70.};
    // int estiTrkNum = 0;
    // bool isBeamLike = false;
    // for(int si=0; si<siDetNum; si++){
    //     if(siESum[si][4] > threshold[3]){isBeamLike = true;}
    //     int siTrk = 0;
    //     for(int th=0; th<3; th++){
    //         if(siESum[si][4] > threshold[th]){
    //             siTrk = th+1;
    //         }
    //     }
    //     estiTrkNum += siTrk;
    // }
    // if(estiTrkNum < 3){return;}


    int trackNum = fTrackArray -> GetEntries();

    int tmpTrkNum = 0;
    for(int trk=0; trk<trackNum; trk++){
        fMCTrack = (LKMCTrack*)fTrackArray -> UncheckedAt(trk);
        int pdg = fMCTrack -> GetPDG();
        int pidIdx = -1;
        if(pdg == id_ne20){pidIdx = kNe20;}
        if(pdg == id_c12){pidIdx = kC12;}
        if(pdg == id_c12_decay){pidIdx = kC12;}
        if(pdg == id_B8){pidIdx = kB8;}
        if(pdg == id_alpha){pidIdx = kAlpha;}
        // if(pidIdx == kNe20 || pidIdx == kC12 || pidIdx == kB8){continue;}
        if(pidIdx != kC12){continue;}

        double vx = fMCTrack -> GetVX();
        double vy = fMCTrack -> GetVY();
        double vz = fMCTrack -> GetVZ();

        fDetector -> GetCoordinateGeantToPad(vx, vy, vz);
        if(-50. >= vy || vy >= 80.){continue;}

        double e = fMCTrack -> GetEnergy();
        double px = fMCTrack -> GetPX();
        double py = fMCTrack -> GetPY();
        double pz = fMCTrack -> GetPZ();

        outVertex[1][0] = vx;
        outVertex[1][1] = vy;
        outVertex[1][2] = vz;

        outTruthTrk[tmpTrkNum][0] = px;
        outTruthTrk[tmpTrkNum][1] = py;
        outTruthTrk[tmpTrkNum][2] = pz;
        outTruthTrk[tmpTrkNum][3] = e;
        outTruthTrk[tmpTrkNum][4] = pidIdx;

        cout << px << " " << py << " " << pz << " " << e << " " << pidIdx << endl;

        tmpTrkNum++;
    }
    outTrkNum = tmpTrkNum;

    if(outTrkNum == 2){
        tree -> Fill();
    }
}

bool STDTest2::EndOfRun()
{
    file -> cd();
    tree -> Write();
    file -> Close();
    return true;
}