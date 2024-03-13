#include "Analysis.h"
#include "RunControl.h"
#include "Channel.h"
#include "Calibration.h"
#include "Graphix.h"

#include <cmath>
#include <cstdlib>
#include <fstream>

using namespace std;


Analysis *Analysis::fInstance =0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Constructor:
Analysis::Analysis(){
	RunControl *RC = RunControl::GetInstance();
	badFilesCounter= 0;
	isPI_AnalysisSuccessful=false;
	total_num_Photons = 0;
	average_num_Photons = 0;
	total_num_Channels = 0;
	useArea = false;
	for(int i=0; i<16;i++){ AllPhotonNumbers[i] = -1; }

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Destructor:
Analysis::~Analysis(){

}


void Analysis::pe_Analysis(){
	RunControl *RC = RunControl::GetInstance();
	for(int ch = 0; ch<RC->num_channels; ch++){
		pe_Analysis(ch);
	}
	RC->WriteChannelResults();
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void Analysis::pe_Analysis(int ch){
	RunControl *RC = RunControl::GetInstance();

	cout << "Setting up channel " << ch << " for pe analysis..."<< endl;
	h0[ch] = RC->GetChannel(ch);		

	if(h0[ch]== NULL) {
		cout << "Channel " << ch << " does not exist. Please check RunControl::SetUpChannels(). Abort." << endl;	
		return;
	}
	if(!(h0[ch]->UseChannel)) {
		cout << ch << ": This channel was not used in the measurement. " << endl;
		return;
	}

	Calibration *cal = new Calibration(h0[ch]);

	// here ask for calibration
	string temp = "empty";
	while(temp != "c" && temp != "d"){
		cout << "Channel is not calibrated yet. \nType \"c\" to calibrate now. Type \"d\" to enter vales: " << endl;
		cin >> temp;
	}
	if(temp == "c") cal->Calibrate();
	else {
		double entered_value[2];
		for(int i=0; i<2; i++){
			std::string theInput;
			double inputAsDouble;
			if (i==0) cout << "Enter slope m: ";
			else cout << "Enter offset t: ";
			cin >> theInput;

			while(std::cin.fail() || std::cin.eof() || theInput.find_first_not_of(".-0123456789") != std::string::npos) {

				std::cout << "Error" << std::endl;

				if( theInput.find_first_not_of(".-0123456789") == std::string::npos) {
					std::cin.clear();
					std::cin.ignore(256,'\n');
				}
				cout << "Set value as type double: ";
				cin >> theInput;
			}

			std::string::size_type st;
			inputAsDouble = std::atof(theInput.c_str());
			entered_value[i] = inputAsDouble;
		}
		cout << "entered values: \tm = " << entered_value[0] << "\tt = " << entered_value[1] << endl;
		h0[ch]->SetCalibration(entered_value[0], entered_value[1]);
	}	
	
/*
	if ( !(h0[ch]->IsCalibrated()) ) {
		string temp = "empty";
		while(temp != "c" && temp != "d"){
			cout << "Channel is not calibrated yet. \nType \"c\" to calibrate now. Type \"d\" to use default vales (m = " << RC->default_m << " and t = " << RC->default_t << ")" << endl;
			cin >> temp;
		}
		if(temp == "c") cal->Calibrate();
		else {
			h0[ch]->SetDefaultCalibration(RC->default_m, RC->default_t);
		}
	}
*/
	cout << "Starting pe analysis..." << endl;

	for(int k = 0; k < RC->num_Files; k++){
		if(integral_values[ch][3][k] < (RC-> sigma_correction_boundary) ) {
			pe_values[ch][k]=h0[ch]->CalculatePhotonNumber(integral_values[ch][1][k], k); 
		}
//		cout << "pe level for ch " << ch << " wfm " << k << ":\t"<< pe_values[ch][k] << "\tcalculated: " << h0[ch]->exact_pe_values[k] << endl;
//		cout << "pe level for ch " << ch << " wfm " << k << ":\t" << h0[ch]->CalculatePhotonNumber(integral_values[ch][1][k], k) << endl;
	}
/*
	for(int k = 0; k < RC->num_Files; k++){
	pe_values[ch][k] = h0[ch]->Return_pe_value(integral_values[ch][1][k], k);
	cout << "pe level for ch " << ch << " wfm " << k << ":\t"<< pe_values[ch][k] << "\tcalculated: " << h0[ch]->exact_pe_values[k] << endl;
	}
*/
// Save photon number in global array
	
	Graphix *grx = new Graphix();
	int ID = grx->GetChipID(h0[ch]->GetSiPMChannel());
	if( ID != -1) {
		AddPhotonNumber(ID, h0[ch]->GetAveragePhotonNumber());
	}

	cout << "Average photon number for that channel: " << h0[ch]->GetAveragePhotonNumber() << endl;
	cout << "BG subtraction: -" << h0[ch]->GetChannelBackground() << endl;
	cout << "--------------------------------------------------------" << endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::AddPhotonNumber(int ID, double d){

	if(AllPhotonNumbers[ID] != -1){					// case: Channel has a value already.
		total_num_Photons = total_num_Photons - AllPhotonNumbers[ID] + d;	// update total num photons
		average_num_Photons = ((average_num_Photons*total_num_Channels) - AllPhotonNumbers[ID] + d)/total_num_Channels;
											// update total average value
		AllPhotonNumbers[ID] = d;						// overwrite existing value.
		
	}else{								// case: Channel does not have a value yet.
		AllPhotonNumbers[ID] = d;						// write value.
		total_num_Photons +=d;
		average_num_Photons = ((average_num_Photons*total_num_Channels)+d)/(total_num_Channels+1);
		total_num_Channels++;
	}
	
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::PI_Analysis(){

	cout << "Starting integral calculation ..." << endl;
	cout << "Reset badFilesCounter." << endl;
	isPI_AnalysisSuccessful=false;
	badFilesCounter = 0;
	RunControl *RC = RunControl::GetInstance();
	int ch = 0;
	while (ch < RC->num_channels){
		if(RC->UseChannel[ch]){
			cout << "Start integrating channel " << ch << " ...\nPercentage processed:" << endl;	
			for(int k = 0; k < RC->num_Files; k++){ 

				if(useArea) GetIntegralFromFile(ch, k);
				else CalculateIntegral(ch, k);

				if (badFilesCounter > 9){
				RC->End();
				return; 		//apparently returns to function from where it was invoked
				}
//				cout << "\t..." << k+1 << "\tPulseIntegral:\t" << integral_values[ch][0][k] << endl;	
//				cout << "\tbaseline corrected integral: " << integral_values[ch][1][k] << endl;
//				cout << "\tbaseline mean value: " << integral_values[ch][2][k] << endl;
//				cout << "\tbaseline sigma value: " << integral_values[ch][3][k] << endl;
	      			if (k % 100 == 0) cout<< "\t" << (double)k/RC->num_Files*100 << "\t" << "%" << endl;
			}
				cout << "\t" << 100 << "\t" << "%" << endl;
			PassIntegralToChannel(ch);
			cout << "Successfully passed the integrals to channel " << ch << "." << endl;		
		} //close if()
		ch++;
	} // close while()
	cout << "... finished integral calculation " << endl;
	Write_PI_Histos();
	isPI_AnalysisSuccessful=true;
}// close function

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::PassIntegralToChannel(int ch){
	RunControl *RC = RunControl::GetInstance();
	if(!(RC->GetChannel(ch)->UseChannel)) return;

	for(int k = 0; k < RC->num_Files; k++){
		for(int i = 0; i<4; i++){
			RC->GetChannel(ch)->integral_values[i][k] = integral_values[ch][i][k];
		}
	}
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::Write_PI_Histos(){
	cout << "Creating and writing histograms to file...." << endl;
	RunControl *RC = RunControl::GetInstance();
	TFile *results = RC->GetROOTFile();
	TH1F *PI_histos[RC->num_channels]; 
	TH1F *corrected_PI_histos[RC->num_channels];
	TH1F *BL_std_dev_histos[RC->num_channels]; 	//histogramm for the base line sigma values a.k.a. standard deviation
	TH1F *rejected_wfms_histos[RC->num_channels];	//rejected waveforms based on standard deviation calculation

	for (int ch = 0; ch < RC->num_channels; ch++){
		if(RC->UseChannel[ch]){
			char temp2[1024];
			sprintf(temp2, "%s%d%s%s", "h_corr_ch", ch, "_", RC->SiPMChannels[ch].c_str());
			corrected_PI_histos[ch] = new TH1F(temp2, temp2, RC->num_Bins, RC->histo_low, RC->histo_high);

			if(!useArea){
				char temp[1024];
				sprintf(temp, "%s%d%s%s", "h_ch", ch, "_", RC->SiPMChannels[ch].c_str());
				char temp3[1024]; 
				sprintf(temp3, "%s%d%s%s", "h_std_dev_ch", ch, "_", RC->SiPMChannels[ch].c_str());
				char temp4[1024];
				sprintf(temp4, "%s%d%s%s", "h_rejctd_ch", ch, "_", RC->SiPMChannels[ch].c_str());

				PI_histos[ch] = new TH1F(temp, temp, RC->num_Bins, RC->histo_low, RC->histo_high);
				BL_std_dev_histos[ch] = new TH1F(temp3, temp3, 250, 0, 1);	
				rejected_wfms_histos[ch] = new TH1F(temp4, temp4, RC->num_Bins, RC->histo_low, RC->histo_high);
			}
	
			if (useArea){
				for(int k = 0; k < RC->num_Files; k++){
					corrected_PI_histos[ch]->Fill(integral_values[ch][1][k]);
				}
			}else {
				for(int k = 0; k < RC->num_Files; k++){
					PI_histos[ch]->Fill(integral_values[ch][1][k]);
					BL_std_dev_histos[ch]->Fill(integral_values[ch][3][k]);
					if(integral_values[ch][3][k] < (RC-> sigma_correction_boundary) ) corrected_PI_histos[ch]->Fill(integral_values[ch][1][k]);
					else rejected_wfms_histos[ch]->Fill(integral_values[ch][1][k]);
				}
			}
			corrected_PI_histos[ch]->Write();
			
			if(!useArea){
				PI_histos[ch]->Write();	
				BL_std_dev_histos[ch]->Write();
				rejected_wfms_histos[ch]->Write();
			}
		}
	}
	cout << "...done writing." << endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::AnalysePeakTimes(){
	cout << "Starting peak time analysis..." << endl;
	cout << "Reset badFilesCounter." << endl;
	isPI_AnalysisSuccessful=false;
	badFilesCounter = 0;

	RunControl *RC = RunControl::GetInstance();
	TFile *results = RC->GetROOTFile();
	TH1F *peakTimes = new TH1F("pT", "Peak Times", 500, -10000, 10000);		// bin width of 40 ps
	for(int ch = 0; ch<3; ch++){
		if(!(RC->GetChannel(ch)->UseChannel)) {
			cout << ch << ": This channel was not used in the measurement. " << endl;
			continue ;
		}
		for(int k= 0; k < RC->num_Files; k++){
			double pT = GetPeakTimeFromFile(ch, k) ;
			cout << "Peak time channel " << ch << " number " << k << ":\t" << pT << endl;
			peakTimes->Fill(pT);
		}
	}
	peakTimes->Write();
	isPI_AnalysisSuccessful=true;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double Analysis::GetPeakTimeFromFile(int ch, int k){
	if (ch > 2){cout << "unvalid channel." << endl; return -1;}
	RunControl *RC = RunControl::GetInstance();

	cout << "Getting peak time from file for channel " << ch << " and file # " << k+1 << endl;

	string data = "/data_";
	string extension = "Meas.txt";
	stringstream ss;
	ss << RC->globalPath << RC->folderName << data << k+1 << extension;
	string temp = ss.str();
	ifstream in(temp.c_str());
//	cout << temp << endl;
	if(!in) {
		cout << "could not open file!" << endl;
		cout << temp.c_str() << endl;
		badFilesCounter++;					
		return -1;
	}

	string coi; 			// channel of interest (coi)
	if(ch == 0) coi = "Delay(Ch4,Ch1)";
	else if(ch == 1 ) coi = "Delay(Ch4,Ch2)";
	else if(ch == 2 ) coi = "Delay(Ch4,Ch3)";

	char dummy[512];
	in.getline(dummy, 512);

	string a = "empty";
	string delays_raw[8];
	string delay_raw;
	string b;
	string c;
	string d;
	string e;
	string f;

	bool flag = false;
	for( int i =0 ; i<8; i++){
		in >> a >> delays_raw[i] >> b >> c >> d >> e >> f;
//			cout << a << "\t" << delays_raw[i]<< "\t" << b << "\t" << c << "\t" <<  d << "\t" <<  e << "\t" <<  f << endl;
		if(a == coi){
			delay_raw = delays_raw[i];
			flag = true; 
			break;
		}
	}
	if(!flag){
		cout << "Couldn't find correct channel in measurements-file." << endl;
		badFilesCounter++;
		return -1;
	}

//		cout << "delay_raw: " << delay_raw << endl;
	string s = "s";
	size_t found = delay_raw.find(s);
	double ten_pot;
	char n[8] = "n";
	if(delay_raw[found-1] == n[0]) ten_pot = 1000;
	else ten_pot = 1;
	char delay_cut[8];
	for(unsigned int j=0; j < found-1; j++){
		delay_cut[j] = delay_raw[j];
	}
//		cout << "delay_cut: " << delay_cut << endl;
	double delay_d = atof(delay_cut);
//	cout << k << ":\t" << delay_d*ten_pot << "\tps" <<endl;
//	cout << "---------------------" <<endl;

	in.close();
	return delay_d*ten_pot;

}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void Analysis::GetIntegralFromFile(int ch, int k){
	RunControl *RC = RunControl::GetInstance();
//	cout << "Getting integral from file for channel " << ch << " and file # " << k << endl;

	string data = "/data_";
	string extension = "Meas.txt";
	stringstream ss;
	ss << RC->globalPath << RC->folderName << data << k+1 << extension;
	string temp = ss.str();
	ifstream in(temp.c_str());
//	cout << temp << endl;
	if(!in) {
		cout << "could not open file!" << endl;
		cout << temp.c_str() << endl;
		badFilesCounter++;					
		return ;
	}

	string coi; 			// channel of interest (coi)
	if(ch == 0) coi = "Area(Ch1)";
	else if(ch == 1 ) coi = "Area(Ch2)";
	else if(ch == 2 ) coi = "Area(Ch3)";
	else if(ch == 3 ) coi = "Area(Ch4)";

	char dummy[512];
	in.getline(dummy, 512);

		string a = "empty";
		string areas_raw[4];
		string area_raw;
		string b;
		string c;
		string d;
		string e;
		string f;
		bool flag = false;
		for( int i =0 ; i<4; i++){
			in >> a >> areas_raw[i] >> b >> c >> d >> e >> f;
//			cout << a << "\t" << areas_raw[i]<< "\t" << b << "\t" << c << "\t" <<  d << "\t" <<  e << "\t" <<  f << endl;
			if(a == coi){
				area_raw = areas_raw[i]; 
				flag = true; 
				break;
			}
		}
		if(!flag){
			cout << "Couldn't find correct channel in area-file." << endl;
			badFilesCounter++;
			return;
		}
//		cout << "area_raw: " << area_raw << endl;
		string Vs = "Vs";
		size_t found = area_raw.find(Vs);
		double ten_pot;
		char n[8] = "n";
		if(area_raw[found-1] == n[0]) ten_pot = 1000;
		else ten_pot = 1;
		char area_cut[8];
		for(unsigned int j=0; j < found-1; j++){
			area_cut[j] = area_raw[j];
		}

		double area_d = atof(area_cut);
//		cout << k << ":\t" << area_d*ten_pot << "\tpVs" <<endl;
//		cout << "---------------------" <<endl;
		integral_values[ch][1][k] = (area_d*ten_pot)/1000 ;
		integral_values[ch][0][k] = (area_d*ten_pot)/1000 ;
		integral_values[ch][2][k] = 0;
		integral_values[ch][3][k] = 0;
	in.close();

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::CalculateIntegral(int ch, int k){
	
	RunControl* RC = RunControl::GetInstance();
//OPEN FILE
	ifstream in;
	in.open(RC->fileList[ch][k].c_str());
	if(!in){ 
		cout << "-------------------------------------------------------------" << endl;
		cout << "PulseIntegral for file \n"<<RC->fileList[ch][k]<< endl;
		cout << "Could not open file. Abort." << endl; 
		badFilesCounter++;					
		return ;				//apparently returns to function from where it was invoked
	}
//		cout << "-------------------------------------------------------------" << endl;
//		cout << "PulseIntegral for file \n"<< RC->fileList[ch][k] << endl;	
//SKIP FIRST LINES
	char line[1000];
	for(int j=1;j<6+1;j++){
		in.getline(line,1000);		
	}

//Read in data and calculate integral
	double t;
	double V;
	int lineCounter=1;		// counter for voltage values that are processed 
	int baseLine_LineCounter =1;	// counter for voltage values that contribute to base line correction
	double integral_V=0;
	double baseLine_Values[1000];	// store base line values in an array to calculate the sigma afterwards
	double baseLine_Mean = 0;
	double baseLine_Sigma = 0;
	double firstTimeStamp = 0;	// first time stamp in data set
	double timeBeforeTrigger_s = RC->timeBeforeTrigger*1e-9;	
					// 	calculate the time before trigger in seconds once so that it doesn't 
					//	have to be calculated in every loop and every if-condition
	double triggerOffset_s = RC->triggerOffset*1e-9;
	double recordLength_s = RC->recordLength*1e-9;

	do {
	in >> t >> V;
	}
	while (t < (-timeBeforeTrigger_s));
// --
	do {
	in >> t >> V;
		baseLine_Mean += V;
		baseLine_Values[baseLine_LineCounter-1] = V;	// measured in V
		baseLine_LineCounter++;
	}
	while(t < 0+triggerOffset_s); 	// this specific point in time is excluded from BL correction
					// here the integration starts
// --
	while (!in.eof() && t < recordLength_s){
	in >> t >> V;
		integral_V += V;
		lineCounter++;	
	}
// --
	if(!in.eof()) {		//add the last one
		in >> t >> V;
		integral_V += V;
		lineCounter++;			
	}						

	baseLine_Mean=(baseLine_Mean/baseLine_LineCounter);	// mean value of base line in Volt
	integral_values[ch][2][k] = baseLine_Mean*1000;		// in mV

	for(int i = 0; i < baseLine_LineCounter ; i++){
		baseLine_Sigma += pow((baseLine_Values[i]-baseLine_Mean),2);
	}
	baseLine_Sigma = sqrt(baseLine_Sigma/baseLine_LineCounter);		// in Volt
	integral_values[ch][3][k] = baseLine_Sigma*1000;			// in mV

//WRITE CORRECTED INTEGRAL
	integral_values[ch][1][k] = (integral_V-(baseLine_Mean*lineCounter))*RC->timePerStep/1000;

//WRITE INTEGRAL
	integral_V *= RC->timePerStep/1000; 	// in nVs
	integral_values[ch][0][k] = integral_V;

//CLOSE FILE
	in.close();

}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::PhotonSpectrum(){
	cout << "Creating a photon spectrum of the selected channels in this run..." << endl;
	RunControl *RC = RunControl::GetInstance();
	TFile *results = RC->GetROOTFile();

	stringstream ss;
	ss << "spectrum";
	for(int ch = 0; ch < RC->num_channels; ch++){
		if(h0[ch]->UseChannel) ss << "_" << RC->GetSiPMChannel(ch);
	}
	string s = ss.str();

	TH1F *photon_spectrum = new TH1F(s.c_str(), s.c_str(), 110, -10, 100);
	for(int k = 0; k < RC->num_Files; k++){
		int i_photons=0;
		for (int ch = 0; ch < RC-> num_channels; ch++){
			if (h0[ch]->UseChannel){
				i_photons += h0[ch]->pe_values[k];
			}
		}
		photon_spectrum->Fill(i_photons);
	}
	photon_spectrum->Write();
	cout << "done." << endl;
}




