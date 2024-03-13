#include "TH1F.h"
#include "TFile.h"
//#include "EventType.h"

#include <stdio.h>
#include <iostream>

using namespace std;
class Channel;
class Calibration{
	public: 
		Calibration(Channel *passedChannelInstance);
		Calibration();
		~Calibration();

		void Calibrate();

// Getters and Setters
		double GetSlope(){return m;}
		double GetOffset(){return t;}
		
		bool IsCalibrated();


	Channel *h0;
//	TFile *TFile;			// for later use in the constructor FIXME
	private:
		void FindPeaks(vector<int>& x);
		void PerformCalibrationFIT();
		void UpdateCalibrationFile();
		void WriteCalibrationToFile();

		double peaks[64];		// stores the peak values of the histogram
		double peakPositionsFIT[64];	// the peaks found in FindPeaks(vector...) via gauss fits
		int numPeaks;
		double t;
		double m;
		double error;			// error on the slope
		string SiPMChannel;
		int ScopeChannel;	
		char histoName[1024];
		string calibrationFile;
		string timeStamp;
		TFile* ROOTFile;	
		double binWidth;
		int entries;			// num entries in the histogram
		double low;			//position of bin 0

};
