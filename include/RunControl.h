#include "TH1F.h"
#include "TFile.h"
//#include "EventType.h"

#include <stdio.h>
#include <iostream>

using namespace std;
class Channel;
class RunControl{
	public:
		static 	RunControl *GetInstance(){	// makes sure only one instance of this class can be created
			if(RunControl::fInstance == NULL){
				RunControl::fInstance = new RunControl;
			}
			return fInstance;
		
		}
		~RunControl();	// Destructor
		void PrepareNewRun();
		void SetUpChannels();
		void SetParameters();
		void CreateROOTFile();
		void CreateIndividualROOTFile();
		void CreateAnalysisDirectory();
		void SetCalibrationFile();
		void PrintParameters();
		void PrintLogFile();
		void End();		
		double AdjustBinWidth(double input_bw);			
		void CalculateUpperBoundary();	
		void CreateFileList();	
		void WriteChannelResults();
		void WriteAllChannelResults();
		void CreateCorrectFolderPaths();
		void SaveAllChannels();
		void CreateResultsFile();
		void WriteChannelSummary();



// Getter and Setter functions:
		string GetTimeStamp();
		string GetTFileName(){return TFileName;}
		string GetSiPMChannel(int ch){return SiPMChannels[ch];}
		TFile* GetROOTFile(){return results;}
		string GetResFileName(){return resFileName;}
		Channel* GetChannel(int i){return h0[i];}
		double GetBinWidth(){return binWidth;}
		string GetCalibrationFile(){return calibrationFile;}
		string GetAnalysisFolder(){return AnalysisFolder;}
		string ReturnCurrentTimeStamp(){return timeStamp;}
		void SetUseArea(bool b){useArea = b;}
// Members:
		Channel *h0[4];
		Channel *allChannels[128];
		string globalPath;	
		string folderName;
		string resFileName;
		string TFileName;
		string AnalysisFolder;
		TFile *results;
		string inputFileName;		
		string outputFileName;
		string prefix;			
		int num_Files;
		int num_channels;
		int num_runs;
		double timePerStep;
		string fileList[4][30000];	
		string SiPMChannels[4];
		double bias;

		bool UseChannel[4];

		float binWidth;
		int num_Bins;
		int stepsPerBin;			

		double histo_low;		
		double histo_high;		
	
		int recordLength; 		
		int timeBeforeTrigger;	
		int triggerOffset;	
		double voltageResolution;	
		double sigma_correction_boundary;

		bool correctParameters;

		string timeStamp;
		string calibrationFile;
		double default_m;
		double default_t;
		bool CorrectBG;
	private:
		RunControl();		// private constructor


		static RunControl* fInstance;
		bool useArea;

};
