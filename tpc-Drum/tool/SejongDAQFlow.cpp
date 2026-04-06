#include "SejongDAQFlow.h"

SejongDAQFlow::SejongDAQFlow()
{
}

TString SejongDAQFlow::AddDash(TString path)
{
    int length = path.Sizeof();
    if(path[length-1] == '/'){return path;}
    else{
        path = path + "/";
        return path;
    }
    return path;
}

RunList SejongDAQFlow::GetRunList(TString input, TString rejectRun, bool isDAQStage)
{
    RunList runList;

    if(input.Index(".list") != -1 || input.Index(".lis") != -1 || input.Index(".text") != -1 || input.Index(".txt") != -1 || input.Index(".dat") != -1){
        vector<Int_t> rejectRuns = GetRunsFromString(rejectRun);
        runList = GetLines(input, isDAQStage);
        runList = RemovedRejectRun(runList, rejectRuns);
        return runList;
    }
    else if(input.Index(".graw") != -1 || input.Index(".root") != -1){
        vector<TString> tmp;
        tmp.push_back(input);
        runList.push_back(make_pair(0, tmp));
        return runList;
    }
    else{
        vector<Int_t> excuteRuns = GetRunsFromString(input);
        vector<Int_t> rejectRuns = GetRunsFromString(rejectRun);
        for(int i=0; i<excuteRuns.size(); i++){
            int run = excuteRuns[i];
            bool isReject = false;
            for(int j=0; j<rejectRuns.size(); j++){
                if(run == rejectRuns[i]){
                    isReject = true;
                    break;
                }
            }
            if(isReject == true){continue;}

            vector<TString> fileList = GetDataBaseFile(run, isDAQStage);
            runList.push_back(make_pair(run, fileList));

        }
        return runList;
    }
    return runList;
}

DAQList SejongDAQFlow::GetDAQList(vector<TString> fileList)
{
    DAQList daqList; 
    
    vector<TString> tmpAsAdFile[ASADNUM];

    // Seperate the AsAd files
    for(int i=0; i<fileList.size(); i++){
        TString file = fileList[i];
        int asadIdx = -1;
        if(file.Index("AsAd") != -1){ // CoBo
            TString idx = file[file.Index("AsAd")+4];
            asadIdx = idx.Atoi();
        }
        if(file.Index("CoBo_") != -1){asadIdx = 0;} // rCoBo
        if(asadIdx < 0 || 4 < asadIdx){continue;}
        tmpAsAdFile[asadIdx].push_back(file);
    }

    // Sort in order the file number
    vector<TString> tmp2AsAdFile[ASADNUM];
    for(int asad=0; asad<ASADNUM; asad++){

        int findDAQNum = 0;
        while(!(findDAQNum == tmpAsAdFile[asad].size())){
            for(int i=0; i<tmpAsAdFile[asad].size(); i++){
                int numberIdx = tmpAsAdFile[asad][i].Index(".graw")-4;
                TString daqNumStr = "";
                for(int idx=0; idx<4; idx++){
                    daqNumStr += tmpAsAdFile[asad][i][numberIdx+idx];
                }
                int daqNum = daqNumStr.Atoi();
                if(findDAQNum == daqNum){
                    tmp2AsAdFile[asad].push_back(tmpAsAdFile[asad][i]);
                    findDAQNum++;
                }
            }
        }

        queue<TString> tmpQueue;
        for(int i=0; i<tmp2AsAdFile[asad].size(); i++){
            tmpQueue.push(tmp2AsAdFile[asad][i]);
        }
        daqList.push_back(tmpQueue);
    }

    return daqList;
}

vector<TString> SejongDAQFlow::GetDAQFiles(TString path)
{
    vector<TString> filePathList;
    TSystemDirectory dir("dir", path);

    TList *listOfFiles = dir.GetListOfFiles();
    
    TIter nextFiles(listOfFiles);
    TObject *objFile;
    while((objFile = nextFiles())){
        TSystemFile* runFile = dynamic_cast<TSystemFile*>(objFile);
        TString fileName = runFile->GetName();
        if(!runFile){continue;}

        TString filePath = path+"/"+fileName;
        if(fileName.Index(".graw") != -1){
            filePathList.push_back(filePath);
        }
    }

    return filePathList;   
}

Bool_t SejongDAQFlow::CheckRunIDFormat(TString run)
{
    if(run.Sizeof() != 10){return false;}
    return true;
}

RunList SejongDAQFlow::RemovedRejectRun(RunList runList, vector<Int_t> rejectRun)
{
    RunList newRunList;
    if(rejectRun.size() == 0){
        newRunList = runList;
        return newRunList;
    }

    for(auto i=0; i<runList.size(); i++){
        int runNumber = runList[i].first;
        bool isReject = false;
        for(int j=0; j<rejectRun.size(); j++){
            if(runNumber == rejectRun[j]){
                isReject = true;
                break;
            }
        }
        if(isReject == true){continue;}
        newRunList.push_back(runList[i]);
    }
    return newRunList;
}


vector<Int_t> SejongDAQFlow::GetRunsFromString(TString run)
{
    vector<Int_t> list;
    if(run == ""){return list;}
    if(run.Index(",") != -1){
        run.ReplaceAll(",", " ");
    }

    TObjArray *tokens = run.Tokenize(" ");
    for(int i=0; i<tokens->GetEntries(); i++){
        TString token = ((TObjString *) tokens -> At(i)) -> GetString();
        if(!CheckRunIDFormat(token)){continue;}
        list.push_back(token.Atoi());
    }

    return list;
}


RunList SejongDAQFlow::GetLines(TString file, bool isDAQStage)
{
    RunList lineList;
    vector<TString> tmpFileArr;

    std::ifstream inputFile(file.Data());
    if(inputFile.is_open()){
        string line;
        while(getline(inputFile, line)){
            TString lineTmp = line;
            lineTmp.ReplaceAll(" ", "");

            if(lineTmp.Index(".graw") != -1 || lineTmp.Index(".root") != -1){
                tmpFileArr.push_back(lineTmp);
            }

            if(CheckRunIDFormat(lineTmp)){ // getting run file in line
                vector<TString> fileinRuns = GetDataBaseFile(lineTmp, isDAQStage);
                lineList.push_back(make_pair(lineTmp.Atoi(), fileinRuns));
            }
        }
        inputFile.close();
    }

    if(lineList.size() == 0 && tmpFileArr.size() != 0){
        lineList.push_back(make_pair(0, tmpFileArr));
    }
    else if(lineList.size() != 0 && tmpFileArr.size() != 0){
        lineList.clear(); // bad list file
    }

    return lineList;
}

vector<TString> SejongDAQFlow::GetDataBaseFile(TString run, bool isDAQStage)
{
    vector<TString> runPathList;

    TString dataBase;
    if(isDAQStage == true){dataBase = kDataBasePath+"daq";}
    if(isDAQStage == false){dataBase = kDataBasePath+"raw";}

    TSystemDirectory dir("dir", dataBase);

    TList *listOfDirs = dir.GetListOfFiles();
    TList *listOfFiles = 0;
    
    TIter next(listOfDirs);
    TObject *objDir;
    TObject *objFile;
    while((objDir = next())){
        TSystemFile* runDir = dynamic_cast<TSystemFile*>(objDir);
        if(runDir && runDir->IsDirectory()){
            
            TString runName = runDir->GetName();
            if(!CheckRunIDFormat(runName)){continue;}
            if(run != runName){continue;}

            TString runPath = dataBase+"/"+runName;
            TSystemDirectory subRunDir("", runPath);
            listOfFiles = subRunDir.GetListOfFiles();

            TIter nextFiles(listOfFiles);
            while((objFile = nextFiles())){
                TSystemFile* runFile = dynamic_cast<TSystemFile*>(objFile);
                TString fileName = runFile->GetName();
                if(!runFile){continue;}

                TString path = dataBase+"/"+runName+"/"+fileName;
                if(fileName.Index(".graw") != -1 && isDAQStage == true){
                    runPathList.push_back(path);
                }
                else if(fileName.Index(".root") != -1 && isDAQStage == false){
                    runPathList.push_back(path);
                }
            }
        }
    }

    return runPathList;
}

vector<TString> SejongDAQFlow::GetDataBaseFile(int runID, bool isDAQStage)
{
    TString run = TString::Itoa(runID, 10);
    return GetDataBaseFile(run, isDAQStage);
}

double SejongDAQFlow::GetFileTime(TString fileName)
{
    int hourIdx = fileName.Index("T");
    int minIdx = fileName.Index("h");
    int secIdx = fileName.Index("m");
    double hour = TString(TString(fileName[hourIdx+1]) + TString(fileName[hourIdx+2])).Atoi();
    double min = TString(TString(fileName[minIdx+1]) + TString(fileName[minIdx+2])).Atoi();
    double sec = double(TString(TString(fileName[secIdx+1]) + TString(fileName[secIdx+2])).Atoi()) + double(TString(fileName[secIdx+4]).Atoi())*0.1;
    double time = hour*3600. + min*60. + sec; // [sec]
    return time;
}



// sort function in order to time stemp
// DAQList SejongDAQFlow::GetDAQList(vector<TString> fileList)
// {
//     DAQList daqList; 
    
//     vector<TString> tmpAsAdFile[ASADNUM];

//     // Seperate the AsAd files
//     for(int i=0; i<fileList.size(); i++){
//         TString file = fileList[i];
//         int asadIdx = -1;
//         if(file.Index("AsAd") != -1){ // CoBo
//             TString idx = file[file.Index("AsAd")+4];
//             asadIdx = idx.Atoi();
//         }
//         if(file.Index("CoBo_") != -1){asadIdx = 0;} // rCoBo
//         if(asadIdx < 0 || 4 < asadIdx){continue;}

//         tmpAsAdFile[asadIdx].push_back(file);
//     }

//     // Sort in order the file number    
//     for(int asad=0; asad<ASADNUM; asad++){
//         vector<double> tmpTime;
//         vector<vector<TString>> tmpTimeSortedFile;
//         vector<vector<TString>> tmpFinalSortedFile;

//         for(int i=0; i<tmpAsAdFile[asad].size(); i++){
//             tmpTime.push_back(GetFileTime(tmpAsAdFile[asad][i]));
//         }
//         std::sort(tmpTime.begin(), tmpTime.end());
//         for(int i=0; i<tmpTime.size(); i++){
//             vector<TString> tmpFiles;

//             if(i == tmpTime.size()-1){
//                 tmpFiles.push_back(tmpAsAdFile[asad][i]);
//                 tmpTimeSortedFile.push_back(tmpFiles);
//                 break;
//             }
//             for(int j=i+1; j<tmpTime.size(); j++){
//                 if(fabs(tmpTime[i] - tmpTime[j]) < 0.25){
//                     tmpFiles.push_back(tmpAsAdFile[asad][i]);
//                     // tmpFiles.push_back(tmpAsAdFile[asad][j]);
//                     break;
//                 }
//                 else{
//                     tmpFiles.push_back(tmpAsAdFile[asad][i]);
//                     break;
//                 }
//             }
//             tmpTimeSortedFile.push_back(tmpFiles);
//         }

//         for(int i=0; i<tmpTimeSortedFile.size(); i++){
//             vector<TString> tmpDAQNumSortedFile;
//             int findDAQNum = 0;
//             while(!(findDAQNum == tmpTimeSortedFile[i].size())){
//                 for(int j=0; j<tmpTimeSortedFile[i].size(); j++){
//                     int numberIdx = tmpTimeSortedFile[i][j].Index(".graw")-4;
//                     TString daqNumStr = "";
//                     for(int idx=0; idx<4; idx++){
//                         daqNumStr += tmpTimeSortedFile[i][j][numberIdx+idx];
//                     }
//                     int daqNum = daqNumStr.Atoi();
//                     if(findDAQNum == daqNum){
//                         tmpDAQNumSortedFile.push_back(tmpTimeSortedFile[i][j]);

//                         cout << tmpTimeSortedFile[i][j] << endl;
//                         findDAQNum++;
//                     }
//                 }
//             }
//             tmpFinalSortedFile.push_back(tmpDAQNumSortedFile);
//         }

//         queue<TString> tmpQueue;
//         for(int i=0; i<tmpFinalSortedFile.size(); i++){
//             for(int j=0; j<tmpFinalSortedFile[i].size(); j++){
//                 tmpQueue.push(tmpFinalSortedFile[i][j]);

//                 cout << tmpFinalSortedFile[i][j] << endl;
//             }
//         }
        
//         daqList.push_back(tmpQueue);
//     }

//     return daqList;
// }