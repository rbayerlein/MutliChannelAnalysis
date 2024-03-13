#include "TH1F.h"
#include "TFile.h"
#include "TH2F.h"
#include "TCanvas.h"

#include <stdio.h>
#include <iostream>

using namespace std;

class Graphix{
	public:
		Graphix();
		~Graphix();
		void Draw3D();
		void Draw3DFromFile(string f);
		int GetChipID(string SiPMChannel);	// returns number between 0 and 15 depending on SiPMChannel (A1, A2, ... , D4)
		void InitializeDrawing();

// Members
		double PhotonsPerChannel[16];
		TH2F *sipm_3d;
		TH2F *sipm_text;
		TFile * ROOTFile;
		string resFile; 	//<date>_ChannelResults.txt
		double max_num_Photons;
		double total_num_Photons;

	private: 
		void sipmdraw3d();
		int goodCounter;
		
};
