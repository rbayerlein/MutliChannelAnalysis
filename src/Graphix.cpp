#include "Channel.h"
#include "RunControl.h"
#include "Analysis.h"
#include "Graphix.h"

#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Constructor:
Graphix::Graphix(){
// Pre-initialize photons per channel to be zero.
	for(int i = 0; i < 16; i++){
		PhotonsPerChannel[i]=0;
	}
max_num_Photons = 0;
goodCounter = 0; 	// counts the number of channels that contribute to the Graphs. This is neccessary to calculate the av num of photons for the whole array.
}

//Destructor:
Graphix::~Graphix(){
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int Graphix::GetChipID(string SiPMChannel){
	int ChipID;
	if (SiPMChannel == "A1" || SiPMChannel == "a1") ChipID = 0;
	else if (SiPMChannel == "A2" || SiPMChannel == "a2") ChipID = 1;
	else if (SiPMChannel == "A3" || SiPMChannel == "a3") ChipID = 2;
	else if (SiPMChannel == "A4" || SiPMChannel == "a4") ChipID = 3;
	else if (SiPMChannel == "B1" || SiPMChannel == "b1") ChipID = 4;
	else if (SiPMChannel == "B2" || SiPMChannel == "b2") ChipID = 5;
	else if (SiPMChannel == "B3" || SiPMChannel == "b3") ChipID = 6;
	else if (SiPMChannel == "B4" || SiPMChannel == "b4") ChipID = 7;
	else if (SiPMChannel == "C1" || SiPMChannel == "c1") ChipID = 8;
	else if (SiPMChannel == "C2" || SiPMChannel == "c2") ChipID = 9;
	else if (SiPMChannel == "C3" || SiPMChannel == "c3") ChipID = 10;
	else if (SiPMChannel == "C4" || SiPMChannel == "c4") ChipID = 11;
	else if (SiPMChannel == "D1" || SiPMChannel == "d1") ChipID = 12;
	else if (SiPMChannel == "D2" || SiPMChannel == "d2") ChipID = 13;
	else if (SiPMChannel == "D3" || SiPMChannel == "d3") ChipID = 14;
	else if (SiPMChannel == "D4" || SiPMChannel == "d4") ChipID = 15;
	else return -1;
	return ChipID;
}


void Graphix::Draw3D(){
	cout << "--------------------------------------" << endl;
	cout << "Drawing 3D results of photon count..." << endl;

	RunControl *RC = RunControl::GetInstance();

	sipm_3d = new TH2F("Sipm_Graphical", "Graphical representation of the Sipm", 4,0,4,4,0,4); //for lego plot of the sipm	
	ROOTFile = RC->GetROOTFile();

	resFile = RC->GetResFileName();
	cout << resFile << endl;

	InitializeDrawing();

}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void Graphix::Draw3DFromFile(string file){
	sipm_3d = new TH2F("Sipm_Graphical", "Graphical representation of the Sipm", 4,0,4,4,0,4); //for lego plot of the sipm	
	resFile = file;
	string temp;
	cout << "Enter output location and filename (/path/folder/name.root): ";
	cin >> temp;
	ROOTFile = new TFile(temp.c_str(), "recreate");
	InitializeDrawing();

}

void Graphix::InitializeDrawing(){
	cout << "Initialize..." << endl;
	ifstream in;
	in.open(resFile.c_str());
	if(!in) return;
	string dummy_scope="0";	// pre-initialize to make first while loop work properly.
	string SiPMChannel;
	double dummy_slope;
	double dummy_offset;
	double av_num_Photons;
	double RMS;
	string dummy_run;

	while(!in.eof()){
		if(dummy_scope != "0" && dummy_scope != "1" && dummy_scope != "2" && dummy_scope != "3") break;
// Read in line

		in >> dummy_scope >> SiPMChannel >> dummy_slope >> dummy_offset >> av_num_Photons >> RMS >> dummy_run ;
//		cout << dummy_scope << "\t" << SiPMChannel << "\t" << dummy_slope <<  "\t" << dummy_offset << "\t" <<  av_num_Photons <<"\t" << RMS << "\t" <<  dummy_run << "\n";

		if(dummy_slope == -1) continue;

// Access Chip ID and save it in temporary variable
		int tempID = GetChipID(SiPMChannel);
		if (tempID == -1) continue;
		else goodCounter++;
		cout << "SiPM Channel: " << SiPMChannel << "\t" << "Channel ID: "<< tempID << "\tav Num Photons: " << av_num_Photons << "\tRMS: " << RMS << endl;

// Write av num photons in array PhotonsPerChannel[]
		PhotonsPerChannel[tempID]=av_num_Photons;
		if (max_num_Photons < av_num_Photons) max_num_Photons = av_num_Photons;
		

	}
	in.close();	
	cout << " draw..." << endl;
	sipmdraw3d();	
}



//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void Graphix::sipmdraw3d()
{
	TCanvas *c1=new TCanvas("sipm","sipm",600,600); 	//canvas for 2d drawing of sipm
	TCanvas *c3=new TCanvas("sipm3d","3dsipm",3000,3000); 	//canvas for 3d drawing of sipm (simply storing the histogram doesn't work properly)

//	sipm_3d->GetXaxis()->SetTitle("Bottom of SiPM");
	sipm_3d->GetXaxis()->SetNdivisions(4);
	sipm_3d->GetYaxis()->SetNdivisions(4);

	int max = ((int)max_num_Photons)+1;
	sipm_3d->GetZaxis()->SetRangeUser(0,max);

	int k=0;
	for(int y=0;y<4;y++)
	{
		for(int x=0;x<4;x++)
		{
			sipm_3d->Fill(x,y,PhotonsPerChannel[k]);
			k++;
			
		}
	}

	c1->cd();
	c1->SetRightMargin(0.18);
	sipm_3d->Draw("COLZ");
	sipm_3d->Draw("SAME TEXT");
	c1->Write();

	c3->cd();
	sipm_3d->SetStats(0);
	c3->SetBottomMargin(0.08);
	sipm_3d->Draw("LEGO2 PLC");
	c3->Write();

	cout << " done." << endl;
}

