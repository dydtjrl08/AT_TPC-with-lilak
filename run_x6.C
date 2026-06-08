void run_x6()
{
    TString lilakPath = gSystem->Getenv("LILAK_PATH");

    auto run = new X6SiHitAnalyzer();

    run->AddInputFile(lilakPath + "/data/test_260509001.root");
    //run->AddInputFile("../macros/test_260509001.root");
    run->SetOutputFile(lilakPath + "/data/test_260509001_X6SiHitAnalyzer.root");
    //run->SetOutputFile("../macros/test_260509001_X6SiHitAnalyzer.root");

    run->LoadConf("26050900n_configuration.csv");
    run -> SetOutputDir("figures/x6_hit_raw");    
    run->SetRunMode("all");

    // 현재 네가 확인한 기준.
    run->SetPeakCut(200.0);

    // 지금은 ADC 300만으로 거의 X-ray만 잘 걸러진다고 했으니 기본은 time cut OFF.
    run -> SetJunctionPeakCut(200.0);
    
    run -> SetPerChannelHitHistogramBinning(100,0,4100);

    run->SetUseTimeCut(kFALSE);

    // 나중에 안전장치 넣고 싶으면 이거 켜면 됨.
    // run->SetUseTimeCut(kTRUE);
    // run->SetPeakTimeWindow(70, 180);

    run->SetSaveWaveforms(kFALSE);
    run -> SetAutoTermination(false);
    if (!run->Init()) {
        std::cout << "X6SiHitAnalyzer initialization failed." << std::endl;
        return;
    }

    // 필요하면 branch 확인
    // run->PrintInputBranchInfo();

    // 필요하면 layout 확인
    // run->Draw2DFrame();

    // 앞 10개만 테스트
    // run->Run(10);

    // 전체 실행
    run->Run();
    run -> DrawEnergySpectrum();
    run -> DrawPerChannelHitSpectra();
    //run->PrintX6HitSummary();

}
