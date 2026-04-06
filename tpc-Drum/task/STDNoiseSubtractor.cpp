#include "STDNoiseSubtractor.h"

ClassImp(STDNoiseSubtractor);

STDNoiseSubtractor::STDNoiseSubtractor()
: fMakeNoiseShapeMode(false), fOnDrawRawADC(false)
{
    fName = "STDNoiseSubtractor";
}

bool STDNoiseSubtractor::Init()
{
    fDetector = (TPCDrum *) fRun -> GetDetector();
    fDetectorPlane = (STDPadPlane*) fDetector -> GetDetectorPlane();
    fChannelArray = fRun -> GetBranchA("RawPad");

    fADCTmp = new TH1D("ADCTmp", "", 512, 0, 512);
    for(int asad=0; asad<4; asad++){
        fADCTemplate[asad] = new TH2D(Form("asad_%i", asad), "", 512, 0, 512, 2000, 0., 2000.);
    }

    if(fMakeNoiseShapeMode){
        TString currentPath = gSystem -> pwd();
        int projectPathIndex = currentPath.Index("tpc-Drum/");
        currentPath.Remove(projectPathIndex);
        TString noiseDataPath = currentPath + "tpc-Drum/common/";

        TString noiseDataName = Form("%sNoise_template.root", noiseDataPath.Data());
        fNoiseFile = new TFile(noiseDataName, "recreate");
        fNoiseTree = new TTree("event", "event");
        fNoiseTree -> Branch("Noise", fNoiseData, Form("Noise[%i]/D", TIMEBUCKET));
    }

    if(fOnDrawRawADC){
        cout << "STDNoiseSubtractor::Init() -- " << "Drawing mode ON "<< endl;
        double x[4] = {-100., -100., 100., 100.};
        double y[4] = {-34., 166., 166., -34.};

        hBoundary = new TH2Poly();
        hBoundary -> SetStats(0);
        hBoundary -> SetTitle("Pad ADC (Noise Subtraction); x [mm]; y [mm]");
        hBoundary -> AddBin(4, x, y);

        hPolyADC_subt = fDetectorPlane -> GetPadPlanePoly();
        hPolyADC_subt -> GetZaxis() -> SetRangeUser(10., 4000);
        hPolyADC_subt -> SetTitle("Pad ADC (Noise Subtraction); x [mm]; y [mm]");

        hPolyTime_subt = (TH2Poly*)hPolyADC_subt->Clone("polyTime_subt");
        hPolyTime_subt -> GetZaxis() -> SetRangeUser(1., 512.);
        hPolyTime_subt -> SetTitle("Pad Time (Noise Subtraction); x [mm]; y [mm]");

        hPolyTime_Y = new TH2D("PolyTime_y", "", 12, -6.1, 139.1, 70, 0., 512);
        hPolyTime_Y -> SetTitle("Pad Time (Noise Subtraction); y [mm]; TB");

        hTB_subt = new TH2D("TB_subt","", 512, 0, 512, 3600, -100, 3500);
        hTB_subt -> SetTitle("Timebucket summary; TB ; ADC");

        cRawPad = new TCanvas("rawPad", "", 1400., 1200.);
        cRawPad -> Divide(2,2);


        cTestCanvas = new TCanvas("cTestCanvas","",600., 600.);
        hTestTB = new TH1D("hTestTB", "", 512, 0., 512.);
        hTestTB -> SetStats(0);
        hTestRawTB = new TH1D("hTestTB", "", 512, 0., 512.);
        hTestRawTB -> SetLineColor(kRed);
        hTestRawTB -> SetStats(0);
    }

    return true;
}

void STDNoiseSubtractor::Exec(Option_t *option)
{
    const int asadNum = 4;

    // test !! Todo fixed a tmpADC and noise template method 25/02/28
    if(fOnDrawRawADC){
        hPolyADC_subt -> ClearBinContents();
        hPolyTime_subt -> ClearBinContents();
        hPolyTime_Y -> Reset("ICESM");
        hTB_subt -> Reset("ICESM");
    }

    double tmpADC[ASADNUM][AGETNUM][CHANNUM][TIMEBUCKET];
    memset(tmpADC, 0, sizeof(tmpADC));

    double tmpPadNum[asadNum]; // [asad][padNum]
    memset(tmpPadNum, 0., sizeof(tmpPadNum));

    const int tbIdxNum = fTBIdxNum;
    double tmpTempADC[asadNum][tbIdxNum][2]; // [asad][tbIdx][mean, StdDev]
    memset(tmpTempADC, 0., sizeof(tmpTempADC));

    const int padNum = fChannelArray -> GetEntries();
    for(int pad=0; pad<padNum; pad++){
        fChannel = (GETChannel*)fChannelArray -> At(pad);
        auto rawADCArr = fChannel -> GetWaveformY();

        int asadID = fChannel -> GetAsad();
        int agetID = fChannel -> GetAget();
        int chanID = fChannel -> GetChan();
        if(chanID == -1){continue;}

        // if(fDetector -> IsDeadChan(0, padID)){continue;} // dead Channel 

        // Find a Mean of ADC by Pad
        double tmpADCMean = 0.;
        for(int tb=fTBStartIdx; tb<fTBEndIdx; tb++){
            tmpADCMean += double(rawADCArr[tb]);
        }
        tmpADCMean /= double(fTBEndIdx - fTBStartIdx);

        // Subtract the ADC mean, added ADC offset into tmpADC for analize
        for(int tb=fTBStartIdx; tb<fTBEndIdx; tb++){
            tmpADC[asadID][agetID][chanID][tb] = double(rawADCArr[tb]) + fTmpADCOffset - tmpADCMean;
        }

        for(int tbIdx=0; tbIdx<fTBIdxNum; tbIdx++){
            int tb = (tbIdx+1)*fTBInterval;
            tmpTempADC[asadID][tbIdx][0] += tmpADC[asadID][agetID][chanID][tb];
        }
        tmpPadNum[asadID] += 1.;
    }

    for(int pad=0; pad<padNum; pad++){
        fChannel = (GETChannel*)fChannelArray -> At(pad);
        int asadID = fChannel -> GetAsad();
        int agetID = fChannel -> GetAget();
        int chanID = fChannel -> GetChan();
        if(chanID == -1){continue;}

        for(int tbIdx=0; tbIdx<fTBIdxNum; tbIdx++){
            int tb = (tbIdx+1)*fTBInterval;
            double mean = tmpTempADC[asadID][tbIdx][0]/tmpPadNum[asadID];
            double adc = tmpADC[asadID][agetID][chanID][tb];

            tmpTempADC[asadID][tbIdx][1] += (adc - mean)*(adc - mean);
        }
    }

    for(int asad=0; asad<asadNum; asad++){
        fADCTemplate[asad] -> Reset("ICESM");
    }

    for(int pad=0; pad<padNum; pad++){
        fChannel = (GETChannel*)fChannelArray -> At(pad);
        int asadID = fChannel -> GetAsad();
        int agetID = fChannel -> GetAget();
        int chanID = fChannel -> GetChan();
        if(chanID == -1){continue;}

        int rejectIdx = 0;
        for(int tbIdx=0; tbIdx<fTBIdxNum; tbIdx++){
            int tb = (tbIdx+1)*fTBInterval;
            double mean = tmpTempADC[asadID][tbIdx][0]/tmpPadNum[asadID];
            double stdDev = sqrt(tmpTempADC[asadID][tbIdx][1]/(tmpPadNum[asadID]-1.));
            if(fabs(tmpADC[asadID][agetID][chanID][tb] > mean + 1.5*stdDev)){rejectIdx++;}
        }

        if(rejectIdx > 2){continue;} // cut the pulse-like-pad

        for(int tb=fTBStartIdx; tb<fTBEndIdx; tb++){
            fADCTemplate[asadID] -> Fill(tb, tmpADC[asadID][agetID][chanID][tb]);
        }
    }

    for(int asad=0; asad<asadNum; asad++){
        fNoiseTemplate[asad] = (TProfile*)fADCTemplate[asad] -> ProfileX();
    }

    for(int pad=0; pad<padNum; pad++){
        fChannel = (GETChannel*)fChannelArray -> At(pad);
        int asadID = fChannel -> GetAsad();
        int agetID = fChannel -> GetAget();
        int chanID = fChannel -> GetChan();
        if(chanID == -1){continue;}

        auto rawADCArr = fChannel -> GetWaveformY();

        fADCTmp -> Reset("ICESM");
        for(int tb=fTBStartIdx; tb<fTBEndIdx; tb++){
            fADCTmp -> SetBinContent(tb+1, double(rawADCArr[tb])+fTmpADCOffset);
        }

        double noiseIntegral = fNoiseTemplate[asadID]->Integral(fTBStartIdx, 40) + fNoiseTemplate[asadID]->Integral(470, fTBEndIdx);
        fNoiseTemplate[asadID] -> Scale(1./noiseIntegral);
        double NoiseOffset = fNoiseTemplate[asadID]->Integral(fTBStartIdx+1, fTBEndIdx-1) / double(fTBEndIdx-1 - fTBStartIdx+1);

        double adcIntegral = fADCTmp->Integral(fTBStartIdx, 40) + fADCTmp->Integral(470, fTBEndIdx);
        fNoiseTemplate[asadID] -> Scale(adcIntegral);

        int tmpADC2[512];
        memset(tmpADC2, 0, sizeof(tmpADC2));
        for(int tb=fTBStartIdx; tb<fTBEndIdx; tb++){
            double adc = fADCTmp -> GetBinContent(tb+1) - fTmpADCOffset;
            double noise =  fNoiseTemplate[asadID] -> GetBinContent(tb+1) - fTmpADCOffset - NoiseOffset;
            tmpADC2[tb] = int(adc - noise);
        }

        if(fOnDrawRawADC){
            int maxADC = 0;
            int maxTime = 0.;

            hTestTB -> Reset("ICESM");
            hTestRawTB -> Reset("ICESM");
            for(int tb=0; tb<512; tb++){
                hTB_subt -> Fill(tb, tmpADC2[tb]);
                hTestTB -> Fill(tb, tmpADC2[tb]);
                hTestRawTB -> Fill(tb, tmpADC[asadID][agetID][chanID][tb]);
                if(maxADC < tmpADC2[tb]){
                    maxADC = tmpADC2[tb];
                    maxTime = tb;
                }
            }

            if(maxADC < 40){continue;}
            if(maxTime < 10 || maxTime > 500){continue;}
            int layer = fDetectorPlane -> GetLayerID(asadID, agetID, chanID);
            int row = fDetectorPlane -> GetRowID(asadID, agetID, chanID);
            double x = fDetectorPlane -> GetX(layer, row);
            double y = fDetectorPlane -> GetY(layer, row);

            hPolyADC_subt -> Fill(x, y, maxADC);
            hPolyTime_subt -> Fill(x, y, maxTime);
            hPolyTime_Y -> Fill(y, maxTime);


            cTestCanvas -> cd();
            // gPad -> SetLogy();
            hTestTB -> GetYaxis()->SetRangeUser(-200, 500.);
            hTestTB -> Draw("hist");
            hTestRawTB -> Draw("same, hist");
            cTestCanvas -> Update();
            // cTestCanvas -> SaveAs(Form("event%i_pad%i.png", fRun->GetCurrentEventID(), pad));
        }
        fChannel -> SetWaveformY(tmpADC2);
    }

    if(fMakeNoiseShapeMode){
        SaveNoiseTemplate();
    }

    //Drawing 
    // if(fOnDrawRawADC){
    //     cRawPad -> cd(1);
    //     hBoundary -> Draw();
    //     hPolyADC_subt -> Draw("same, colz");
    //     cRawPad -> cd(2);
    //     hPolyTime_Y -> Draw("colz");
    //     cRawPad -> cd(3);
    //     hPolyTime_subt -> Draw("colz");
    //     cRawPad -> cd(4);
    //     hTB_subt -> Draw("colz, hist");

    //     cRawPad -> Update();
    //     cRawPad -> SaveAs(Form("./figure/rawPad_evt%i.png", fRun->GetCurrentEventID()));
    // }
}

bool STDNoiseSubtractor::EndOfRun()
{
    if(fMakeNoiseShapeMode){
        fNoiseFile -> cd();
        fNoiseTree -> Write();
        fNoiseFile -> Close();
    }

    return true;
}

void STDNoiseSubtractor::SaveNoiseTemplate()
{
    for(int asad=0; asad<ASADNUM; asad++){
        double noiseIntegral = fNoiseTemplate[asad]->Integral(fTBStartIdx, 40) + fNoiseTemplate[asad]->Integral(470, fTBEndIdx);
        if(noiseIntegral < 0.01){continue;}
        fNoiseTemplate[asad] -> Scale(1./noiseIntegral);
        memset(fNoiseData, 0., sizeof(fNoiseData));

        double NoiseOffset = fNoiseTemplate[asad]->Integral(fTBStartIdx+1, fTBEndIdx-1) / double(fTBEndIdx-1 - fTBStartIdx+1);

        for(int tb=fTBStartIdx; tb<fTBEndIdx; tb++){
            fNoiseData[tb] = fNoiseTemplate[asad]->GetBinContent(tb+1) - NoiseOffset;
        }
        fNoiseTree -> Fill();
    }
}