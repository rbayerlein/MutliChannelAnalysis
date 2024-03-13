#include "Channel.h"
#include "RunControl.h"
#include "Analysis.h"
#include "Graphix.h"

#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <cmath>
#include <cstdlib>

using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Constructor:
Channel::Channel(int i)
: ScopeChannel(i)
{
	RunControl *RC = RunControl::GetInstance();
	SiPMChannel = RC-> SiPMChannels[i];
	UseChannel = RC-> UseChannel[i];
	NumHistoBoundaries = 0;			//this counts the number of boundaries between individual pe-peaks that were passed to this specific histogram
	LowestBoundary = 0;
	fBoundariesSet = false;
	fCalibrated= false;
	slope = -1;
	m=-1;
	t=-1;
	AveragePhotonNumber = 0;
	calibrationFile = RC->GetCalibrationFile();
	entries = RC->num_Files;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Destructor:

Channel::~Channel(){
	
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

bool Channel::IsCalibrated(){
	cout << "Checking, if channel " << SiPMChannel << " already has a valid calibration. " << endl;
	ifstream in(calibrationFile.c_str());
	if(!in) cout << "error opening file " << calibrationFile.c_str() << endl;
	string SiPM;
	double m_file;
	double t_file;
	bool isCalibrated;
	string tS;
	int peaks;

	char line[1000];
	in.getline(line, 1000);

	while(in >> SiPM >> m_file >> t_file >> isCalibrated >> tS >>peaks){
		if (SiPM == SiPMChannel) {
			if(isCalibrated == true){ 
				m=m_file;
				t=t_file;
				fCalibrated = true;
				cout << "Calibrated." << endl;
				return true;
			}
			else return false;
		}
	}
	cout << "Channel not found. Wrong spelling?" << endl;
	return false;

//	in.close();
	// returns true if calibrated and stores values 
	// otherwise simply returns false;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void Channel::SetDefaultCalibration(double m_in, double t_in){
	m= m_in;
	t= t_in;
	fCalibrated = true;
}
void Channel::SetCalibration(double m_in, double t_in){
	m= m_in;
	t= t_in;
	fCalibrated = true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int Channel::CalculatePhotonNumber(double intgrl, int k){

	if(!fCalibrated) {
		cout << "Channel not calibrated!" << endl;
		return -1;
	}
	double i_photons;
	double d_photons = intgrl*m+t;
	int d_cutOff = (int)d_photons;

// round up or down to get integer photon number
	if(d_photons-d_cutOff <0.5){
		
		i_photons = d_cutOff;
	}
	else{
		i_photons = (d_cutOff+1);
	}

	i_photons = i_photons-GetChannelBackground();			//subtract BG
	if (i_photons < 0 ) i_photons = 0;

	pe_values[k] = i_photons;
	exact_pe_values[k] = d_photons-GetChannelBackground();		//subtract BG
	AveragePhotonNumber=((AveragePhotonNumber*k)+i_photons)/(k+1);
	if (i_photons < 0 ) return 0;
	return i_photons;
}

double Channel::CalculateRMS(){
	double RMS = 0;
	for(int k = 0; k < entries; k++){
		RMS += pow((pe_values[k]-AveragePhotonNumber),2);
	}
	RMS = sqrt(RMS/(entries-1));
	return RMS;
}


double Channel::GetChannelBackground(){
	RunControl *RC = RunControl::GetInstance();
	if (!RC->CorrectBG) return 0;
	Graphix* grx = new Graphix(); 
	double BG[16] = {0.2105, 0.2096, 0.2139, 0.2441, 0.2663, 0.169, 0.1961, 0.3026, 0.2094, 0.2392, 0.1931, 0.2456, 0.221, 0.3101, 0.2183, 0.276};
	return BG[grx->GetChipID(SiPMChannel)];
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


