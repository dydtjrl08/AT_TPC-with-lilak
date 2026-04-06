void daqRun()
{
    auto runManager = new STDRunManager();
    runManager -> AddDetector(new TPCDrum_SiTest());
    runManager -> SetDAQStage(); // for decoding
    // runManager -> AddPar("TPCDrum_Reco.mac");
    runManager -> SetRunList("260130001");

//    runManager -> SetEventNumber(2);

    STDNoiseSubtractor* noiseSubtractor = new STDNoiseSubtractor();
    noiseSubtractor -> DrawRawADCPad();
    noiseSubtractor -> MakeNoiseShape();

    STDPulseAnalyser* pulseAnal = new STDPulseAnalyser();
    pulseAnal -> MakePulseShape();

    runManager -> Add(noiseSubtractor);
    runManager -> Add(pulseAnal);

    runManager -> Init();
    runManager -> Run();
}
