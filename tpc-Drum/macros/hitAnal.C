void hitAnal()
{
    auto runManager = new STDRunManager();
    runManager -> AddDetector(new TPCDrum());
    // runManager -> SetDataPath("/home/shlee/workspace/lilak/tpc_Drum/macros/");
    runManager -> SetRunList("250304003");

    STDChannelViewer* viewer = new STDChannelViewer();
    runManager -> Add(viewer);

    runManager -> Init();
    runManager -> Run();
}