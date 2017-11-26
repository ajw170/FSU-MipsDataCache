/*
 Andrew J Wood
 CDA3101 Project 6 - MIPS DataCache Simulator
 November 16, 2017
 
 The program performs the following in order:
 
 1) Reads the configuration file using the ReadConfig() function to determine the set configuration and line size.
 2) Prints the configuration using the PrintConfig() function.
 3) Builds the sets assuming the maximum number and associativity. The program may or may not use all of them depending on the configuration.
 
 Note that the code is self-documenting.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

//Variables red from configuation file
size_t numSets;
size_t associativityLevel;
size_t lineSize;
size_t numDataLines;

//Variables for program
size_t numEntries;

//Program constants; represent the greatest possible extents
const size_t MAX_SETS = 8192;
const size_t MAX_ASSOCIATIVITY = 8;

//Function prototypes
void ReadConfig();
void PrintConfig();
size_t ReadDataTrace(std::vector<std::string> &);
void ParseDataTrace(size_t);



class DataStruct
{
public:
    
    DataStruct(); //default constructor
    ~DataStruct(); //default destructor
    
    void EraseMembers()
    {
        cacheBlock.LRU = 0;
        cacheBlock.Data = 0;
        cacheBlock.Tag = 0;
        cacheBlock.dirtyBit = 0;
        cacheBlock.validBit = 0;
    }
    
    //establishes a cahceBlock which holds all the relevant data
    struct {
        unsigned short LRU;
        unsigned short Data; //we don't actually care about the data in this simulation
        unsigned short Tag;
        unsigned short dirtyBit;
        unsigned short validBit;
    } cacheBlock;
    
private:
    //copies of DataStructs shouldn't occur and copy constructor is not implemented
    DataStruct(DataStruct &)
    {};
    
}; //end class DataStruct



int main()
{
    //Establish Configuration
    ReadConfig();
    PrintConfig();
    
    //Reads the Data Trace File and stores it.  Determines how many lines were sucessfully read.
    std::vector<std::string> traceDat;
    numDataLines = ReadDataTrace(traceDat);
    
    //Configure data cache
    
    DataStruct test;
    
    test.cacheBlock.LRU = 0;
    
    
    
    
}



//Reads the configuration file and assigns values to program variables.
//The configuration file is assumed to be valid for this program.
void ReadConfig()
{
    std::ifstream inConfigFile("trace.config",std::ios::in);
    if (!inConfigFile)
    {
        std::cerr << "Configuration file could not be opened\n";
        exit(EXIT_FAILURE);
    }
    inConfigFile.seekg(std::ios::beg);
    
    //iterate through the line until the ":" is found
    char searchChar = '\0';
    
    while (searchChar != ':')
    {
        searchChar = inConfigFile.get();
    }
    inConfigFile >> numSets; //read the numSets line
    searchChar = '\0';
    
    //continue to the next line until next ':' is found
    while (searchChar != ':')
    {
        searchChar = inConfigFile.get();
    }
    inConfigFile >> associativityLevel; //read the numSets line
    searchChar = '\0';
    
    //continue to the next line until next ':' is found
    while (searchChar != ':')
    {
        searchChar = inConfigFile.get();
    }
    inConfigFile >> lineSize; //read the numSets line
    searchChar = '\0';
    
    //Finished with configuration file
    inConfigFile.close();
}



//Prints the configuration information to the
void PrintConfig()
{
    std::cout << "Cache Configuration\n\n";
    std::cout << "   " << numSets << " " << associativityLevel << "-way set associative entries\n";
    std::cout << "   of line size " << lineSize << " bytes\n\n\n";
}



//Reads trace.dat file and stores it in a vector for later use
size_t ReadDataTrace(std::vector<std::string> & traceDat)
{
    //NOTE - This will need to be changed for final program when input will be directed via stdin on command line.
    //This program was developed and tested using XCode which does not directly support re-directed input
    std::streambuf *cinBuf, *datBuf;
    cinBuf = std::cin.rdbuf(); //backs up the std::cin buffer.
    
    //*************comment out this section to prepare for cin read
    std::ifstream inDatFile("trace.dat",std::ios::in);
    if (!inDatFile)
    {
        std::cerr << "Failed to read data file.\n";
        exit(EXIT_FAILURE);
    }
    datBuf = inDatFile.rdbuf(); //obtains buffer for inFile stream
    std::cin.rdbuf(datBuf);   //Assign cin to the data buffer.  Now, cin will be reading from file.
    //*************comment out this section to prepare for cin read
    
    std::string inputLine;
    numDataLines = 0;
    
    //continue reading until end found
    while (std::getline(std::cin, inputLine))
    {
        traceDat.push_back(inputLine); //adds the line to the data vector
        ++numDataLines;
        
    }
    
    std::cin.rdbuf(cinBuf); //restore cin to original state; has no effect if previous section is commented out
    return numDataLines;
}








