void makeSimData()
{
    auto run = new LKRun();
    run -> AddDetector(new TPCDrum());
    // run -> SetInputFile("./simTest.root");
    run -> SetInputFile("./data/one_track/mc_direct_single_0003.root");

    run -> SetOutputFile("./data/one_track_digi/mc_direct_single_0003_all_digi_GAIN500.root");

    run -> Add(new STDDriftElectronMaker);
    run -> Add(new STDElectronicsMaker);
    run -> Add(new STDNoiseSubtractor);
    run -> Add(new STDPulseAnalyser);
    //run -> Add(new STDMCViewer);


    run -> Init();
//    
//    TTree* tree = run -> GetInputChain();
//    tree -> Print();
   //TTree *tree = run -> GetOutputTree();

   //tree -> Print();

//


/*    if (run->GetBranchA("MCTrack",false) != nullptr)
	    run->KeepBranchA("MCTrack");

    if (run -> GetBranchA("MCStepTPCDrum",false) != nullptr)
	    run -> KeepBranchA("MCStepTPCDrum");
*/

    run -> Run();   

}
