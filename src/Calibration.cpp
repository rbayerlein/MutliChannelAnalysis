#include "Channel.h"
#include "Calibration.h"
#include "RunControl.h"
#include "Analysis.h"
#include "TH1F.h"
#include "TF1.h"
#include "TGraph.h"
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdlib>

using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Constructor:

Calibration::Calibration(Channel *passedChannelInstance)
{
RunControl *RC = RunControl::GetInstance();
h0 = passedChannelInstance;
ROOTFile = RC->GetROOTFile();
SiPMChannel = h0->GetSiPMChannel();
ScopeChannel = h0->GetScopeChannel();
calibrationFile = RC->GetCalibrationFile();
timeStamp = RC->GetTimeStamp();
double binWidth = (RC->GetBinWidth())*1E-3;
sprintf(histoName, "%s%d%s%s", "h_corr_ch", ScopeChannel, "_", RC->SiPMChannels[ScopeChannel].c_str());	
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// alternative constructor:

Calibration::Calibration(){
	SiPMChannel = "C2";
	calibrationFile = "/home/bayerlein/Measurements/20180112_16_ch_chkv_measurement/Calibration_56.5V.txt";
	timeStamp = "newTimeStamp";
	ROOTFile = new TFile("/home/bayerlein/Measurements/20180112_16_ch_chkv_measurement/Analysis/2018-01-29-17-27_Results.root");
	sprintf(histoName, "%s", "h_corr_ch0_C2");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Destructor:

Calibration::~Calibration(){
	
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Calibration::Calibrate(){
	cout << "Calibrate channel " << SiPMChannel << endl; 

	TH1F * h = new TH1F("h_TEST", "h title", 100,0,4);
	h = (TH1F*)ROOTFile->Get(histoName);
//	h->Draw();
//	h->Write();	

// READ IN DATA

//	int *bins = new int[h->GetSize()];

	entries = h->GetEntries();
	low = h->GetXaxis()->GetBinLowEdge(0);


	cout << " lower boundary of the histogram:\t" << low << endl;
	cout << "Num entries in histo:\t" << h->GetEntries() << endl;
	vector<int> x;	
       	for (int i=0;i<h->GetSize();i++) {
//		bins[i] = h->GetBinContent(i);
		x.push_back(h->GetBinContent(i));
//		cout << i << "\t" << x[i] << endl;
	}
	FindPeaks(x);
	PerformCalibrationFIT();
//	UpdateCalibrationFile();	// changes values in the calibration file
	h0->SetCalibration(m, t);	// passes the calibration values to the channel
}	



//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Calibration::FindPeaks(vector<int> &x){
	RunControl *RC = RunControl::GetInstance();
	cout << "FindPeaks() " << endl;

	binWidth = RC->GetBinWidth()*1E-3;
	cout << "BinWidth [nVs] = " << binWidth << endl;

	int size = x.size();

	int peakPositions[64];

	int bound = 3; 		// boundary in which a maximum is counted as local maximum
	int it = bound;
	bool maxFound;
	int peakNumber = 0;

// Find maxima
	while(it < size-bound){
		maxFound = true;
		int currentValue = x.at(it);
		for(int j = it-bound; j < it+bound+1; j++){	// search for maximum in this interval
			if(j == it) continue;			// don't compare current value to current value
			else if(x.at(j) > currentValue){ 
				maxFound = false;
				break;
			}
			else if(x.at(j) == currentValue) {	// if they are identical they are allowed only if they are neighbours
				if ((j-it) == 1 || (j-it) == -1) {
//					cout << "neighbours detected" << endl;
					it++;			// to prevent double counting 
					continue;
				}
				else {maxFound = false; break;}
			}
		}

		if(maxFound){
			if(x.at(it)>0.01*entries){		// take only largest peaks (height > 1% of num_entries)
				cout << "max found at bin: " << it << "\t-> integral value " << it*binWidth+low << "\tentries: " << x.at(it) <<endl;
				peakPositions[peakNumber] = it;
				peakNumber++;
			}else if (x.at(it)>0.002*entries && peakNumber < 2){		// accept smaller than 1% if it's the first or second peak in the histogram
											// and if it's larger than 0.2% (cut's off noice before 0pe peak)
				cout << "max found at bin: " << it << "\t-> integral value " << it*binWidth+low << "\tentries: " << x.at(it) <<endl;
				peakPositions[peakNumber] = it;
				peakNumber++;
			}
		}
		it++;
	}
	cout << peakNumber << " peaks found. " << endl;

// Check, if FIRST detected peak is larger than 50% of the height of the neighbouring peak. if not kick it out.

/*	int largestPeak = 0;
	for (int ii = 0; ii < peakNumber; ii++){		// search for largest peak
		cout << ii << "\t" << x.at(peakPositions[ii]) << endl;
		if (largestPeak < x.at(peakPositions[ii]) ) largestPeak =  x.at(peakPositions[ii]);
	}
	cout << "largest peak value: " << largestPeak << endl;
*/
	cout << "Check, if first peak is larger than 25 percent of its next neighbour in the histogram... " << endl;
	int next_peak = x.at(peakPositions[1]);
	if (0.25*next_peak > x.at(peakPositions[0]) ){
		cout << "First peak is smaller than 25 percent of the next neighbouring peak value in the histogram and will be kicked out." << endl;
		for(int jj = 1; jj < peakNumber; jj++){
			peakPositions[jj-1] = peakPositions[jj];
		}
		numPeaks = peakNumber-1;
	}
	else{
		cout << "checked." << endl;
		numPeaks = peakNumber;					// copy into global variable
	}
	cout <<"Finally, " << numPeaks << " peaks were found and saved for fitting. " << endl;	

// Perform gauss fits to find mean value of peaks
	TH1F * h = new TH1F("h_TEST", "h title", 100,0,4);
	h = (TH1F*)ROOTFile->Get(histoName);

	double parameterArray[3][numPeaks];
	TF1 *f[numPeaks];
	int num_gauss_fits = 0;
	for (int i = 0; i < numPeaks; i++){
		char fit_name[128];
		sprintf(fit_name, "%s%d", "f", i);
		cout << "Fit boundaries: " << (peakPositions[i]-3)*binWidth+low << " -> " << (peakPositions[i]+3)*binWidth+low << endl;
		f[i] = new TF1(fit_name, "gaus", (peakPositions[i]-3)*binWidth+low , (peakPositions[i]+3)*binWidth+low);
		h->Fit(fit_name, "R+");
		for(int par=0; par < 3; par++){
			parameterArray[par][i] = f[i]->GetParameter(par);
			cout << "fit " << i <<":\tparameter " << par << "\t" << parameterArray[par][i] << endl;
		}

//		cout << i << "\t" << peakPositionsFIT[i] << endl;
		double mean_error =  f[i]->GetParError(1);
		double mean_error_in_bins = mean_error / binWidth; 
		cout << "fit error on mean value: " << mean_error << endl;
		cout << "fit error in units of bins: " << mean_error_in_bins << endl;
		if (i > 3 && mean_error_in_bins > ((2*bound+1)/2)) {		// only reject if there are at least 4 peaks available to perform a fit
										// use half the fitting range as rejection criterion 
			cout << "Error too large - Reject this point." << endl;
			break;
		}
		else{
			num_gauss_fits++;
			peakPositionsFIT[i] = parameterArray[1][i];
		}
	}
	cout << "Number of correct gauss fits to the peaks: " << num_gauss_fits << endl;
	cout << "Number of unfitted peaks due to high errors: " << numPeaks - num_gauss_fits << endl;
	numPeaks = num_gauss_fits;		// update number of peaks for the fitting of the calibration curve i.e. only take the ones with successful gauss fit.
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Calibration::PerformCalibrationFIT(){
	RunControl *RC = RunControl::GetInstance();
	// TFile *f_calib = new TFile("/home/bayerlein/Measurements/20180112_16_ch_chkv_measurement/Analysis/Test_for_Calibration.root", "recreate");

	cout << "Fiting linear slope to data to obtain calibration curve..." << endl;
	double pe[64] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};
	TGraph *g_cal = new TGraph(numPeaks, peakPositionsFIT, pe);
	char fName[1024];
	sprintf(fName, "%s%d%s%s", "f_", ScopeChannel, "_", RC->SiPMChannels[ScopeChannel].c_str());
	TF1 *cal_fit = new TF1(fName, "[1] * x + [0]"); 
	g_cal->Fit(cal_fit); 
	g_cal->SetTitle(fName);
	g_cal->GetXaxis()->SetTitle("Peak integral value [nVs]");
	g_cal->GetYaxis()->SetTitle("Photon Number");
	g_cal->Draw("AL");
//	g_cal->Draw("A*");
	g_cal->Write();
	t = cal_fit->GetParameter(0);
	m = cal_fit->GetParameter(1);
	error= cal_fit ->GetParError(1);
	cout << "slope: \t" << m << " +/- " << error << "\tOffset:\t" << t << endl;
/*
	cout << "Testing last peak: " << endl;
	cout << "pe value: " << numPeaks-1<< endl;
	double cal_pe = m*peakPositionsFIT[numPeaks-1] +t;
	cout << "calculated pe values: " << cal_pe << endl;
	double rel_dev = ((numPeaks-1)-cal_pe)/(numPeaks-1);		// relative deviation
	cout << "relative deviation " << rel_dev << endl;
	if (rel_dev < -0.1 || rel_dev > 0.1) {
		if(numPeaks>4){						// check if there are still enough points after rejecting (at least 4)
			cout << "reject this point." << endl;
		}	
	}
*/
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void Calibration::WriteCalibrationToFile(){
	cout << "Writing calibration to file..." << endl;
	//	res.open(resFileName.c_str(), std::ios_base::app| std::ios_base::out);
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Calibration::UpdateCalibrationFile(){
	RunControl *RC = RunControl::GetInstance();
	cout << "Updating Calibration File..." << endl;
	ifstream in_old(calibrationFile.c_str());

	stringstream ss;
	ss << RC->AnalysisFolder << RC->GetTimeStamp() << "_Calibration_" << RC->bias << "V.txt";
	string updatedCalib = ss.str();
	ofstream new_file(updatedCalib.c_str());
    if(!in_old || !new_file)
    {
        cout << "Error opening files!" << endl;
    }
	char line[1000];
	in_old.getline(line, 1000);
	cout << line << endl;
	new_file << line << endl;
	string SiPM;
	double m_file;
	double t_file;
	bool isCalibrated;
	string tS;
	int peaks;
	
	while(in_old >> SiPM >> m_file >> t_file >> isCalibrated >> tS >> peaks){
		
		if(SiPM == SiPMChannel){
			cout << "Found correct line. Replacing content... " << endl;
			isCalibrated = false;		//dont update the calibration file boolean flag any more. keep it false. (02.03.2018)
			m_file = m;
			t_file = t;
			tS = timeStamp;
			peaks = numPeaks;
			cout << SiPM << "\t" << m_file << "\t" << t_file << "\t" << isCalibrated << "\t" << tS <<"\t"<< peaks << endl;
			cout << " done." << endl;
		}
		new_file << SiPM << "\t" << m_file << "\t" << t_file << "\t" << isCalibrated << "\t" << tS <<"\t" << peaks << endl;
	}
	in_old.close();
	new_file.close();
	remove(calibrationFile.c_str());

/*	int result = rename("temp.txt", calibrationFile.c_str());
	if (result == 0) puts ("successfully updated calibration file:");
	else perror ("Error renaming file");
	cout << calibrationFile << endl;
*/

	std::ifstream  src(updatedCalib.c_str(), std::ios::binary);
	std::ofstream  dst(calibrationFile.c_str(), std::ios::binary);
	dst << src.rdbuf();

	remove(updatedCalib.c_str());
}






//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


bool Calibration::IsCalibrated(){
	cout << "Checking, if channel " << SiPMChannel << " is calibrated yet. " << endl;
	cout << "Opening " << calibrationFile << endl;
	ifstream in(calibrationFile.c_str());
	char line[1000];
	in.getline(line, 1000);

	string SiPM;
	double m_file;
	double t_file;
	bool isCalibrated;
	string tS;
	int peaks;
	
	while(in >> SiPM >> m_file >> t_file >> isCalibrated >> tS >> peaks){
		if (SiPM == SiPMChannel) {
			if(isCalibrated == true) {
				cout << m_file << "\t" << t_file << endl;
				in.close();
				return true;
			}
			else {in.close(); return false;}
		}
	}

	in.close();
	cout << "Channel not found. Wrong spelling?" << endl;
	return false;
	// returns true if calibrated and stores values 
	// otherwise simply returns false;
}


