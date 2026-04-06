void daqRun()
{
    auto runManager = new STDRunManager();
    runManager -> AddDetector(new TPCDrum());
    runManager -> SetDAQStage(); // for decoding
    runManager -> SetRunList("250818005");

    STDNoiseSubtractor* noiseSubtractor = new STDNoiseSubtractor();
    noiseSubtractor -> DrawRawADCPad();

    STDPulseAnalyser* pulseAnal = new STDPulseAnalyser();
    runManager -> Add(noiseSubtractor);
    runManager -> Add(pulseAnal);

    runManager -> Init();
    runManager -> Run();
}
