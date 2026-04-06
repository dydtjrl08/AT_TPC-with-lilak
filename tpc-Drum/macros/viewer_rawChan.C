void viewer_rawChan()
{
    TFile* file = new TFile("ped250304003.root", "read");
    TTree* tree = (TTree*)file -> Get("event");

    double outADC[3][4][68][512];
    tree -> SetBranchAddress("adc", &outADC);

    TH1D* hADC = new TH1D("hADC","",512, 0, 512);
    TH2D* ADCProfile = new TH2D("ADCProfile","", 512, 0, 512, 3100, -100., 3000.);

    int eventNum = tree -> GetEntries();
    for(int event=0; event<3000; event++){
        tree -> GetEntry(event);


        for(int asad=0; asad<3; asad++){
            for(int aget=0; aget<4; aget++){
                for(int chan=0; chan<68; chan++){
                    
                    // bool isPulse = false;
                    // for(int tb=50; tb<400; tb++){
                    //     double adc = outADC[asad][aget][chan][tb];
                    //     if(adc > 50.){isPulse = true;}
                    // }
                    // if(isPulse){continue;}

                    for(int tb=0; tb<512; tb++){
                        double adc = outADC[asad][aget][chan][tb];
                        ADCProfile -> Fill(tb+1, adc);
                    }
                }

            }
        }
    }

    TCanvas* c1 = new TCanvas("","",1200,1200);

    c1 -> Divide(2,2);

    c1 -> cd(1);
    ADCProfile -> SetStats(0);
    ADCProfile -> SetTitle("Time bucket; TB [20ns]; ADC");
    ADCProfile -> Draw("colz");
    c1 -> cd(2);
    // gPad -> SetLogy();
    auto profileX = (TH1D*)ADCProfile -> ProjectionY();

    profileX -> SetStats(0);
    profileX -> Scale(1./512.);
    profileX -> SetTitle("ADC projection; ADC; 1/TB N");
    profileX -> Draw("hist");

    c1 -> Draw();

}