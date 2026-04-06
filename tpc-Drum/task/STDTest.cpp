#include "STDTest.h"

#include <vector>
#include <tuple>

ClassImp(STDTest);

STDTest::STDTest()
{
    fEventFigSave = true;
    fSmaredData = true;

    fName = "STDTest";
}

bool STDTest::Init()
{
    fDetector = (TPCDrum *) fRun -> GetDetector();
    fPadPlane = (STDPadPlane*) fDetector -> GetDetectorPlane();
    fTrackArray = fRun -> KeepBranchA("MCTrack");
    fStepArray = fRun -> KeepBranchA("MCStepTPCDrum");

    double x[4] = {-100., -100., 100., 100.};
    double y[4] = {-34., 166., 166., -34.};

    hBoundaryXY = new TH2Poly();
    hBoundaryXY -> SetStats(0);
    hBoundaryXY -> SetTitle("; x [mm]; y [mm]");
    hBoundaryXY -> AddBin(4, x, y);

    double z[4] = {0., 0., 160., 160.};
    hBoundaryYZ = new TH2Poly();
    hBoundaryYZ -> SetStats(0);
    hBoundaryYZ -> SetTitle("; y [mm]; z [mm]");
    hBoundaryYZ -> AddBin(4, y, z);

    fPadPoly = fPadPlane -> GetPadPlanePoly();
    
    fDynamicRange = 10000.; // [fC]
    fEChargeToADC = fElectronCharge/(fDynamicRange *1.0e-15)*fADCMaxAmp;

    fRandom = new TRandom3(0);
    //tpc track
    int trackColorArr[10] = {1, 2, 4, 6, 8, 9, 51, 94, 90, 13};
    for(int i=0; i<10; i++){
        fTrack[i] = new TGraph();
        fTrackTime[i] = new TGraph();

        fFitTrk[i] = new TF1(Form("fitTrk_%i", i), "[0]+[1]*x", -100., 100.);
        fFitTrk[i] -> SetLineColor(trackColorArr[i]);
        fFitTrkTime[i] = new TF1(Form("fitTrkTime_%i", i), "[0]+[1]*x", -30., 200.);
        fFitTrkTime[i] -> SetLineColor(trackColorArr[i]);
    }

    cTrk = new TCanvas("cTrk", "", 1200., 1200.);

    fTuneManager = STDSimTuningManager::GetSimTuningManager();
    fFieldDistortion = new STDFieldDistortionMaker();
    fGatingGrid = new STDGatingGridResponse();

    fFieldDistortion -> Init();
    fGatingGrid -> Init();

    fTuneManager -> SetFieldDostortionMaker(fFieldDistortion);


    for(int i=0; i<siDetNum; i++){
        fSiStepArray[i] = fRun -> KeepBranchA(Form("MCStep%s", fDetector->fSiDetectorName[i].Data()));
    }

    int colorIdx[5] = {2, 4, 6, 8, 1};
    // =======================================

    TString pidName[5] = {"Ne20", "C12", "B8", "#alpha", "all"};
    for(int i=0; i<siDetNum; i++){
        for(int j=0; j<5; j++){
            hSiEnergy[i][j] = new TH1D(Form("siE_%i_%i", i, j), "", 50., 0., 100.);
            hSiEnergy[i][j] -> SetStats(0);
            hSiEnergy[i][j] -> SetLineColor(colorIdx[j]);
            hSiEnergy[i][j] -> SetTitle(Form("%s Deposit energy; energy [MeV]; Counts", fDetector->fSiDetectorName[i].Data()));

            for(int k=0; k<4; k++){
                hSiEnergyByNum[i][j][k] = new TH1D(Form("siE_trk_%i_%i_%i", i, j, k), "", 50., 0., 100.);
                hSiEnergyByNum[i][j][k] -> SetStats(0);
                hSiEnergyByNum[i][j][k] -> SetLineColor(colorIdx[k]);
                if(k == 3){hSiEnergyByNum[i][j][k] -> SetLineColor(1);}
                hSiEnergyByNum[i][j][k] -> SetTitle(Form("%s Deposit energy %s; energy [MeV]; Counts", fDetector->fSiDetectorName[i].Data(), pidName[j].Data()));
            }
        }
    }

    for(int i=0; i<4; i++){
        hPlane[i] = new TH2D(Form("plane_%i", i), "", 100, -120., 120., 100, 0., 250.);
        hPlane[i] -> SetStats(0);

        hPlaneAlngpe[i] = new TH2D(Form("hPlaneAlngpe%i", i), "", 70, -45., 45., 70, 45., 45.);
        hPlaneAlngpe[i] -> SetStats(0);

        hTrkEnergy[i][0] = new TH1D(Form("trkEnergyStart_%i", i), "", 100, 0., 150.);
        hTrkEnergy[i][1] = new TH1D(Form("trkEnergyEnd_%i", i), "", 100, 0., 150.);
        hTrkEnergy[i][0] -> SetStats(0);
        hTrkEnergy[i][1] -> SetStats(0);

        if(i == kNe20){
            hPlaneAlngpe[i] -> SetTitle("Ne20 angle; #theta_{x} [degree]; #theta_{y} [degree]");
            hPlane[i] -> SetTitle("Ne20 position at y=251 mm plane; x [mm]; z [mm]");
            hTrkEnergy[i][0] -> SetTitle("Ne20 energy; Energy [MeV]; Counts");
            hTrkEnergy[i][1] -> SetTitle("Ne20 energy at Si; Energy [MeV]; Counts");
        }
        if(i == kC12){
            hPlane[i] -> SetTitle("C12 position at y=251 mm plane; x [mm]; z [mm]");
            hTrkEnergy[i][0] -> SetTitle("C12 energy; Energy [MeV]; Counts");
            hTrkEnergy[i][1] -> SetTitle("C12 energy at Si;  Energy [MeV]; Counts");
        }
        if(i == kAlpha){
            hPlane[i] -> SetTitle("#alpha position aat y=251 mm plane; x [mm]; z [mm]");
            hTrkEnergy[i][0] -> SetTitle("#alpha energy; Energy [MeV]; Counts");
            hTrkEnergy[i][1] -> SetTitle("#alpha energy at Si;  Energy [MeV]; Counts");
        }
    }

    hVertex = new TH1D("vertexDiff","",100., 0., 50.);
    hVertex -> SetTitle("Difference vertex; #DeltaR_{vtx} [mm]; Counts");
    hVertex -> SetStats(0);

    hDedx[0] = new TH2D("dedx_reco", "", 70., 5., 40., 70, 1., 80.);
    hDedx[1] = new TH2D("dedx_reco2", "", 70., 5., 40., 70, 1., 80.);
    hDedx[0] -> SetTitle("TPC dEdx; E [MeV]; dE/dx [KeV/mm]");
    hDedx[1] -> SetTitle("TPC dEdx; E [MeV]; dE/dx [KeV/mm]");
    hDedx[0] -> SetStats(0);
    hDedx[1] -> SetStats(0);

    hECorr[0] = new TH2D("EECorr", "", 70., 0., 40., 70, 0., 100.);
    hECorr[1] = new TH2D("EECorr2", "", 70., 0., 40., 70, 0., 40.);
    hECorr[0] -> SetTitle(";Si Energy [MeV]; Generate Energy [MeV]");
    hECorr[1] -> SetTitle(";Si Energy [MeV]; Generate Energy [MeV]");
    hECorr[0] -> SetStats(0);
    hECorr[1] -> SetStats(0);

    TString name = fRun -> GetOutputFileName();
    file = new TFile(name, "recreate");
    tree = new TTree("event", "event");
    tree -> Branch("trkNum", &outTrkNum, "TrkNum/I");
    tree -> Branch("vertex", outVertex, "vertex[2][3]/D");
    tree -> Branch("TPCdedx", outTPCDedx, "TPCdedx[10]/D");
    tree -> Branch("trk", outTrk, "trk[10][4]/D");
    tree -> Branch("truthTrk", outTruthTrk, "truthTrk[10][5]/D");

    hMomemtum = new TH1D("hMomemtum", "", 100, -1., 1.);

    return true;
}

void STDTest::Exec(Option_t *option)
{
    fRandom -> SetSeed(0);

    int trackNum = fTrackArray -> GetEntries();

    double trackEdep[4][20][2];
    memset(trackEdep, 0., sizeof(trackEdep));

    double siESum[siDetNum][5];
    double siTrkNum[siDetNum][5];
    memset(siTrkNum, 0., sizeof(siTrkNum));
    memset(siESum, 0., sizeof(siESum));

    // ============ Si det data ==============
    for(int si=0; si<4; si++){
        fSiInfo[si].clear();
    }
    
    for(int si=0; si<siDetNum; si++){
        vector<int> tmpTrackId;
        int stepNum = fSiStepArray[si] -> GetEntries();
        for(int step=0; step<stepNum; step++){
            fStep = (LKMCStep*)fSiStepArray[si] -> UncheckedAt(step);

            int trackId = fStep -> GetTrackID();
            double x = fStep -> GetX();
            double y = fStep -> GetY();
            double z = fStep -> GetZ();
            double t = fStep -> GetTime();
            double e = fStep -> GetEdep();
            fDetector -> GetCoordinateGeantToPad(x, y, z);

            fMCTrack  = (LKMCTrack*)fTrackArray -> UncheckedAt(trackId-1);
            double startE = fMCTrack->GetEnergy();
            int pid = fMCTrack -> GetPDG();
            int pidIdx = -1;
            if(pid == id_ne20){pidIdx = kNe20;}
            if(pid == id_c12 || pid == id_c12_decay){pidIdx = kC12;}
            if(pid == id_alpha){pidIdx = kAlpha;}
            if(pid == id_B8){pidIdx = kB8;}
            if(pidIdx == -1){continue;}

            trackEdep[pidIdx][trackId-1][0] = startE;
            trackEdep[pidIdx][trackId-1][2] += e;

            siESum[si][pidIdx] += e;
            siESum[si][4] += e;

            bool isAlreadyFill = false;
            for(int i=0; i<tmpTrackId.size(); i++){
                if(trackId == tmpTrackId[i]){
                    isAlreadyFill = true;
                    break;
                }
            }
            if(isAlreadyFill){
                for(int i=0; i<fSiInfo[3].size(); i++){
                    if(fSiInfo[3][i] == trackId-1){
                        fSiInfo[2][i] += e;
                    }
                }
                continue;
            }

            tmpTrackId.push_back(trackId);
            siTrkNum[si][pidIdx] += 1;
            siTrkNum[si][4] += 1;


            double posX = x;
            double posZ = z;
            // if(fSmaredData){
            //     posX = fRandom -> Gaus(posX, 1.);
            //     posZ = fRandom -> Gaus(posZ, 1.);
            // }
            fSiInfo[0].push_back(posX);
            fSiInfo[1].push_back(posZ);
            fSiInfo[2].push_back(e);
            fSiInfo[3].push_back(trackId-1);
        }
    }

    if(fEventFigSave){
        for(int i=0; i<4; i++){
            for(int trk=0; trk<20; trk++){
                double startE = trackEdep[i][trk][0];
                double edep = trackEdep[i][trk][1];
                if(startE < 0.1){continue;}
                hTrkEnergy[i][0] -> Fill(startE);

                if(edep < 0.001){continue;}
                hTrkEnergy[i][1] -> Fill(edep);
            }
        }
    }

    // =================================== trigger  =======================================
    const double threshold[4] = {0.5, 12., 40., 70.};
    int estiTrkNum = 0;
    bool isBeamLike = false;
    for(int si=0; si<siDetNum; si++){
        if(siESum[si][4] > threshold[3]){isBeamLike = true;}
        int siTrk = 0;
        for(int th=0; th<3; th++){
            if(siESum[si][4] > threshold[th]){
                siTrk = th+1;
            }
        }
        estiTrkNum += siTrk;
    }

    if(estiTrkNum < 3){return;}

    if(fEventFigSave){
        for(int trk=0; trk<trackNum; trk++){
            fMCTrack = (LKMCTrack*)fTrackArray -> UncheckedAt(trk);
            int pid = fMCTrack -> GetPDG();
            int pidIdx = -1;
            if(pid == id_ne20){pidIdx = kNe20;}
            if(pid == id_c12){pidIdx = kC12;}
            if(pid == id_c12_decay){pidIdx = kC12;}
            if(pid == id_c12_decay){pidIdx = kC12;}
            if(pid == id_alpha){pidIdx = kAlpha;}

            double px = fMCTrack->GetPX();
            double py = fMCTrack->GetPY();
            double pz = fMCTrack->GetPZ();
            double e = fMCTrack->GetEnergy();

            double thetaX = atan(px/pz)* 180/TMath::Pi();
            double thetaY = atan(py/pz)* 180/TMath::Pi();
            hPlaneAlngpe[pidIdx] -> Fill(thetaX, thetaY);
        }
        for(int si=0; si<siDetNum; si++)
        {
            for(int i=0; i<4; i++){
                if(siESum[si][i] < 0.01){continue;}
                hSiEnergy[si][i] -> Fill(siESum[si][i]);
                
                int trkNumIdx = (siTrkNum[si][i] > 3 || siTrkNum[si][i] == 0)? -1 : siTrkNum[si][i]-1;
                if(trkNumIdx < 0){continue;}
                hSiEnergyByNum[si][i][trkNumIdx] -> Fill(siESum[si][i]);
                hSiEnergyByNum[si][i][3] -> Fill(siESum[si][i]);
            }
            if(siESum[si][4] < 0.01){continue;}
            hSiEnergy[si][4] -> Fill(siESum[si][4]);

            int trkNumIdx = (siTrkNum[si][4] > 3 || siTrkNum[si][4] == 0)? -1 : siTrkNum[si][4]-1;
            if(trkNumIdx < 0){continue;}
            hSiEnergyByNum[si][4][trkNumIdx] -> Fill(siESum[si][4]);
            hSiEnergyByNum[si][4][3] -> Fill(siESum[si][4]);
        }
    }

    // =============================== TPC ========================================
    memset(tpcPad, 0., sizeof(tpcPad));
    bool isTPCTrack[10];
    memset(isTPCTrack, 0., sizeof(isTPCTrack));
    for(int i=0; i<10; i++){
        fTrack[i] -> Set(0);
        fTrackTime[i] -> Set(0);
    }

    int stepNumTPC = fStepArray -> GetEntries();
    for(int step=0; step<stepNumTPC; step++){
        fStep = (LKMCStep*)fStepArray -> UncheckedAt(step);

        int trackId = fStep -> GetTrackID();
        double x = fStep -> GetX();
        double y = fStep -> GetY();
        double z = fStep -> GetZ();
        double t = fStep -> GetTime();
        double e = fStep -> GetEdep();
        fDetector -> GetCoordinateGeantToPad(x, y, z);

        // Checking for TPC-Drum active area
        bool isIn = fDetector -> IsInBoundary(x, y, z);
        if(!isIn){continue;}

        double velocityD = fTuneManager -> GetDriftVelocity(x, y, z); // [mm/ns]
        t = z/velocityD;

        if(fSmaredData){
            double sigmaT = fTuneManager ->GetDiffusionT(x, y, z) * sqrt(z); // [mm]
            double sigmaL = fTuneManager -> GetDiffusionL(x, y, z) * sqrt(z); // [mm]

            double dr = fRandom -> Gaus(0., sigmaT);
            double dt = fRandom -> Gaus(0, sigmaL)/velocityD;
            double phi = fRandom -> Uniform(2*TMath::Pi());

            double dx = dr * cos(phi);
            double dy = dr * sin(phi);

            x = x + dx;
            y = y + dy;
            t = t + dt;
        }

        int padID = fPadPlane -> FindPadID(x, y);
        if(padID < 0){continue;}
        int layer = fPadPlane -> GetLayerID(padID);
        int row = fPadPlane -> GetRowID(padID);
        
        tpcPad[trackId-1][layer][row][0] += (e*1000.); // [KeV]
        tpcPad[trackId-1][layer][row][1] += 1.;
        tpcPad[trackId-1][layer][row][2] += t;
        isTPCTrack[trackId-1] = true;
    }

    // if(fEventFigSave){
    //     cTrk -> Clear();
    //     cTrk -> Divide(2,2);
    //     cTrk -> cd(1);
    //     hBoundaryXY -> Draw();
    //     fPadPoly -> Draw("same");

    //     cTrk -> cd(2);
    //     hBoundaryYZ -> Draw();
    // }
    
    double tpcTrkESum[10][2];
    bool isValidTrackReco[10];
    memset(isValidTrackReco, 0., sizeof(isValidTrackReco));
    memset(tpcTrkESum, 0., sizeof(tpcTrkESum));

    for(int trk=0; trk<10; trk++){
        if(!isTPCTrack[trk]){continue;}

        int validLayer = 0;
        for(int layer=0; layer<12; layer++){
            double truthW = 0.;
            double w = 0.;
            double posX = 0.;
            double t = 0.;
            for(int r=0; r<64; r++){
                double weight = tpcPad[trk][layer][r][0];
                if(fSmaredData){weight = fRandom -> Gaus(weight, weight*0.01);} // smared tpc adc within energy 5% resolution
                
                if(weight <= 1.){continue;}
                w += weight;
                truthW += tpcPad[trk][layer][r][0];
                double x = fPadPlane -> GetX(layer, r);
                posX += (x*weight);

                double time = tpcPad[trk][layer][r][2]/tpcPad[trk][layer][r][1];
                if(fSmaredData){time = fRandom -> Gaus(time, time*0.008);} // smared tpc TB within time 0.8% resolution
                t += (weight*time);
            }
            if(w <= 1.){continue;}
            posX = (posX/w);
            t = (t/w);
            double posY = fPadPlane -> GetY(layer, 0);

            tpcTrkESum[trk][0] += w;
            tpcTrkESum[trk][1] += truthW;

            fTrack[trk] -> SetPoint(fTrack[trk]->GetN(), posX, posY);
            fTrackTime[trk] -> SetPoint(fTrackTime[trk]->GetN(), posY, t*(5.4 * 0.01));

            validLayer++;
        }

        if(validLayer < 3){continue;}

        fTrack[trk] -> Fit(fFitTrk[trk], "Q");
        fTrackTime[trk] -> Fit(fFitTrkTime[trk], "Q");
        double constantXY = fFitTrk[trk]->GetParameter(0);
        double constantYZ = fFitTrkTime[trk]->GetParameter(0);
        if(-40. <= constantXY && constantXY <= 40.){
            if(0. <= constantYZ && constantYZ <= 150.){
                isValidTrackReco[trk] = true;

                // if(fEventFigSave){
                //     cTrk -> cd(1);
                //     fFitTrk[trk]  -> Draw("same");
                //     cTrk -> cd(2);
                //     fFitTrkTime[trk]  -> Draw("same");
                // }
            }
        }   
    }

    vector<double> tpcVtxCandiate[3]; // [x, y, z]
    memset(tpcVtxCandiate, 0., sizeof(tpcVtxCandiate));

    // clear tpc data
    for(int i=0; i<7; i++){fTruthTrkInfo[i].clear();}
    for(int i=0; i<3; i++){
        fTPCTrkUnit[i].clear();
        if(i < 2){
            fTPCTrkInfo[i].clear();
            fMatchedTrkE[i].clear();
        }
    }
    fTruthTrkID.clear();
    memset(fTruthVretex, 0., sizeof(fTruthVretex));

    int validTrackNum = 0;
    for(int trk=0; trk<10; trk++){
        if(!isValidTrackReco[trk]){continue;}

        double constantXY1 = fFitTrk[trk]->GetParameter(0);
        double slopeXY1 = fFitTrk[trk]->GetParameter(1);
        double constantYZ1 = fFitTrkTime[trk]->GetParameter(0);
        double slopeYZ1 = fFitTrkTime[trk]->GetParameter(1);

        double y_start = fPadPlane -> GetY(0, 0) - fPadPlane->GetPadHeight()/2.;
        double y_end = fPadPlane -> GetY(12, 0) + fPadPlane->GetPadHeight()/2.;

        // ============================== truth track ================================
        fMCTrack  = (LKMCTrack*)fTrackArray -> UncheckedAt(trk);
        int truthPID = fMCTrack -> GetPDG();
        int truthPIDIdx = -1;
        if(truthPID == id_ne20){truthPIDIdx = kNe20;}
        if(truthPID == id_c12 || truthPID == id_c12_decay){truthPIDIdx = kC12;}
        if(truthPID == id_alpha){truthPIDIdx = kAlpha;}
        if(truthPID == id_B8){truthPIDIdx = kB8;}

        double truthPx = fMCTrack -> GetPX();
        double truthPy = fMCTrack -> GetPY();
        double truthPz = fMCTrack -> GetPZ();
        double truthVx = fMCTrack -> GetVX();
        double truthVy = fMCTrack -> GetVY();
        double truthVz = fMCTrack -> GetVZ();
        fDetector -> GetCoordinateGeantToPad(truthVx, truthVy, truthVz);
        double truthE = fMCTrack->GetEnergy();
        double truthdETPC = tpcTrkESum[trk][1];

        double truthP = sqrt(truthPx*truthPx + truthPy*truthPy + truthPz*truthPz);
        double truthUnitX = truthPx/truthP;
        double truthUnitY = truthPz/truthP;
        double truthUnitZ = truthPy/truthP;

        double truthPhi = atan(truthUnitX/truthUnitY);
        double truthTheta = atan(truthUnitZ/truthUnitY);
        double truthLengthY = y_end - truthVy;
        double truthLengthX = truthLengthY*tan(truthPhi);
        double truthLengthZ = truthLengthY*tan(truthTheta);
        double truthLength = sqrt(truthLengthX*truthLengthX + truthLengthY*truthLengthY + truthLengthZ*truthLengthZ);

        fTruthVretex[0] = truthVx;
        fTruthVretex[1] = truthVy;
        fTruthVretex[2] = truthVz;

        fTruthTrkInfo[0].push_back(truthPx);
        fTruthTrkInfo[1].push_back(truthPz);
        fTruthTrkInfo[2].push_back(truthPy);
        fTruthTrkInfo[3].push_back(truthdETPC);
        fTruthTrkInfo[4].push_back(truthLength);

        double truthSiE = 0.;
        for(int si=0; si<fSiInfo[0].size(); si++){
            if(fSiInfo[3][si] == trk){
                truthSiE = fSiInfo[2][si];
            }
        }
        fTruthTrkInfo[5].push_back(truthSiE);
        fTruthTrkInfo[6].push_back(truthPIDIdx);

        // =================== reconstruct track unit vector ====================
        double x_start = (y_start - constantXY1)/slopeXY1;
        double x_end = (y_end - constantXY1)/slopeXY1;
        double z_start = constantYZ1 + y_start*slopeYZ1;
        double z_end = constantYZ1 + y_end*slopeYZ1;

        double diffX = x_end - x_start;
        double diffY = y_end - y_start;
        double diffZ = z_end - z_start;

        double trkLength = sqrt(diffX*diffX + diffY*diffY + diffZ*diffZ);
        fTPCTrkUnit[0].push_back(diffX/trkLength);
        fTPCTrkUnit[1].push_back(diffY/trkLength);
        fTPCTrkUnit[2].push_back(diffZ/trkLength);
        fTPCTrkInfo[0].push_back(trkLength);
        fTPCTrkInfo[1].push_back(tpcTrkESum[trk][0]);
        fTruthTrkID.push_back(trk);

        double siPlaneX = 0.;
        double siPlaneY = 0.;
        double siPlaneZ = fDetector -> fSiPlanePosAtPadPlaneCenter;
        fDetector -> GetCoordinateGeantToPad(siPlaneX, siPlaneY, siPlaneZ);
        double y_atSi = siPlaneY;
        double x_atSi = (y_atSi - constantXY1)/slopeXY1;
        double z_atSi = constantYZ1 + y_atSi*slopeYZ1;

        for(int siIdx=0; siIdx<fSiInfo[0].size(); siIdx++){
            if(-5.+fSiInfo[0][siIdx] <= x_atSi && x_atSi <= 5.+fSiInfo[0][siIdx]){
                if(-5.+fSiInfo[1][siIdx] <= z_atSi && z_atSi <= 5.+fSiInfo[1][siIdx]){
                    double siEnergy = fSiInfo[2][siIdx];
                    if(fSmaredData){
                        siEnergy = fRandom->Gaus(siEnergy, 0.05); // 50 KeV resolution
                    }
                    fMatchedTrkE[0].push_back(siEnergy);
                    fMatchedTrkE[1].push_back(fTruthTrkID.size()-1);
                    fMatchedTrkE[2].push_back(fSiInfo[3][siIdx]);
                    break;
                }
            }
        }

        bool isFileup = false;
        for(int i=trk+1; i<10; i++){
            if(!isValidTrackReco[i]){continue;}

            double constantXY2 = fFitTrk[i]->GetParameter(0);
            double slopeXY2 = fFitTrk[i]->GetParameter(1);
            double constantYZ2 = fFitTrkTime[i]->GetParameter(0);
            double slopeYZ2 = fFitTrkTime[i]->GetParameter(1);

            double leastX = 0.;
            double leastZ = 0.;
            for(int layer=0; layer<12; layer++){
                double posY = fPadPlane -> GetY(layer, 0);

                double x1 = (posY - constantXY1)/slopeXY1;
                double x2 = (posY - constantXY2)/slopeXY2;

                double z1 = constantYZ1 + posY*slopeYZ1;
                double z2 = constantYZ2 + posY*slopeYZ2;

                double distX = fabs(x1-x2);
                double distZ = fabs(z1-z2);

                if(leastX < distX){leastX = distX;}
                if(leastZ < distZ){leastZ = distZ;}
            }

            if(leastX < 4. && leastZ < 2.){isFileup = true;}

            double estiVertexforTwoTrkY = slopeXY1*(constantXY2 - constantXY2)/(slopeXY1 - slopeXY2) + constantXY1;

            double tmpX = 100.;
            double tmpZ = 100.;
            double yAtMinX = 100.;
            double yAtMinZ = 100.;
            for(int v=0; v<200; v++){
                double yRef = -20. + (0.2*v) + estiVertexforTwoTrkY;
                double x1 = (yRef - constantXY1)/slopeXY1;
                double x2 = (yRef - constantXY2)/slopeXY2;
                double z1 = constantYZ1 + yRef*slopeYZ1;
                double z2 = constantYZ2 + yRef*slopeYZ2;

                double distX = fabs(x1-x2);
                double distZ = fabs(z1-z2);

                if(distX < tmpX){tmpX = distX; yAtMinX = yRef;}
                if(distZ < tmpZ){tmpZ = distZ; yAtMinZ = yRef;}
            }
            
            double vtxX1 = (yAtMinX - constantXY1)/slopeXY1;
            double vtxX2 = (yAtMinX - constantXY2)/slopeXY2;
            double vtxZ1 = constantYZ1 + yAtMinZ*slopeYZ1;
            double vtxZ2 = constantYZ2 + yAtMinZ*slopeYZ2;

            tpcVtxCandiate[0].push_back(vtxX1);
            tpcVtxCandiate[0].push_back(vtxX2);
            tpcVtxCandiate[1].push_back(yAtMinX);
            tpcVtxCandiate[1].push_back(yAtMinZ);
            tpcVtxCandiate[2].push_back(vtxZ1);
            tpcVtxCandiate[2].push_back(vtxZ2);
        }
        if(!isFileup){validTrackNum++;}
    }

    // ========================================================================================
    // ========================================================================================
    // ========================================================================================

    // TPC track valideate as well !!!
    if(fMatchedTrkE[0].size() != validTrackNum){return;}
    if(validTrackNum < 3){return;}
    // if(fEventFigSave){
    //     cTrk -> Draw();
    //     cTrk -> SaveAs(Form("./figure/evt%i_sim.pdf", fRun->GetCurrentEventID()));
    // }

    memset(fRecoVertex, 0., sizeof(fRecoVertex));
    for(int i=0; i<3; i++){
        double size = tpcVtxCandiate[i].size();
        double tmpValue = 0.;
        for(int v=0; v<int(size); v++){
            tmpValue += tpcVtxCandiate[i][v];
        }
        fRecoVertex[i] = tmpValue/size;
    }
    for(int trk=0; trk<fTPCTrkUnit[0].size(); trk++){
        double y_end = fPadPlane -> GetY(12, 0) + fPadPlane->GetPadHeight()/2.;

        double x = fTPCTrkUnit[0][trk];
        double y = fTPCTrkUnit[1][trk];
        double z = fTPCTrkUnit[2][trk];

        double vx = fRecoVertex[0];
        double vy = fRecoVertex[1];
        double vz = fRecoVertex[2];

        double phi = atan(z/y);
        double theta = atan(z/y);
        double lengthY = y_end - vy;
        double lengthX = lengthY*tan(phi);
        double lengthZ = lengthY*tan(theta);
        double length = sqrt(lengthX*lengthX + lengthY*lengthY + lengthZ*lengthZ);
        fTPCTrkInfo[0][trk] = length;
    }

    if(fEventFigSave){
        double diffX = fTruthVretex[0] - fRecoVertex[0];
        double diffY = fTruthVretex[1] - fRecoVertex[1];
        double diffZ = fTruthVretex[2] - fRecoVertex[2];
        double deltaR = sqrt(diffX*diffX + diffY*diffY + diffZ*diffZ);
        hVertex -> Fill(deltaR);

        for(int trk=0; trk<fTPCTrkUnit[0].size(); trk++){
            double recodEdx = fTPCTrkInfo[1][trk]/fTPCTrkInfo[0][trk];
            double recoSiE = fMatchedTrkE[0][trk];
            double truthSiE = fTruthTrkInfo[5][trk];

            // double e = 0.85*(recoSiE+fTPCTrkInfo[1][trk]*0.001) + 3.5;
            double e = recoSiE;

            int mcId = fTruthTrkID[trk];
            fMCTrack  = (LKMCTrack*)fTrackArray -> UncheckedAt(mcId);
            double genEnergy = fMCTrack -> GetEnergy();

            int pid = fMCTrack -> GetPDG();
            if(pid == id_c12 || pid == id_c12_decay){pid = 0;}
            else if(pid == id_alpha){pid = 1;}
            else{continue;}
            
            hDedx[pid] -> Fill(e, recodEdx);
            hECorr[pid] -> Fill(e, genEnergy);
        }
    }
    // ============================== Kinematics ==================================
    double alphaMass = 3727.379; // [MeV]

    outTrkNum = fTPCTrkUnit[0].size();
    for(int trk=0; trk<outTrkNum; trk++){
        double unitX = fTPCTrkUnit[0][trk];
        double unitY = fTPCTrkUnit[1][trk];
        double unitZ = fTPCTrkUnit[2][trk];
        double e = fMatchedTrkE[0][trk];
        double dedx = fTPCTrkInfo[1][trk]*0.001;
        
        outTPCDedx[trk] = dedx;
        outTrk[trk][0] = unitX;
        outTrk[trk][1] = unitY;
        outTrk[trk][2] = unitZ;
        outTrk[trk][3] = e;

        double truthPx = fTruthTrkInfo[0][trk];
        double truthPy = fTruthTrkInfo[1][trk];
        double truthPz = fTruthTrkInfo[2][trk];
        double truthP = sqrt(truthPx*truthPx + truthPy*truthPy + truthPz*truthPz);

        fMCTrack = (LKMCTrack*)fTrackArray -> UncheckedAt(trk);
        double pxpx = fMCTrack -> GetPX();

        double truthSiE = fTruthTrkInfo[5][trk];
        int pid = fTruthTrkInfo[6][trk]; 

        outTruthTrk[trk][0] = truthPx;
        outTruthTrk[trk][1] = truthPy;
        outTruthTrk[trk][2] = truthPz;
        outTruthTrk[trk][3] = truthSiE;
        outTruthTrk[trk][4] = pid;

        // double corrE = 0.85*(e+dedx) + 3.5;
        // double E = e + alphaMass;
        // double p = sqrt(E*E - alphaMass*alphaMass);
        // double px = unitX*p;
        // double py = unitY*p;
        // double pz = unitZ*p;

        // double diffP = (truthP - p)/truthP;
        // hMomemtum -> Fill(diffP);
    }

    for(int i=0; i<3; i++){
        outVertex[0][i] = fRecoVertex[i];
    }
    for(int i=0; i<3; i++){
        outVertex[1][i] = fTruthVretex[i];
    }

    if(outTrkNum >= 3){
        tree -> Fill();
    }
}

bool STDTest::EndOfRun()
{
    file -> cd();
    tree -> Write();
    file -> Close();

    TH2Poly* siPoly = new TH2Poly();
    for(int i=0; i<fDetector->fSiDetNum; i++){
        TString siName = fDetector->fSiDetectorName[i];

        double SiLocalPosX = fDetector->fSiDetectorCenter[i][0];
        double SiLocalPosZ = fDetector->fSiDetectorCenter[i][1];
        bool IsRotation = (i<6)? true : false;

        double siWidth = fDetector -> fSiWidth;
        double siHeight = fDetector -> fSiHeight;

        if(IsRotation){siHeight = fDetector -> fSiWidth;}
        if(IsRotation){siWidth = fDetector -> fSiHeight;}

        double x[5];
        double z[5];
        for(int i=0; i<5; i++){
            double xSign = (i<2 || i==4)? -1. : +1.;
            double zSign = (i==1 || i==2)? -1. : +1.;
            x[i] = SiLocalPosX + xSign*siWidth/2.;
            z[i] = SiLocalPosZ + zSign*siHeight/2.;
        }
        siPoly -> AddBin(5, x, z);
    }

    TCanvas* c1 = new TCanvas("","",1800, 2400);
    c1 -> Divide(3,4);

    int tmp = 0;
    for(int i=0; i<3; i++){
        const int cidx = tmp+1;
        c1 -> cd(cidx);
        hPlane[i] -> Draw("colz");
        siPoly -> Draw("same");
        tmp++;
    }

    for(int i=0; i<3; i++){
        const int cidx = tmp+1;
        c1 -> cd(cidx);
        hPlaneAlngpe[i] -> Draw("colz");

        tmp++;
    }

    for(int i=0; i<3; i++){
        const int cidx = tmp+1;
        c1 -> cd(cidx);
        hTrkEnergy[i][0] -> Draw();
        tmp++;
    }

    for(int i=0; i<3; i++){
        const int cidx = tmp+1;
        c1 -> cd(cidx);
        hTrkEnergy[i][1] -> Draw();
        tmp++;
    }

    c1 -> Draw();
    c1 -> SaveAs(Form("mc_trkHit_siDet_sequen.pdf"));

    TCanvas* cSi = new TCanvas("cSi","",600*2, 600*4);
    cSi -> Divide(2,4);

    TLegend* legSi = new TLegend(0.65, 0.65, 0.89, 0.89);
    legSi -> AddEntry( hSiEnergy[0][0], "Ne20");
    legSi -> AddEntry( hSiEnergy[0][1], "C12");
    legSi -> AddEntry( hSiEnergy[0][2], "#alpha");
    legSi -> AddEntry( hSiEnergy[0][4], "ALL");

    for(int si=0; si<siDetNum; si++){
        const int cidx = si+1;
        cSi -> cd(cidx);

        hSiEnergy[si][4] -> Draw();
        for(int i=0; i<3; i++){
            hSiEnergy[si][i] -> Draw("same");
        }
        legSi -> Draw("same");
    }

    cSi -> Draw();
    cSi -> SaveAs("siEnergy_sequen.pdf");

    TCanvas* cSiTrk = new TCanvas("cSiTrk", "" , 600.*5., 600.*8.);
    cSiTrk -> Clear();
    cSiTrk -> Divide(5, 8);

    TLegend* legTrkNum = new TLegend(0.65,0.65, 0.89, 0.89);
    legTrkNum -> AddEntry(hSiEnergyByNum[0][0][0], "Trk == 1");
    legTrkNum -> AddEntry(hSiEnergyByNum[0][0][1], "Trk == 2");
    legTrkNum -> AddEntry(hSiEnergyByNum[0][0][2], "Trk == 3");
    legTrkNum -> AddEntry(hSiEnergyByNum[0][0][3], "ALL");

    int tmp2 = 0;
    for(int si=0; si<siDetNum; si++){
        for(int pid=0; pid<5; pid++){
            const int cidx = tmp2+1;
            cSiTrk -> cd(cidx);

            hSiEnergyByNum[si][pid][3] -> Draw();
            for(int trk=0; trk<3; trk++){
                hSiEnergyByNum[si][pid][trk] -> Draw("same");
            }   
            legTrkNum -> Draw("same");   
            tmp2++;       
        }
    }

    cSiTrk -> Draw();
    cSiTrk -> SaveAs("siE_byTrk_sequen.pdf");

    TCanvas* cReco = new TCanvas("cReco","", 1200., 1800.);
    cReco -> Divide(2,3);
    cReco -> cd(1);
    hVertex -> Draw();

    cReco -> cd(2);
    hDedx[0] -> Draw("colz");
    cReco -> cd(3);
    hDedx[1] -> Draw("colz");

    cReco -> cd(4);
    hECorr[0] -> Draw("colz");
    cReco -> cd(5);
    hECorr[1] -> Draw("colz");
    cReco -> cd(5);
    hMomemtum -> Draw();

    cReco -> Draw();
    cReco -> SaveAs("./RecoData_sequen.pdf");

    return true;
}