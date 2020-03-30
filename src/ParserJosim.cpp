/**
 * Author:  		Jude de Villiers
 * Origin:  		E&E Engineering - Stellenbosch University
 * For:					Supertools, Coldflux Project - IARPA
 * Created: 		2019-04-25
 * Modified:
 * license:
 * Description: Passer to create a JoSIM file.
 * File:				ParserJosim.cpp
 */

#include "die2sim/ParserJosim.hpp"

/**
 * [JoSimFile::genCir - Generates the final .cir file in the correct formate]
 * @return [1 - all good; 0 - error]
 */

int JoSimFile::genCir(string fileName){
	cout << "Creating JoSIM file -> " << fileName << endl;

  /**
   * Initialising the file
   */


  ofstream cirFile;

  cirFile.open(fileName, fstream::out);
  if(!cirFile){
    cout << "Could not create \"" << fileName << "\"" << endl;
    return 0;
  }

  /**
   * Creating header for the file
   */

  cirFile << makeFileHeader("");
  cirFile.close();


  /**
   * Importing the gate circuit files
   */

  cout << "Importing:" << endl;
  for(auto const &itFn :this->fileNamesImport){
    int i = 0;
    cout << "  [" << i++ << "]: " << itFn << endl;
    cpFile(itFn, fileName);
  }


  cirFile.open(fileName, fstream::out | std::fstream::app);

  /**
   * prepend the subcircui
   */

  cirFile << makeHeader(this->subcktName) << endl;
  cirFile << this->createSubcktHeader() << endl;

  /**
   * Generating the components for the circuit
   */

  cirFile << makeHeader("Components") << endl;
  for(auto &it: this->comps){
    cirFile << it.to_cir() << endl;
  }

  /**
   * Generating the PTLs for the components
   */

  cirFile << makeHeader("Passive Transmission Lines") << endl;
  for(auto &it: this->PTLs){
    cirFile << it.to_cir() << endl;
  }

  /**
   * Append the circuit
   */

  cirFile << ".ends Created_subckt" << endl;
  cirFile.close();

  cout << "Creating JoSIM file: \"" << fileName << "\" done." << endl;

	return 1;
}

/**
 * [JoSimFile::importCir - Adds the name of the .cir files to be imported]
 * @param fileName [Name of the .cir file]
 */

int JoSimFile::importCir(string fileName){
	this->fileNamesImport.push_back(fileName);
	return 1;
};

/**
 * [JoSimFile::pushComp - Adds a component]
 * @param  name         [unique name of the component]
 * @param  compTypeName [the component/subcircuit name]
 * @param  netNames     [vector of all nets connected to the pins]
 * @return              [0 - all good; 1 - error]
 */

int JoSimFile::pushComp(string name, string compTypeName, vector<string> &netNames){
  CompClass fooComp;

  fooComp.create(name, compTypeName, netNames);
  this->comps.push_back(fooComp);

  if(!compTypeName.compare("PAD")){
    this->subcktNetName.push_back(netNames.back());
    this->subcktNetDes.push_back(name);
  }
  return 0;
}

/**
 * [JoSimFile::pushPTL - Adds a PTL]
 * @param  name    [unique name of the PTL]
 * @param  netName [the name of the net]
 * @param  len     [The length of the track in nm]
 * @return         [0 - all good; 1 - error]
 */

int JoSimFile::pushPTL(const string &name, const string &netName, unsigned int len){
  PTLclass fooPTL;

  fooPTL.create(name, netName, len);
  this->PTLs.push_back(fooPTL);

  return 0;
}

/**
 * [JoSimFile::createSubcktHeader - Creates the header for the main sub-circuit]
 * @return [The built string for the header]
 */

string JoSimFile::createSubcktHeader(){
	string subcktStr = "* ";
	for(unsigned int i = 0; i < this->subcktNetDes.size(); i++){
	  subcktStr += "\t" + subcktNetDes[i];
	}

  subcktStr += "\n.SUBCKT " + this->subcktName;

  for(unsigned int i = 0; i < this->subcktNetName.size(); i++){
	  subcktStr += "\t" + subcktNetName[i];
	}

	return subcktStr;
}

/**
 * [JoSimFile::to_str - Prints the class information to the terminal]
 */

void JoSimFile::to_str(){
	// imported .cir files
	cout << "Name of the files to me imported:" << endl;
	for(int i = 0; i < this->fileNamesImport.size(); i++){
		cout << "[" << i << "]: " << this->fileNamesImport[i] << endl;
	}
	cout << endl;

	// components added
	cout << "Components added:" << endl;
	for(int i = 0; i < this->placeComp.size(); i++){
		cout << "[" << i << "]: " << this->placeComp[i] << endl;
	}
}


/**
 * [CompClass::create - Creates the class]
 * @param  CompName     [The unique name of the component/subcircuit]
 * @param  compTypeName [The subcircuit or component type]
 * @param  netsIn       [The vectors of the nets connected to the component or subcircuit]
 * @return              [0 - Good; 1 - Error]
 */

int CompClass::create(const string &CompName, const string &compTypeName, vector<string> netsIn){
  this->name = CompName;
  this->compType = compTypeName;
  this->nets = netsIn;

  return 0;
}


/**
 * [CompClass::to_cir - creates the subcircuit instance for JoSIM]
 * @return [The string in the format JoSIM requires]
 */

string CompClass::to_cir(){
  stringstream ss;

  ss << left;
  ss << setw(20) << ("X" + this->name);
  ss << setw(20) << compType;

  for(auto const &it: this->nets){
    ss << setw(7) << it;
  }

  return ss.str();
}


/**
 * [PTLclass::create - Creates the class]
 * @param  PTLname   [The name of the net/PTL]
 * @param  NetName   [The net name]
 * @param  PTLlength [The length of the PTL in nm]
 * @return           [0 - Good; 1 - Error]
 */

int PTLclass::create(const string &PTLname, const string &NetName, int PTLlength){
  this->name = PTLname;
  this->nameNet = NetName;
  this->length = PTLlength;

  return 0;
}

/**
 * [PTLclass::to_cir - creates the transmission line for JoSIM]
 * @return [The string in the format JoSIM requires]
 */

string PTLclass::to_cir(){
  stringstream ss;

  ss << setw(7) << ("T" + this->name);
  ss << setw(6) << (nameNet + "A");
  ss << setw(4) << (0);
  ss << setw(6) << (nameNet + "B");
  ss << setw(4) << (0);
  ss << setprecision(2);
  ss << setw(0) << "  LOSSLESS Z0=5.00  TD=" << this->length * speedConstant <<  "p";

  return ss.str();
}


/**
 * [cpFile - Copies one text file to another]
 * @param  fromFile [Name of the file to be copied from]
 * @param  toFile   [Name of the file to be copied to]
 * @return          [1 - all good; 0 - error]
 */

int cpFile(string fromFile, string toFile){
  FILE *inFile, *outFile;
  char c;

  inFile = fopen(fromFile.c_str(), "r");
	if(inFile==NULL){
  	cout << "Could not open \"" << fromFile.c_str() << "\" for copying" << endl;
  	return 0;
  }

  outFile = fopen(toFile.c_str(), "a");
	if(inFile==NULL){
  	cout << "Could not create \"" << toFile.c_str() << "\" for copying" << endl;
  	return 0;
  }

  c = fgetc(inFile);
  while(c != EOF){
	  fputc(c, outFile);
	  c = fgetc(inFile);
  }

  fclose(inFile);
  fclose(outFile);

	return 1;
}

/**
 * [makeHeader - Creates a paragraph header]
 * @param  HeaderName [The name of the header]
 * @return            [The created header]
 */

string makeHeader(string HeaderName){
	string foo;

	foo = "\n\n* ==========================================================================\n* ";

	for(int i = 0; i < (74 - HeaderName.length())/2-1; i++){
		foo = foo + "=";
	}

	foo = foo + " " + HeaderName + " ";

	for(int i = 0; i < (74 - HeaderName.length())/2-1; i++){
		foo = foo + "=";
	}

	foo = foo + "\n* ==========================================================================\n\n";

	return foo;
}

string makeFileHeader(string someText){
  string foo;

  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime(&rawtime);


  foo =  "* JoSIM file generated with Die2Sim, " + (string)asctime(timeinfo);
  foo += "\n* Jude de Villiers, Stellenbosch University\n\n";

  return foo;
}