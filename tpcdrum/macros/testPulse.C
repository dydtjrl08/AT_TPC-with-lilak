
Double_t fPulseData[300];

// par[0]=tb, par[1]=adc, par[2]=ped
Double_t pulseFunc(Double_t* x, Double_t* par)
{
    int tbIdx = x[0] - (par[0] - 100);

    double adc = fabs(par[1]);
    if(tbIdx < 0){return par[2]+adc*fPulseData[0];}
    else if(tbIdx >= 300){return par[2]+adc*fPulseData[299];}
    return par[2]+adc*fPulseData[tbIdx];
}


Double_t multiPulse(Double_t* x, Double_t* par)
{
    int pulseNum = par[0];
    Double_t pedestal = par[1];

    Double_t sumFit = 0.;
    for(int i=0; i<pulseNum; i++){
        int parIdx = 2+i*2;
        int tbIdx = x[0] - (par[parIdx] - 100.);
        double adc = par[parIdx+1];

        if(tbIdx < 0 || tbIdx >= 300){continue;}
        sumFit += adc*fPulseData[tbIdx];
    }

    return sumFit+pedestal;
}

Double_t c1Value(int c1)
{
    return 0.+c1*100.;
}
Double_t c2Value(int c2)
{
    return 0.+c2*1.;
}
void testPulse()
{
    TFile* file = new TFile("/home/shlee/workspace/lilak/tpcdrum/common/Pulse_template_502PT_20ns.root", "read");
    TTree* tree = (TTree*)file -> Get("pulse");

    double pulse[300];
    tree -> SetBranchAddress("template", &pulse);
    tree -> GetEntry(0);

    for(int tb=0; tb<300; tb++){
        double adc = pulse[tb];
        if(tb > 100.){adc = adc - pulse[200];}
        fPulseData[tb] = adc;
        if(tb < 50. || tb > 200.){fPulseData[tb] = 0.;}
    }

    TFile* noiseFile = new TFile("/home/shlee/workspace/lilak/tpcdrum/common/Noise_template_test.root", "read");
    TTree* NoiseTree = (TTree*)noiseFile -> Get("event");

    Double_t noiseData[512];
    NoiseTree -> SetBranchAddress("Noise", &noiseData);
    int noiseEventNum = NoiseTree -> GetEntries();

    // ================================================================================
    const int config1 = 5;
    const int config2 = 50;
    const int eventNum = 1000;
    const int generatePulseNum = 2;
    bool drawingOn = false;


    TH1D* hTB = new TH1D("hTB", "", 512, 0, 512);
    hTB -> SetStats(0);
    hTB -> SetLineWidth(2);
    hTB -> SetLineColor(kBlack);

    TH1D* hTruthPulse[generatePulseNum];
    for(int n=0; n<generatePulseNum; n++){
        hTruthPulse[n] = new TH1D(Form("%i_pulse", n), "", 512, 0, 512);
    }
    TGraph* specPoint = new TGraph();
    specPoint -> SetMarkerStyle(20);
    specPoint -> SetMarkerSize(1);
    specPoint -> SetMarkerColor(kRed);

    TLatex* latex = new TLatex();

    TH1D* hChi2 = new TH1D("", "", 40, 0., 10.);


    // ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Migrad");
    // ROOT::Math::MinimizerOptions::SetDefaultErrorDef(0.5);      // NLL
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(0);
    // ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(200000);
    ROOT::Math::MinimizerOptions::SetDefaultTolerance(1e-7);
    // TVirtualFitter::SetMaxIterations(20000);
    ROOT::Math::IntegratorOneDimOptions::SetDefaultIntegrator("GaussLegendre");
    ROOT::Math::IntegratorOneDimOptions::SetDefaultNPoints(6);
    // ROOT::Math::IntegratorOneDimOptions::SetDefaultRelTolerance(1e-10);

    // ==================================================================
    // ==================================================================
    // ==================================================================

    Double_t recoPulseNum[config1][config2][2]; // [reco, truth]
    Double_t misEstiEventNum[config1][config2][2];
    memset(recoPulseNum, 0., sizeof(recoPulseNum));
    memset(misEstiEventNum, 0., sizeof(misEstiEventNum));

    TH1D* hRecoPulseDiff[config1][config2][2]; // [tb, adc];
    for(int c1=0; c1<config1; c1++){
        for(int c2 = 0; c2<config2; c2++){
            hRecoPulseDiff[c1][c2][0] = new TH1D(Form("diffTB_c1_%i_c2_%i", c1, c2), "", 70, -100., 100.);
            hRecoPulseDiff[c1][c2][1] = new TH1D(Form("diffADC_c1_%i_c2_%i", c1, c2), "", 70, -300., 300.);
        }
    }

    TCanvas* canvas1 = new TCanvas("ccanvase","", 600., 600.);

    double genNoise[512];
    int tbTruth = 100;

    for(int c1=0; c1<config1; c1++){
        double adcTruth = 100;
        double adcDiff = c1Value(c1);

        for(int c2 = 0; c2<config2; c2++){
            int tbDiff = c2Value(c2);

            // Generate Multiple pulse events
            for(int event=0; event<eventNum; event++){
                cout << "ADC config: " << c1 << " TB config: " << c2 << " Run: " << event<< endl;

                hTB -> Reset("ICESM");

                gRandom -> SetSeed(0);

                // ================= Pulse generation ======================
                double pedestalLevel = gRandom->Uniform(460., 540.);
                memset(genNoise, 0., sizeof(genNoise));
                int mixNum = gRandom->Uniform(2, 5);
                for(int i=0; i<mixNum; i++){
                    double sign = (gRandom -> Uniform(0, 2) == 0)? -1. : 1;
                    NoiseTree -> GetEntry(gRandom->Uniform(0, noiseEventNum));
                    for(int tb=0; tb<512; tb++){
                        genNoise[tb] += (sign*noiseData[tb]);
                    }
                }
                for(int n=0; n<generatePulseNum; n++){
                    hTruthPulse[n] -> Reset("ICESM");

                    for(int i=0; i<300; i++){
                        double tb = (i-100) + tbTruth+tbDiff*n;
                        double adc = fPulseData[i] * (adcTruth+adcDiff*n);
                        if(tb < 0 || tb > 511){continue;}

                        double entry = hTB -> GetBinContent(tb+1) + adc + pedestalLevel;
                        // if(entry > 4096){entry = 4096;}
                        hTB -> SetBinContent(tb+1, entry - pedestalLevel);
                        hTruthPulse[n] -> SetBinContent(tb+1, adc);
                    }
                }

                double NoiseScale = 1.e4*gRandom->Gaus(1.3, 1.8);
                for(int tb=0; tb<512; tb++){
                    double entry = hTB -> GetBinContent(tb+1) + genNoise[tb]*NoiseScale;
                    hTB -> SetBinContent(tb+1, entry);
                }

                // ====================================================================================
                // Pulse seperation method !!!

                TSpectrum* spectrum = new TSpectrum();
                double threshold_spec = 0.008; // peak must be over Threshold_spec * ADC of highest peak 
                double sigma_spec = 1.; // ?? 

                spectrum->Search(hTB, sigma_spec, "nodraw goff", threshold_spec);
                int numPeak = spectrum->GetNPeaks();
                auto tSpecPeakX = spectrum->GetPositionX();
                auto tSpecPeakY = spectrum->GetPositionY();

                int peakNum = 0;
                for(int peak=0; peak<numPeak; peak++){
                    double tb = tSpecPeakX[peak];
                    double adc = tSpecPeakY[peak];
                    if(adc < 20.){continue;}
                    peakNum++;
                }

                TF1* fit = new TF1(Form("pulseFit_%i", 1), &multiPulse, 10., 490., 2*peakNum+2);
                fit -> SetLineColor(kGreen+2);
                fit -> FixParameter(0, peakNum);
                fit -> FixParameter(1, 0.);

                specPoint -> Set(0);
                for(int peak=0; peak<peakNum; peak++){
                    double tb = tSpecPeakX[peak];
                    double adc = tSpecPeakY[peak];
                    if(adc < 20.){continue;}
                    specPoint -> SetPoint(specPoint->GetN(), tb, adc);
                    
                    fit -> SetParameter(2+peak*2, tb);
                    fit -> SetParameter(2+peak*2+1, adc);
                }
                hTB -> Fit(fit, "RQIM");

                if(peakNum <= generatePulseNum){
                    recoPulseNum[c1][c2][0] += double(peakNum);
                    recoPulseNum[c1][c2][1] += double(generatePulseNum);
                
                    for(int i=0; i<peakNum; i++){
                        double recoTB = fit->GetParameter(2+i*2);
                        double recoADC = fit->GetParameter(2+i*2+1);

                        double truthTB = tbTruth+tbDiff*(peakNum-1-i);
                        double truthADC = adcTruth+adcDiff*(peakNum-1-i);
                        
                        double diffTB = recoTB - truthTB;
                        double diffADC = recoADC - truthADC;
                        hRecoPulseDiff[c1][c2][0] -> Fill(diffTB);
                        hRecoPulseDiff[c1][c2][0] -> Fill(diffADC);
                    }
                }
  
                if(drawingOn){
                    canvas1 -> cd(1);
                    // gPad -> SetLogy();
                    hTB -> GetYaxis()->SetRangeUser(-50., 3500.);
                    hTB -> Draw();
                    for(int n=0; n<generatePulseNum; n++){
                        hTruthPulse[n] -> Draw("same");
                    }
                    specPoint -> Draw("same, p");
                    fit -> Draw("same");

                    TLegend* leg = new TLegend(0.5, 0.65,0.89, 0.89);
                    leg -> AddEntry(hTB, "Gen. Pulse");
                    leg -> AddEntry(specPoint, "Esti. Peaks", "p");
                    leg -> AddEntry(fit, "all Fit");

                    for(int i=0; i<peakNum; i++){
                        TF1* recoPulse = new TF1(Form("recoPulse_%i", i), &pulseFunc, 10., 490., 3);
                        recoPulse -> SetParameter(2, fit->GetParameter(1));
                        recoPulse -> SetParameter(0, fit->GetParameter(2+i*2));
                        recoPulse -> SetParameter(1, fit->GetParameter(2+i*2+1));
                        recoPulse -> Draw("same");

                        if(i==0){
                            leg -> AddEntry(recoPulse, "Pulse Fit");
                        }
                    }
                    latex -> DrawLatexNDC(0.12, 0.85, "Pulse ToyMC");
                    latex -> DrawLatexNDC(0.12, 0.79, Form("Esti. #Peak = %i", peakNum));
                    leg -> Draw("same");
                    canvas1 -> Update();
                    canvas1 -> SaveAs(Form("./pulse/pulse_event%i.png", event));
                }
            }
        }
    }

    int Palette1[10] = {2, 4, 8, 95, 51, 91, 66, 28, 14, 1};

    TGraph* gEffi[config1];
    TGraph* gMean[config1];
    TGraph* gSigma[config1];

    TF1* gaus = new TF1("fit", "gaus", -300., 300.);
    for(int c1=0; c1<config1; c1++){
        gEffi[c1] = new TGraph();
        gEffi[c1] -> SetMarkerColor(Palette1[c1]);
        gEffi[c1] -> SetMarkerStyle(20);
        gEffi[c1] -> SetLineColor(Palette1[c1]);
        gEffi[c1] -> SetMarkerSize(2);
        gMean[c1] = new TGraph();
        gMean[c1] -> SetMarkerColor(Palette1[c1]);
        gMean[c1] -> SetMarkerStyle(20);
        gMean[c1] -> SetLineColor(Palette1[c1]);
        gMean[c1] -> SetMarkerSize(2);
        gSigma[c1] = new TGraph();
        gSigma[c1] -> SetMarkerColor(Palette1[c1]);
        gSigma[c1] -> SetMarkerStyle(20);
        gSigma[c1] -> SetLineColor(Palette1[c1]);
        gSigma[c1] -> SetMarkerSize(2);

        for(int c2=0; c2<config2; c2++){
            int pointEffi = gEffi[c1] -> GetN();
            double recoNum = recoPulseNum[c1][c2][0];
            double truthNum = recoPulseNum[c1][c2][1];

            cout << c1 << " " << c2 << " | " << recoNum << " " << truthNum << " " << recoNum/truthNum << endl;
            double effi = recoNum/truthNum;

            gEffi[c1] -> SetPoint(pointEffi, c2Value(c2), effi);

            gaus -> SetParameters(eventNum/5., 0., 5.);
            hRecoPulseDiff[c1][c2][0] -> Fit(gaus, "QR");
            Double_t mean = gaus -> GetParameter(1);
            Double_t sigma = gaus -> GetParameter(2);
            
            int pointMean = gMean[c1] -> GetN();
            gMean[c1] -> SetPoint(pointMean, c2Value(c2), mean);
            
            int pointSigma = gSigma[c1] -> GetN();
            gSigma[c1] -> SetPoint(pointSigma, c2Value(c2), sigma);
        }
    }


    TCanvas* cDiffPulse = new TCanvas("cDiffPulse", "", 600*config1, 600*config2);
    cDiffPulse -> Divide(config1, config2);

    int cIdx = 0;
    for(int c1=0; c1<config1; c1++){
        for(int c2=0; c2<config2; c2++){
            cIdx++;
            cDiffPulse -> cd(cIdx);
            hRecoPulseDiff[c1][c2][0] -> Draw();
        }
    }
    cDiffPulse -> Draw();
    cDiffPulse -> SaveAs("./diffPulse.pdf");
    
    TCanvas* cSummary = new TCanvas("cSummary", "", 1200, 1200);
    cSummary -> Divide(2,2);

    cSummary -> cd(1);
    TGraph* base = new TGraph();
    base -> SetPoint(0, 0., 1.5);
    base -> SetPoint(1, c2Value(config2), 0.);
    base -> SetMarkerSize(0.);
    base -> SetMarkerColor(0.);
    base -> SetLineWidth(0.);
    base -> Draw("ap");

    for(int c1=0; c1<config1; c1++){
        gEffi[c1] -> Draw("same, pl");
    }

    cSummary -> cd(2);
    TGraph* baseMean = new TGraph();
    baseMean -> SetPoint(0, 0., -50.);
    baseMean -> SetPoint(1, c2Value(config2), 50.);
    baseMean -> SetMarkerSize(0.);
    baseMean -> SetMarkerColor(0.);
    baseMean -> SetLineWidth(0.);
    baseMean -> Draw("ap");

    for(int c1=0; c1<config1; c1++){
        gMean[c1] -> Draw("same, pl");
    }

    cSummary -> cd(3);
    gSigma[0] -> Draw("ap");
    for(int c1=1; c1<config1; c1++){
        gSigma[c1] -> Draw("same, pl");
    }

    cSummary -> Draw();
    cSummary -> SaveAs("./EfficiencyPulse.pdf");
}