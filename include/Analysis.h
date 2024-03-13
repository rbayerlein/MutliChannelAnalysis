#include "TH1F.h"
#include "TFile.h"
//#include "EventType.h"

#include <stdio.h>
#include <iostream>

//#include "RunControl.h"

using namespace std;
class Channel;
class Analysis{
	public:
		static Analysis *GetInstance(){	// makes sure only one instance of this class can be created
			if(Analysis::fInstance == NULL){
				Analysis::fInstance = new Analysis;
			}
			return fInstance;
		
		}
		~Analysis();	// Destructor
		void pe_Analysis(int ch);
		void pe_Analysis();
		void DefineMinima();
		void PI_Analysis();
		double integral_values[4][4][30000];	// [channel][value_type][k]
		int pe_values[4][30000];
		double Minima[4][20];			// [channel][value]
		int badFilesCounter;
		Channel* h0[4];
		bool isPI_AnalysisSuccessful;
//		void SaveTotalNumPhotons(double in){total_num_Photons = in;}
//		void SaveTotalNumChannels(int in){total_num_Channels = in;}
		double GetTotalNumPhotons(){return total_num_Photons;}
		int GetTotalNumChannels(){return total_num_Channels;}			
		double GetAvNumPhotons(){return average_num_Photons;}			
		void PhotonSpectrum();
		void SetUseArea(bool b){useArea = b;}
		void GetIntegralFromFile(int ch, int k);
		void AnalysePeakTimes();
		double GetPeakTimeFromFile(int ch, int k);

	private:
		Analysis();		// private constructor
		void CalculateIntegral(int ch, int k);
		void PassIntegralToChannel(int ch);
		void Write_PI_Histos();
		void AddPhotonNumber(int ID, double d);		// adds num photons to channel and updated total num photons and av num photons and total num channels
		static Analysis* fInstance;
		double total_num_Photons;
		double average_num_Photons;
		int total_num_Channels;
		bool useArea;
		double AllPhotonNumbers[16];

};
