#include "STDPulseAnalyser.h"

ClassImp(STDPulseAnalyser);

STDPulseAnalyser::STDPulseAnalyser()
: fMethodOpt(0), fMakePulseShapeMode(false)
{
    fName = "STDPulseAnalyser";
}

bool STDPulseAnalyser::Init()
{
    fDetector = (TPCDrum *) fRun -> GetDetector();
    fDetectorPlane = (STDPadPlane*) fDetector -> GetDetectorPlane();
    fChannelArray = fRun -> GetBranchA("RawPad");

    InitPulseTemplate();

    if(fMakePulseShapeMode){
        fPulseSum = new TH2D("pulseSum", "", 300, 0, 300, 300, 0, 1.);
        fPulseSum -> SetStats(0);
        fPulseSum -> SetTitle("Summed pulse; TB; ADC fraction");
    }
    else{
        fHitArray = fRun -> RegisterBranchA("Hit", "LKHit");
    }

    return true;
}

void STDPulseAnalyser::Exec(Option_t *option)
{
    if(fMakePulseShapeMode){
        FillPulseShape();
    }
    else{
        fHitArray -> Clear("C");
        int hitNum = 0;
        int padNum = fChannelArray -> GetEntries();
        for(int pad=0; pad<padNum; pad++){
            fChannel = (GETChannel*)fChannelArray -> At(pad);

            int asadId = fChannel -> GetAsad();
            int agetId = fChannel -> GetAget();
            int chanId = fChannel -> GetChan();
            int padID = fChannel -> GetPadID();

            int layer = fDetectorPlane -> GetLayerID(asadId, agetId, chanId);
            int row = fDetectorPlane -> GetRowID(asadId, agetId, chanId);
            double x = fDetectorPlane -> GetX(layer, row);
            double y = fDetectorPlane -> GetY(layer, row);

            // if(fDetector -> IsDeadChan(0, padID)){continue;}
            auto adcArr = fChannel -> GetWaveformY();

            double adcPoint = -999;
            double tbPoint = -999;
            if(fMethodOpt == 0){
                GetMaxPoint(adcArr, adcPoint, tbPoint);
            }
            else{
                // to be updated for other method
            }

            if(adcPoint < fThreshold){continue;}

            fHit = (LKHit*)fHitArray -> ConstructedAt(hitNum);
            fHit -> SetPadID(padID);
            fHit -> SetLayer(layer);
            fHit -> SetRow(row);
            fHit -> SetX(x);
            fHit -> SetY(y);
            fHit -> SetTb(tbPoint);
            fHit -> SetW(adcPoint);

            hitNum++;
        }
        lk_info << " number of Hit: " << fHitArray -> GetEntries() << endl;  
    }

}

bool STDPulseAnalyser::EndOfRun()
{
    if(fMakePulseShapeMode){
        PulseShapeAnalyser();
    }
    
    return true;
}

void STDPulseAnalyser::SetMethod(int opt){fMethodOpt = opt;}

void STDPulseAnalyser::GetMaxPoint(int* adcArray, double& maxADC, double& maxTB)
{
    maxADC = *std::max_element(adcArray + fTBStartIdx, adcArray + fTBEndIdx);
    maxTB = double(std::max_element(adcArray + fTBStartIdx, adcArray + fTBEndIdx) - &adcArray[0]) + 0.5;
}

void STDPulseAnalyser::InitPulseTemplate()
{
    TString currentPath = gSystem -> pwd();
    int projectPathIndex = currentPath.Index("tpc-Drum/");
    currentPath.Remove(projectPathIndex);
    fPulseTemplatePath = currentPath + "tpc-Drum/common/";

    double peakingTime = 502;
    double tbUnit = 20;
    if(fPar -> CheckPar("TPCDrum/PeakingTime")){
        peakingTime = fPar -> GetParDouble("TPCDrum/PeakingTime");
    }
    if(fPar -> CheckPar("TPCDrum/TBUnit")){
        tbUnit = fPar -> GetParDouble("TPCDrum/TBUnit");
    }

    TString pulseDataName = Form("%sPulse_template_%iPT_%ins.root", fPulseTemplatePath.Data(), int(peakingTime), int(tbUnit));
    TFile* pulseFile = new TFile(pulseDataName, "read");
    if(!pulseFile->IsOpen()){
        lk_info << "There is no " << pulseDataName << ", Turn on the Pulse shape analysis mode." << endl;
        fMakePulseShapeMode = true;
        return;
    }
    lk_info << "Found a pulse data, " << pulseDataName << endl;

    TTree* pulseTree = (TTree*)pulseFile -> Get("pulse");    
    pulseTree -> SetBranchAddress("template", &fPulseTemplate);
    pulseTree -> GetEntry(0);
}

void STDPulseAnalyser::FillPulseShape()
{
    int padNum = fChannelArray -> GetEntries();
    for(int pad=0; pad<padNum; pad++){
        fChannel = (GETChannel*)fChannelArray -> At(pad);
        auto adcArr = fChannel -> GetWaveformY();

        double maxADC = -999;
        double maxTB = -999;
        GetMaxPoint(adcArr, maxADC, maxTB);
        maxTB -= 0.5;

        if(maxADC < fThreshold || maxADC > 3400){continue;}
        double normFactorADC = 1./maxADC;
        double shiftTB = fMaxADCBase - maxTB;

        for(int tb=0; tb<450; tb++){
            double adc = adcArr[tb] * normFactorADC;
            if(tb == fMaxADCBase || adc < 0.001){continue;}
            fPulseSum -> Fill(tb+shiftTB, adc);
        }
    }
}

void STDPulseAnalyser::PulseShapeAnalyser()
{
    double peakingTime = fPar -> GetParDouble("TPCDrum/PeakingTime");
    double tbUnit = fPar -> GetParDouble("TPCDrum/TBUnit");

    double tbBoundaryLower = fMaxADCBase - peakingTime/tbUnit*2. -10;
    double tbBoundaryUpper = fMaxADCBase + peakingTime/tbUnit*4. -10;
    
    TH1D* pulseProfile = new TH1D("pulseProfile", "Pulse_Profile", 300, 0., 300.);

    double offsetADC = 0.;
    double offsetNum = 0.;
    for(int bx=0; bx<fPulseSum->GetNbinsX(); bx++){
        double xValue = fPulseSum -> GetXaxis() -> GetBinCenter(bx+1);

        double mean = 0.;
        double weight = 0.;
        for(int by=0; by<fPulseSum->GetNbinsY(); by++){
            double entry = fPulseSum -> GetBinContent(bx+1, by+1);
            double yValue = fPulseSum -> GetYaxis() -> GetBinCenter(by+1);
            mean += (pow(entry,4.)*yValue);
            weight += pow(entry,4.);
                   
        }
        mean /= weight;
        pulseProfile -> Fill(xValue, mean);
        
        if(xValue < tbBoundaryLower){
            offsetADC += mean;
            offsetNum += 1.;
        }
    }
    // subtract the remnant offset
    offsetADC /= offsetNum;
    for(int bx=0; bx<pulseProfile->GetNbinsX(); bx++){
        double xValue = pulseProfile -> GetXaxis() -> GetBinCenter(bx+1);
        double entry = pulseProfile -> GetBinContent(bx+1);
        fPulseTemplate[bx] = entry - offsetADC;
    }
    // for(int tb=0; tb<fPulseTemplateArrNum; tb++){
    //     if(tb < tbBoundaryLower){fPulseTemplate[tb] = 0.;} // calibrate pulse before starting of pulse
    //     if(tb > tbBoundaryLower){fPulseTemplate[tb] = 0.;} // calibrate pulse before starting of pulse
    // }

    // Save the pulse template
    TFile* pulseFile = new TFile(Form("%sPulse_template_%iPT_%ins.root", fPulseTemplatePath.Data(), int(peakingTime), int(tbUnit)), "recreate");
    TTree* pulseTree = new TTree("pulse", "pulse");
    pulseTree -> Branch("template", fPulseTemplate, Form("template[%i]/D", fPulseTemplateArrNum));
    pulseTree -> Branch("PeakingTime", &peakingTime, "peakingTime/D");
    pulseTree -> Branch("TbUnit", &tbUnit, "TbUnit/D");
    pulseTree -> Fill();
    pulseFile -> cd();
    pulseTree -> Write();
    pulseFile -> Close();

    // drawing pulse template
    TCanvas* cPulseCanvas = new TCanvas("","",1200,1200);
    cPulseCanvas -> Divide(2,1);
    cPulseCanvas -> cd(1);
    gPad -> SetLogz();
    fPulseSum -> Draw("colz");
    pulseProfile -> SetLineColor(kRed);
    pulseProfile -> SetLineWidth(2);
    pulseProfile -> Draw("same, hist");
    cPulseCanvas -> Draw();

    cPulseCanvas -> SaveAs(Form("./pulse_template_%iPT_%ins.pdf", int(peakingTime), int(tbUnit)));
}
