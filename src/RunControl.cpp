#include "RunControl.h"
#include "Analysis.h"
#include "Channel.h"
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <cmath>
#include <cstdlib>

using namespace std;

RunControl *RunControl::fInstance =0;
	Channel *h0;
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Constructor:
RunControl::RunControl(){
					// Enter all parameters here!
// globalPath = "/data/compton1/Measurements/20180112_16_ch_chkv_measurement/Run_1/";
// globalPath = "/home/bayerlein/Measurements/20180213_Hedia_Rey_Test/";
 globalPath = "/home/bayerlein/Measurements/20180313_PeakTimeMeasurement/";
 AnalysisFolder = "/home/bayerlein/Measurements/20180213_Hedia_Rey_Test/Analysis/";
 calibrationFile = "/home/bayerlein/Measurements/20180213_Hedia_Rey_Test/Calibration_56.5V_Amp.txt";
 folderName = "Run_1";
 inputFileName = "data_";		// everything that comes before "_wmf.txt"
 bias = 56.5;
 prefix = "Run_1";			// used for creating the file name
 num_Files = 100;
 num_channels = 4;			// num of channels that were saved in this Run
 num_runs = 0;				// counts the number of runs 
 SiPMChannels[0] = "C2";
 SiPMChannels[1] = "C3";
 SiPMChannels[2] = "B2";
 SiPMChannels[3] = "B3";
 UseChannel[0] = true;
 UseChannel[1] = true;
 UseChannel[2] = true;
 UseChannel[3] = true;

 timePerStep = 320; 			// in pico seconds ( = timing resolution)

 binWidth = 4*50;	// Old 4		// in Volt*ps
 num_Bins = 400;
 stepsPerBin = 25;			// will be calculated in method AdjustBinWidth(double blabla);

 histo_low = -0.2*50;			// lower histogram boundary
 histo_high ;				// higher histogram boundary
	
 recordLength = 200; 			// in ns
 timeBeforeTrigger = 100;		// delay after which calculation starts [in ns]. I.e. skips first xxx ns of data
 triggerOffset = -10;			// shift trigger artificially, so that BL correction does not stop at t=0 but before or after that. [in ns]
 voltageResolution = 0.2;		// in mV
 sigma_correction_boundary = 0.5*50;	// in mV

 correctParameters = true;

 default_m = 32.0;
 default_t = 0.38;

 useArea = false;
 CorrectBG = false; 	

}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Destructor:
RunControl::~RunControl(){	
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::PrepareNewRun(){
	cout << "Preparing new run..." << endl;

// Overwrite parameters

	cout << "Enter folder name (Without the slash!): ";
	cin >> folderName ;
	cout << "Set prefix: (usually same as folderName): ";
	cin >> prefix ;
	std::string theInput;
    	int inputAsInt;
	cout << "Set number of files: ";
	cin >> theInput;

    	while(std::cin.fail() || std::cin.eof() || theInput.find_first_not_of("0123456789") != std::string::npos) {

       		std::cout << "Error" << std::endl;

		if( theInput.find_first_not_of("0123456789") == std::string::npos) {
			std::cin.clear();
			std::cin.ignore(256,'\n');
		}
		cout << "Set number of files: ";
		cin >> theInput;
	}

	std::string::size_type st;
	inputAsInt = std::atoi(theInput.c_str());
//	std::cout << inputAsInt << std::endl;
	num_Files = inputAsInt;

//	cin >> num_Files ;
	cout << "Assign the correct SiPM channel to Scope channels. (If a channel is not used type nA) " <<endl ;
	cout << "1: " ;
	cin >>  SiPMChannels[0];
	cout << "2: " ;
	cin >> SiPMChannels[1];
	cout << "3: " ;
	cin >>  SiPMChannels[2];
	cout << "4: " ;
	cin >> SiPMChannels[3];
	cout << "State if the channel should be used for analysis as well (Y/n) " << endl ;
	
	for(int ch = 0; ch < num_channels; ch++){
		string temp = "empty";		
		if(SiPMChannels[ch] != "nA") {	
			do {
			cout << SiPMChannels[ch] <<  ": ";
			cin >> temp;
			}
			while (temp != "Y" && temp != "n");
		}
		if(temp == "Y") UseChannel[ch] = true;
		else UseChannel[ch] = false;
	}

	cout << "Here are your settings: " << endl;
	cout << "Folder: " << folderName << "\n" << "Prefix: " << prefix << "\n" << "num files: " << num_Files << endl;
	for (int i= 0; i < num_channels ; i++){cout << "Scope channel " << i << " -> SiPM channel " << SiPMChannels[i] << "\t" << UseChannel[i] << endl;}
	cout << "Please confirm your settings:" << endl;
	string yesNo = "empty";
	while (yesNo != "Y" && yesNo != "n"){
		cout << "Press \"Y\" if correct. Otherwise press \"n\"." ;
		cin >> yesNo ;
	}
	if (yesNo != "Y"){
		PrepareNewRun();
	} 

//SaveAllChannels();
SetUpChannels();
CreateFileList();
num_runs++;

// Let User doublecheck in the end (recursively)!
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::SaveAllChannels(){
//Save previous channels in channel list.
	int ch = 0;
	while(allChannels[ch] != NULL && ch>31) ch++;
	if (ch > 31){
		cout << "Array of old Channels has too many entries (32). No more can be saved." << endl;
		return;
	}
	int num_entries=ch+num_channels;
	int h = 0;
	for(ch; ch<num_entries; ch++){
		allChannels[ch] = h0[h];
		h++;
	}
}

void RunControl::SetUpChannels(){

 h0[0]= new Channel(0);
 h0[1]= new Channel(1);
 h0[2]= new Channel(2);
 h0[3]= new Channel(3);

 SaveAllChannels();
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::SetParameters(){
	cout << "Set Parameters(): "<< endl;

cout << "----------------------------------" << endl;
	cout << "There are default values stored in the MACRO. Use them? " << endl;
	string yesNo_2 = "empty";
	while(yesNo_2 != "Y" && yesNo_2 != "n" && yesNo_2 != "A"){
		cout << "Type Y/n (or press \"A\" to abort): " ;
		cin >> yesNo_2 ;
	}
	if(yesNo_2 == "A"){
		correctParameters = false;
		return;
	}

	if(yesNo_2 == "Y") {
		binWidth = AdjustBinWidth(binWidth);
		CalculateUpperBoundary();		
		PrintParameters();

		string yesNo_3 = "emptyy";
		while (yesNo_3 != "Y" && yesNo_3 != "n" && yesNo_3 != "A"){
			cout << "If these are correct settings, press \"Y\". Otherwise press \"n\" (Press \"A\" to abort) " ;
			cin >> yesNo_3 ;
		}
		if (yesNo_3 == "A") {
			correctParameters = false;
			return;
		}
		if (yesNo_3 != "Y"){
			cout << "Correct your values now, please: "<< endl;
			SetParameters();		// recursive invocation this method
		}
		return;
	}
	cout << "----------------------------------" << endl;

	string yesNo = "empty";
	cout << "Use default folder location?\n" << "Path: " << globalPath << endl;
	while(yesNo != "Y" && yesNo != "n" && yesNo != "A"){
		cout << "Type Y/n/A: " ;
		cin >> yesNo ;
	}
	if(yesNo == "n"){
		cout << "Enter folder location :";
		cin >> globalPath ;
	}else if (yesNo == "A"){
		correctParameters = false;
		return;
	}
	
	CreateCorrectFolderPaths();

	cout << "----------------------------------\nType in the following parameters:" << endl;

//	cout << "Folder name of current run: ";
//	cin >> folderName ;						// will be done in PrepareNewRun() FIXME
	cout << "Input file name (prefix entered in scope, e.g. \"data_\" etc): ";
	cin >> inputFileName ;
	cout << "----------------------------------" << endl;
//	cout << "Set output file name prefix (e.g. Run_#) : ";
//	cin >> prefix ;							// will be done in PrepareNewRun() FIXME
//	cout << "----------------------------------" << endl;
//	cout << "Number of files: ";					// will be done in PrepareNewRun() FIXME
//	cin >> num_Files ;
//	cout << "----------------------------------" << endl;

	if(!useArea){
		cout << "Time per step (in pico seconds): " ;
		cin >> timePerStep ;
		cout << "----------------------------------" << endl;	
		cout << "Time before trigger for baseline correction (in nano seconds): " ;
		cin >> timeBeforeTrigger ;
		cout << "----------------------------------" << endl;	
		cout << "Record length to be processed (in nano seconds): " ;
		cin >> recordLength ;
		cout << "----------------------------------" << endl;	
		cout << "Trigger Offset (in nano seconds - Use this to make BL correction end before/after t=0): " ;
		cin >> triggerOffset ;
		cout << "----------------------------------" << endl;
		cout << "Boundary for BL correction with sigma rejection :" ;
		cin >> sigma_correction_boundary ;	
	}

	cout << "Bin width [V*ps] - Adjustments will be made automatically if neccessary\nReasonable value without amplifier (using the amplifier): 4 (200): " ;
	cin >> binWidth ;
	binWidth = AdjustBinWidth(binWidth);		
	cout << "----------------------------------" << endl;	

	cout << "Number of bins: " ;
	cin >> num_Bins ;
	CalculateUpperBoundary();			
	cout << "----------------------------------" << endl;

//	cout << "Horizontal Offset (in Volts e.g. 3.008e-6): " ;
//	cin >> horizontalOffset ;
//	cout << "----------------------------------" << endl;

	
	PrintParameters();

	string yesNo_3 = "empty";
	cout << "Please check your settings:" << endl;
	while (yesNo_3 != "Y" && yesNo_3 != "n" && yesNo_3 != "A"){
		cout << "Press \"Y\" if correct. Otherwise press \"n\". Press \"A\" to abort: " ;
		cin >> yesNo_3 ;
	}

	if (yesNo_3 == "A") {
		correctParameters = false;
		return;
	}
	if (yesNo_3 != "Y"){
		SetParameters();
	}
	cout << "----------------------------------" << endl;
//	SetCalibrationFile();		// Only needed, if an existing calibration file is needed, which turned out to be not practical (07.03.2018)

}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

string RunControl::GetTimeStamp(){
	string s;
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	int *timeArray;
	timeArray = new int[5];
	timeArray[0] = now->tm_year + 1900;
	timeArray[1] = now->tm_mon +1;
	timeArray[2] = now->tm_mday;
	timeArray[3] = now->tm_hour;
	timeArray[4] = now->tm_min;
	
	stringstream ss;
	for(int i=0; i<4; i++){
		if(timeArray[i]<10){
			ss << "0" << timeArray[i] << "-";
		}else{
			ss << timeArray[i] <<"-";
		}
	}
	if(timeArray[4]<10) ss << "0" << timeArray[4] ;
	else ss << timeArray[4] ;
	s = ss.str();
	return s;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::CreateROOTFile(){			//Creates ROOT file for all runs
	CreateAnalysisDirectory();
	TFileName;
	stringstream ss;
	timeStamp = GetTimeStamp();
	ss << AnalysisFolder << timeStamp << "_Results.root";
	TFileName = ss.str();
	cout <<"ROOT file: "<< TFileName << endl;
	results = new TFile(TFileName.c_str(), "recreate");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::CreateIndividualROOTFile(){		//Creates a new ROOT file for the current run number
	CreateAnalysisDirectory();
	TFileName;
	stringstream ss;
	ss << AnalysisFolder << prefix << ".root";
	TFileName = ss.str();
	cout <<"ROOT file: "<< TFileName << endl;
	results = new TFile(TFileName.c_str(), "recreate");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::CreateAnalysisDirectory(){
	stringstream ss;
	ss << globalPath << "Analysis/" ;
	AnalysisFolder = ss.str();
	if (mkdir(AnalysisFolder.c_str(), 1) == -1){	//This function returns -1 if the Folder already exists
		cout << "No analysis folder created. Probably already exists." << endl;
	}
	else{
	mkdir(AnalysisFolder.c_str(), 1);
	chmod(AnalysisFolder.c_str(), S_IRWXU|S_IRWXG);
	}
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::SetCalibrationFile(){
	cout << "Setting up calibration file. " << endl;

	std::string theInput;
    	double inputAsDouble;
	cout << "Enter SiPM bias voltage for picking up correct calibration file: ";
	cin >> theInput;

    	while(std::cin.fail() || std::cin.eof() || theInput.find_first_not_of(".0123456789") != std::string::npos) {

       		std::cout << "Error" << std::endl;

		if( theInput.find_first_not_of(".0123456789") == std::string::npos) {
			std::cin.clear();
			std::cin.ignore(256,'\n');
		}
		cout << "Set bias as type double: ";
		cin >> theInput;
	}

	std::string::size_type st;
	inputAsDouble = std::atoi(theInput.c_str());
//	std::cout << inputAsDouble << std::endl;
	bias = inputAsDouble;

	cout << "----------------------------------" << endl;
	stringstream temp_ss;
	string first10;
	for(int i = 0; i<10; i++){
		temp_ss << globalPath.at(i);
	}
	first10 = temp_ss.str();
//	cout << first10 << endl;

	stringstream ss;
	if(first10 == "/home/baye"){
//		ss << "/home/bayerlein/Measurements/20180112_16_ch_chkv_measurement/Calibration_" << bias << "V.txt";
		ss << "/home/bayerlein/Measurements/20180213_Hedia_Rey_Test/Calibration_" << bias << "V_Amp.txt";
		calibrationFile = ss.str();
		cout << calibrationFile << endl;
/*	}
	else if(first10 == "/home/wale"){
		ss << "/home/YOUR_PATH/Calibration_" << bias << "V.txt";
		calibrationFile = ss.str();
		cout << calibrationFile << endl;	*/
	}else if(first10 == "/data/comp"){
		ss << "/data/compton1/Measurements/Calibration_" << bias << "V.txt"; //for cluster FIXME
		calibrationFile = ss.str();
		cout << calibrationFile << endl;
	}else { 
		string temp_calibFile;
		cout << "NO CALIBRATION FILE FOUND!\nPlease enter location and name of Calibration to be used:" << endl;
		cin >> temp_calibFile;
		calibrationFile = temp_calibFile;
	}
	string yes_no = "empty";
	while (yes_no != "Y" && yes_no != "n"){
		cout << "Type \"Y\" if correct and \"n\" if not: ";
		cin >> yes_no ;
	}
	if(yes_no == "n") {
		string yes_no2 = "empty";
		while(yes_no2 !="Y" && yes_no2 != "n"){
			cout << "Would you like to enter a file manually (Y)? Otherwise go back one step to enter a bias voltage (n). ";
			cin >> yes_no2;
		}
		if(yes_no2 == "n") SetCalibrationFile();
		else {
			string temp_calibFile;
			bool calibFileExists = false;
			while(!calibFileExists){
				cout << "Please enter location and name of Calibration to be used:" << endl;
				cin >> temp_calibFile;
				ifstream in(temp_calibFile.c_str());
				if(!in) cout << "could not open file:\n" << temp_calibFile.c_str() << endl;
				else calibFileExists = true;
			}
			calibrationFile = temp_calibFile;
		}
	}
	cout << "----------------------------------" << endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::PrintParameters(){
	cout << "----------------------------------" << endl;
	cout << "----------------------------------" << endl;
	cout << "Summary of input values: " << endl;
	cout << "----------------------------------" << endl;
	cout << "Folder location (global Path): " << globalPath << endl;
//	cout << "Folder Name: " << folderName << endl;
	cout << "Input file name: "<< inputFileName << endl;
//	cout << "File name prefix: " << prefix << endl;
//	cout << "Number of files: "<< num_Files << endl;
//	cout << "Bias: " << bias << endl;
	cout << "Time per step (in pico seconds): " << timePerStep << endl ;
	cout << "Voltage Resolution (in mV): " 	<< voltageResolution << endl;
	cout << "Time before trigger (for base line correction): " << timeBeforeTrigger << endl;
	cout << "Record Length to be processed (in ns): " << recordLength << endl;
	cout << "Trigger Offset (in ns): " << triggerOffset << endl;
	cout << "Number of bins: " << num_Bins << endl;
	cout << "Bin width (in V*ps) :" << binWidth << endl;
	cout << "Lower histogram boundary: " << histo_low << endl;		
	cout << "Calculated upper histogram boundary: " << histo_high << endl;
//	cout << "----------------------------------" << endl;
	cout << "Boundary for BL correction with sigma rejection :"<< sigma_correction_boundary << endl;
	cout << "----------------------------------" << endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double RunControl::AdjustBinWidth(double input_bw){

	double minBin = (voltageResolution*timePerStep)/1000;	// minimum bin width according to resolution in (mV*ps)/1000=V*ps
	cout << "Min. bin width based on resolution [V*ps]: " << minBin << endl;

	stepsPerBin = (int)(input_bw / minBin);
	cout << "=> Steps Per Bin :" <<  stepsPerBin << endl;

	double output_bw = stepsPerBin*minBin;			// in V*ps
	cout << "=> Bin Width [V*ps] :" << output_bw << endl;

	return output_bw;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
					
void RunControl::CalculateUpperBoundary(){
	histo_low = -(20*binWidth/1000);
	histo_high = (num_Bins * binWidth/1000) + histo_low;	// in integer*ps/1000 + ns = integer*ns + ns
	cout << "=> Calculated upper histogram boundary: " << histo_high << endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::PrintLogFile(){
	cout << "Printing Log File..." << endl;	
/*	ofstream o;
	o.open(AnalysisFolder.c_str());
	o << "TEST" << endl;
	o.close();
*/
	stringstream ss;
//	ss << prefix << "_" << recordLength << "ns_" << num_Bins << "bins_" << binWidth << "Vps";
	ss << timeStamp << "_" << prefix; 
	outputFileName = ss.str();
	cout << "Output file name: " << outputFileName << endl;
	cout << "-------------------------------------------------------------" << endl;
	
	stringstream ss_2;
	string logFileName;
	ss_2 << AnalysisFolder << outputFileName << ".log";

	logFileName = ss_2.str();
	ofstream log;
	log.open(logFileName.c_str());
	log << "Summary of input valUes: " << endl;
	log << "----------------------------------" << endl;
	log << "Folder location (data): " << globalPath << endl;
	log << "Folder name of current run: " << folderName << endl;
	log << "Input file name: "<< inputFileName << endl;
//	log << "Bias: " << bias << endl;
	log << "File name prefix: " << prefix << endl;
	log << "Number of files: "<< num_Files << endl;
	log << "Channels: " << SiPMChannels[0] << ", " << SiPMChannels[1] << ", " << SiPMChannels[2] << ", " << SiPMChannels[3] << endl;
	log << "Time per step (in pico seconds): " << timePerStep << endl ;
	log << "Voltage Resolution (in mV): " 	<< voltageResolution << endl;
	log << "Time before trigger (for base line correction): " << timeBeforeTrigger << endl;
	log << "Record Length to be processed (in ns): " << recordLength << endl;
	log << "Trigger Offset (in ns): " << triggerOffset << endl;
	log << "Number of bins: " << num_Bins << endl;
	log << "Bin width (in V*ps) :" << binWidth << endl;
//	log << "Horizontal Offset (in Volts): " << horizontalOffset << endl;
	log << "Lower histogram boundary: " << histo_low << endl;
	log << "Calculated upper histogram boundary: " << histo_high << endl;
	log << "----------------------------------" << endl;
	log << "Boundary for BL correction with sigma rejection: "<< sigma_correction_boundary << endl;
	log << "----------------------------------" << endl;
	log << "Output file name: " << outputFileName << endl;
	//log << "Largest (uncorrected) Integral value: " << integral_max << endl;			// FIXME
	log.close();

}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::End(){
	cout << "----------------------------------" << endl;
	cout << "Error!" << endl;
	cout << "Too many bad files! Check folder location and input files for correct spelling! " << endl;
	cout << "----------------------------------" << endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::CreateFileList(){
cout << "Creating File List..." << endl;

//	char extension[128] = "Wfm.txt";
	char extension[128] = ".txt";	// This version of the script is beeing used for CONVERTED wfm files.
	for(int i = 1; i<num_Files+1;i++){
//		cout << i << endl;
		for(int ch = 1; ch<num_channels+1; ch++){
			if(!UseChannel[ch-1]) continue;
			string tempFileName;
			stringstream ss;
			ss <<globalPath <<folderName << "/"<< inputFileName << i << "_Ch" << ch << extension;
			tempFileName = ss.str();
	//		sprintf(tempFileName, "%s%s%d%s", globalPath, inputFileName, i, extension);
	//		cout << tempFileName << endl;
			fileList[ch-1][i-1] = tempFileName;
		}
	} 

cout << "...done." << endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::CreateResultsFile(){
	stringstream ss;
//	ss << AnalysisFolder << "ChannelResults_" << prefix << ".txt";
	ss << AnalysisFolder << timeStamp << "_ChannelResults.txt";
	resFileName = ss.str();	
	cout << "Channel-Results File name: " << resFileName << endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::WriteChannelResults(){
//	CreateResultsFile();
	ofstream res;
	res.open(resFileName.c_str(), std::ios_base::app| std::ios_base::out);
	for (int ch = 0; ch < num_channels; ch++){
		if(UseChannel[ch]){
			res << ch << "\t" << SiPMChannels[ch] << "\t" << h0[ch]->m << "\t" << h0[ch]->t << "\t" << h0[ch]->GetAveragePhotonNumber() << "\t" << h0[ch]->CalculateRMS()/sqrt(num_Files) << "\t" << prefix << endl; 
		}else{
			res << ch << "\t" << "nA\t" << h0[ch]->m << "\t" << h0[ch]->t << "\t-1\t" << "\t-1\t" << prefix << endl;
		}
	}
}

void RunControl::WriteChannelSummary(){
	Analysis *ana = Analysis::GetInstance();
	ofstream res;
	res.open(resFileName.c_str(), std::ios_base::app| std::ios_base::out);
	res 	<< "-----------------------------------------------------------------------------------------\n"
		<< "Explanation:\n"
		<< "Scope_Ch || SiPM_Ch || calib_sope || calib_offset || Av ph num || stat_uncert || prefix\n"
		<< "-----------------------------------------------------------------------------------------\n"
		<< "Summary of this measurement:\nTotal average number of photons:\t" << ana-> GetTotalNumPhotons()
		<< "\nTotal number of channels:\t" << ana->GetTotalNumChannels() 
		<< "\nAverage number of photons per channel:\t" << ana->GetAvNumPhotons() << endl;				//FIXME
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::WriteAllChannelResults(){
//	CreateResultsFile();
	ofstream res;
	res.open(resFileName.c_str(), std::ios_base::app| std::ios_base::out);
	for (int ch = 0; ch < num_channels*num_runs; ch++){
		if(allChannels[ch]->UseChannel){
			res << ch << "\t" << allChannels[ch]->SiPMChannel << "\t" << allChannels[ch]->m << "\t" << allChannels[ch]->GetAveragePhotonNumber()<< "\t" << prefix << endl; 
		}else{
			res << ch << "\t"<< h0[ch]->m << "\t-1\t" << prefix  << "nA" << endl;
		}
	}
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunControl::CreateCorrectFolderPaths(){	// Does not have to be in the code if it is too complicated to modify for strings //FIXME
	string slash = "/";

	size_t found = globalPath.find_last_not_of(slash);
	if(found!=string::npos){
		if (found+1 == globalPath.length()) {
			stringstream ss;
			ss << globalPath << slash;
			globalPath = ss.str();
			cout << "Added one slash at the end of the path. You're welcome." << endl;
		}
	}
	cout << "Global Path to be used: " << globalPath << endl;

// This comes from former WaveformAnalysis.cc

/*
  char * pch;
//  printf ("Looking for the '/' character in \"%s\"...\n",globalPath);
  pch=strchr(globalPath,'/');
	int slashCounter = 0;
	int slashArray[16];
  while (pch!=NULL)
  {
//    printf ("found at %d\n",pch-globalPath+1);
	slashArray[slashCounter] = pch-globalPath+1;
//	cout << "slashArray[" << slashCounter << "] " << slashArray[slashCounter] << endl;
    pch=strchr(pch+1,'/');
	slashCounter++;
  }
//cout << "Number of slashes in globalPath: " << slashCounter << endl;

int anaFolderLevel = slashCounter-2;

char empty[2] = "";
char slash[2] = "/";
if (globalPath[slashArray[slashCounter-1]] != empty[0]) {
cout << "slash missing..." << endl;
sprintf (globalPath, "%s%s", globalPath, slash);
anaFolderLevel = slashCounter-1;
cout << "adding slash at the end:" << endl;
cout << "Global path of input files: "<< globalPath << endl;
}else{
cout << "Folder Location correct." << endl;
cout << "Global path of input files: "<< globalPath << endl;
}

for(int i = 0; i < slashArray[anaFolderLevel]; i++){
	AnalysisFolder[i] = globalPath[i];
}

sprintf(AnalysisFolder, "%s%s%s", AnalysisFolder, prefix, "_Analysis/");
cout << "AnalysisFolder: " << AnalysisFolder << endl;

	if (mkdir(AnalysisFolder, 1) == -1){	//This function returns -1 if the Folder already exists
		cout << "No analysis folder created. Probably already exists." << endl;
	}
	else{
	mkdir(AnalysisFolder, 1);
	chmod(AnalysisFolder, S_IRWXU);
	}
*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
