#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdint.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <cstdio>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

#include "TStyle.h"
#include "TFile.h"
#include "TAttText.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TH1F.h"
#include "TH2F.h"
#include "THStack.h"
#include "TF1.h"
#include "TGraph.h"
#include "TColor.h"
#include "TROOT.h"
#include "TProfile.h"
#include "TBenchmark.h"
#include "TSystem.h"
#include "TMath.h"
#include "Rtypes.h"

#include "RunControl.h"
#include "Analysis.h"
#include "Channel.h"
#include "Graphix.h"
#include "Calibration.h"

using namespace std;

//	TH2F *sipm_3d = new TH2F("Sipm_Graphical", "Graphical representation of the Sipm", 4,0,4,4,0,4); //for lego plot of the sipm			

void sipmdraw3d();

int main(int argc, char *argv[]){
// One can run the former script in here as well:
bool createPhotonSpectrum = false;
bool useArea = false;
bool testSpace = false;
	if(argc > 1) {
		int optn = atoi(argv[1]);
		if(optn == 1) cout << "Using default values with offline integration." << endl;
		else if(optn == 2){
			gROOT->ProcessLine(".x ../../WaveformAnalysis.cc++");
			return 0;
		} else if(optn == 3){
			cout << "plotting Graphix with existing file /path/<date_and_time>_ChannelResults.txt" << endl;
			Graphix *gphx = new Graphix();
			string temp;
			cout << "Enter intput location and filename: ";
			cin >> temp;

			gphx->Draw3DFromFile(temp);
			cout << "Finished." << endl;
			return 0;
		}else if (optn == 4){
			createPhotonSpectrum = true;
		}else if(optn == 5){
			cout << "Using area calculated by scope directly" << endl;
			useArea = true;
		}else if(optn == 6) {
			cout << "Using area calculated by scope directly and create photon spectrum for the channels from each run" << endl;
			useArea = true;
			createPhotonSpectrum = true;
		}else if(optn == 7) {
			testSpace = true;
		}else{ 
			cout << "Please specify a valid option to run the program." << endl; return 0;
		} 
	}else{
		cout 	<< "Usage: " << argv[0] << " <option>" << endl;
		cout 	<< "The following options are available using numbers:" << endl;
		cout 	<< "1\tUse default options (offline integration)" << endl;
		cout	<< "2\tUse WaveformAnalysis.cc (old script)" << endl;
		cout 	<< "3\tPlot Graphix only (from existing Results-file)" << endl;
		cout 	<< "4\tCreate spectrum of counted photons for the channels in each run." << endl;
		cout	<< "5\tUse area calculation from scope." << endl;
		cout 	<< "6\tCombines number 4 and 5." << endl;
		cout	<< "7\tSecure testspace only." << endl;
		return 0;
	}

	cout << "------------------------------------------------------" << endl;
	cout << "Programm for multichannel analysis starts..." << endl;
// Instantiate:
	cout << "Instantiating classes." << endl;
	RunControl* RC = RunControl::GetInstance();
	Analysis* ana = Analysis::GetInstance();
	ana->SetUseArea(useArea);
	RC->SetUseArea(useArea);

// Set Parameters MUST be the first function to be called:
	RC->SetParameters();
	if (!(RC->correctParameters)){
		cout << "=> Abort and Exit." << endl;
		return 0;
	}	

	RC -> CreateROOTFile();
	RC -> CreateResultsFile();
//	RC -> SetCalibrationFile();		This happens at the end of RC->SetParameters();
if (!testSpace){
	string yes_no = "Y";
	while(yes_no == "Y" || yes_no == "yes" || yes_no == "y" || yes_no == "Yes"){
		RC->PrepareNewRun(); // this also contains the SetUpChannels() and CreateFileList() method

		ana-> PI_Analysis();
		if(!ana->isPI_AnalysisSuccessful) continue;

		ana->pe_Analysis();
		if(createPhotonSpectrum) ana->PhotonSpectrum();
		RC->PrintLogFile();

		cout << "Start new run? ";
		cin>> yes_no;
		if (yes_no != "Y" && yes_no != "yes" && yes_no != "y" && yes_no != "Yes"){
			cout << "You wish to finish your runs. In case you mistyped and instead want to add another run, type \"yes\". For termination type \"no\"." << endl;
			cin >> yes_no;
		}
	}
	
	Graphix *grx = new Graphix();
	grx->Draw3D();
// Summarize measurement in file: MUST be after grx3D->Draw();
	RC->WriteChannelSummary();
}else{
	cout << "Entering Secure Test Space..." << endl;

	string yes_no = "Y";
	while(yes_no == "Y" || yes_no == "yes" || yes_no == "y" || yes_no == "Yes"){
		RC->PrepareNewRun(); // this also contains the SetUpChannels() and CreateFileList() method
		ana->AnalysePeakTimes();
		if(!ana->isPI_AnalysisSuccessful) continue;

		cout << "Start new run? ";
		cin>> yes_no;
		if (yes_no != "Y" && yes_no != "yes" && yes_no != "y" && yes_no != "Yes"){
			cout << "You wish to finish your runs. In case you mistyped and instead want to add another run, type \"yes\". For termination type \"no\"." << endl;
			cin >> yes_no;
		}
	}
}
	cout << "Finished." << endl;
	return 0;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% END OF MAIN FUNCTION

// Save colibration file that was used for this run.
/*
cout << "Copying calibration file that was used for this run:" << endl;
	string calibFile = RC->GetCalibrationFile();
	cout << "From\t" << calibFile << endl;
	stringstream ss;
	ss << RC->GetAnalysisFolder() << RC->ReturnCurrentTimeStamp() << "_CalibFile_For_This_Analysis" << ".txt";
	string calibOut = ss.str();
	cout << "To\t" << calibOut << endl;
	std::ifstream  src(calibFile.c_str(), std::ios::binary);
	std::ofstream  dst(calibOut.c_str(), std::ios::binary);
	dst << src.rdbuf();
*/

