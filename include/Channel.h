#include "TH1F.h"
#include "TFile.h"
//#include "EventType.h"

#include <stdio.h>
#include <iostream>

using namespace std;

class Channel{
	public: 
		Channel(int i);
		~Channel();

		int GetNumBoundaries(){return NumHistoBoundaries;}
		void SetNumBoundaries(int i){NumHistoBoundaries = i;}
		void SetBoundary(int peakNumber, double value);
		double GetBoundary(int peakNumber){return HistoBoundaries[peakNumber];}
		void SetLowestBoundary(double d){LowestBoundary = d;}
		void Calibrate();
		double GetAveragePhotonNumber(){return AveragePhotonNumber;}
		int Return_pe_value(double integral, int k);
		string GetSiPMChannel(){return SiPMChannel;}
		int GetScopeChannel(){return ScopeChannel;}
		bool IsCalibrated();
		int CalculatePhotonNumber(double intgrl, int k);
		void SetDefaultCalibration(double m, double t);
		void SetCalibration(double m, double t);
		double CalculateRMS();
		double GetChannelBackground();
// Members:
		int ScopeChannel;
		string SiPMChannel;
		bool UseChannel;
		int NumHistoBoundaries;
		double HistoBoundaries[16];	// which is called Minima[][] in Analysis.cpp
		bool fBoundariesSet;
		bool fCalibrated;
		double slope;
		int pe_values[30000];
		double exact_pe_values[30000];
		double integral_values[4][30000] ;// [type][value]
		double m;
		double t;
		string calibrationFile;
		int entries;


	private:
		double LowestBoundary;	
		double AveragePhotonNumber;	
};
