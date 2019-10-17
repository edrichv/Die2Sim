/**
 * Author:  		JF de Villiers
 * Origin:  		E&E Engineering - Stellenbosch University
 * For:					Supertools, Coldflux Project - IARPA
 * Created: 		2019-03-20
 * Modified:
 * license:
 * Description: Primary file for the program.
 * File:				main.cpp
 */

#include <iostream>				//stream
#include <string>					//string goodies
#include <set>
#include <map>

// #include "die2sim/ToJosim.hpp"
#include "die2sim/genFunc.hpp"
// #include "die2sim/ClassDef.hpp"
#include "toml/toml.hpp"
#include "die2sim/ToJosim.hpp"

#define versionNo 1.0
#define configFile "config.toml"
#define outFolder "data/results/"
#define outFolderJoSIM "data/results/josim/"
using namespace std;

/**
 * Declaring functions
 */

void welcomeScreen();
void helpScreen();
int RunTool(int argCount, char** argValues);
int RunToolFromConfig();

/**
 * Main loop
 */

int main(int argc, char* argv[]){
	// welcomeScreen();
	if(!RunTool(argc, argv)) return 0;

	return 0;
}

/**
 * [RunTool - Runs the appropriate tool]
 * @param  argCount  [Size of the input argument array]
 * @param  argValues [The input argument array]
 * @return           [1 - all good; 0 - error]
 */

int RunTool(int argCount, char** argValues){
	welcomeScreen();

	if(argCount <= 1){
		return 0;
	}

	set<string> validCommands = {"-j", "-v", "-h", "-c"};

	string outFName = "\0";			// The output file, which is follow by the -o parameter
	string lefFName = "\0";			// The LEF file
	string defFName = "\0";			// The DEF file
	string command  = "\0";			// The command to be executed

	string foo;

	// --------------------------------------------------------------------
	// ---------------------- Searching for parameters --------------------
	// --------------------------------------------------------------------

	// search for command
	for(int i = 0; i < argCount; i++){
		foo = string(argValues[i]);
		if(validCommands.find(foo) != validCommands.end()){
			command = foo;
		}
	}
	if(!command.compare("\0")){
		cout << "Invalid." << endl;
		return 0;
	}

	// search for .lef
	for(int i = 0; i < argCount; i++){
		foo = string(argValues[i]);
	  if(foo.find(".lef")!=string::npos){
	  	lefFName = foo;
	  }
	}

	// search for .def
	for(int i = 0; i < argCount; i++){
		foo = string(argValues[i]);
	  if(foo.find(".def")!=string::npos){
	  	defFName = foo;
	  }
	}

	// --------------------------------------------------------------------
	// ------------------------ Output file naming ------------------------
	// --------------------------------------------------------------------

	// search for output filename
	for(int i = 0; i < argCount-1; i++){
	  if(!string(argValues[i]).compare("-o")){
	  	outFName = string(argValues[i+1]);
	  }
	}
	// auto assign output filename if non has been set
	if(!outFName.compare("\0") && defFName.compare("\0")){
		if(!command.compare("-j")){
			outFName = fileRenamer(defFName, outFolderJoSIM, ".cir");
		}
	}


	// --------------------------------------------------------------------
	// ------------------------------ Commands ----------------------------
	// --------------------------------------------------------------------
	if(!command.compare("-j")){
		if(defFName.compare("\0") && outFName.compare("\0")){
			// runJoSIM(lefFName, defFName, outFName);
			executeDef2Josim("config.toml", defFName, "judescir.ir");
			return 1;
		}
		else {
			cout << "Input argument error." << endl;
			return 0;
		}
	}
	else if(!command.compare("-c")){
		if(argCount == 1 + 1){
			return RunToolFromConfig();
		}
		cout << "Input argument error." << endl;
		return 0;
	}
	else if(!command.compare("-v")){
		if(argCount == 1 + 1){
			cout << setprecision(2);
			cout << "Version: " << versionNo << endl;
			return 1;
		}
		cout << "Input argument error." << endl;
		return 0;
	}
	else if(!command.compare("-h")){
		helpScreen();
		return 1;
	}
	else{
		cout << "Quickly catch the smoke before it escapes." << endl;
		return 0;
	}

	cout << "I am smelling smoke." << endl;
	return 0;
}


/**
 * [RunToolFromConfig - Runs the tools using parameters from config.toml]
 * @return [1 - all good; 0 - error]
 */

int RunToolFromConfig(){
	cout << "Importing execution parameters from config.toml" << endl;

	const auto mainConfig  = toml::parse(configFile);
	map<string, string> run_para = toml::get<map<string, string>>(mainConfig.at("run_parameters"));

	map<string, string>::iterator it_run_para;

	string outFName = "\0";
	string lefFName = "\0";
	string defFName = "\0";
	string command  = "\0";

	it_run_para = run_para.find("Command");
	if(it_run_para != run_para.end()){
		command = it_run_para->second;
	}
	else{
		cout << "Invalid parameters." << endl;
		return 0;
	}

	it_run_para = run_para.find("defFileName");
	if(it_run_para != run_para.end()){
		defFName = it_run_para->second;
	}

	it_run_para = run_para.find("outFileName");
	if(it_run_para != run_para.end()){
		outFName = it_run_para->second;
	}

	if(!outFName.compare("\0") && defFName.compare("\0")){
		if(!command.compare("josim")){
			outFName = fileRenamer(defFName, outFolderJoSIM, ".cir");
		}
	}

	if(!command.compare("josim")){
		if(defFName.compare("\0") && outFName.compare("\0")){
			// runJoSIM(lefFName, defFName, outFName);
			def_file deffile;
			deffile.importFile(defFName);
			deffile.to_str();
			return 1;
		}
		else {
			cout << "Input argument error." << endl;
			return 0;
		}
	}
	else{
		cout << "Invalid command." << endl;
		return 0;
	}

	return 0;
}


void helpScreen(){
	cout << "===============================================================================" << endl;
	cout << "Usage: Die2Sim [ OPTION ] [ filenames ]" << endl;
	cout << "-j(oSIM)      Converts LEF/DEF to .cir then simulates it through JoSIM." << endl;
	cout << "                [.def file] [.def file] -o [.cir file]" << endl;
	cout << "-c(onfig)     Runs the tools using the parameters in the config.toml file." << endl;
	cout << "-v(ersion)    Displays the version number." << endl;
	cout << "-h(elp)       Help screen." << endl;
	cout << "===============================================================================" << endl;
}

/**
 * Welcoming screen
 */

void welcomeScreen(){
	cout << "=====================================" << endl;
	cout << "              Die2Sim" << endl;
	cout << "       Author JF de Villiers" << endl;
	cout << "      Stellenbosch University" << endl;
	cout << "    For IARPA, ColdFlux project" << endl;
	cout << "=====================================" << endl;
}