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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//Variables red from configuation file
size_t numSets; //set is similar to a block
size_t associativityLevel;
size_t lineSize;
size_t numDataLines;

//Variables for program
size_t numEntries;

//Program constants; represent the greatest possible extents
const size_t MAX_SETS = 8096;
const size_t MAX_ASSOCIATIVITY = 8;


//Function prototypes
void ReadConfig();
void PrintConfig();
size_t ReadDataTrace(std::vector<std::string> &);
void ParseDataTrace(size_t);
void ModeValidityCheck(const char);
bool SizeCheck(const unsigned int);
bool AlignmentCheck(const unsigned int,const unsigned int);


class CacheBlock
{
public:
    //default constructor
    CacheBlock()
    {
        blockData.LRU = 0;
        blockData.Data = 0;
        blockData.Tag = 0;
        blockData.dirtyBit = 0;
        blockData.validBit = 0;
    };
    
    //constructor with argument
    CacheBlock(size_t init)
    {
        blockData.LRU = init;
        blockData.Data = init;
        blockData.Tag = init;
        blockData.dirtyBit = init;
        blockData.validBit = init;
    }
    
    //default destructor
   ~CacheBlock()
    {};
    
    //copy constructor, necessary for vector assignment
    CacheBlock(const CacheBlock &c2)
    {
        blockData.LRU = c2.blockData.LRU;
        blockData.Data = c2.blockData.Data;
        blockData.Tag = c2.blockData.Tag;
        blockData.dirtyBit = c2.blockData.dirtyBit;
        blockData.validBit = c2.blockData.validBit;
    }
    
    void EraseMembers()
    {
        blockData.LRU = 0;
        blockData.Data = 0;
        blockData.Tag = 0;
        blockData.dirtyBit = 0;
        blockData.validBit = 0;
    }
    
    //establishes a cahceBlock which holds all the relevant data
    struct {
        unsigned short LRU;
        unsigned short Data; //we don't actually care about the data in this simulation
        unsigned short Tag;
        unsigned short dirtyBit;
        unsigned short validBit;
    } blockData;
    
private:
    //copies of DataStructs shouldn't occur and copy constructor is not implemented
    
    
}; //end class DataStruct



//Important typeDefs
typedef std::vector<CacheBlock> cacheSet; //vector of cacheBlocks of 8192 sets
typedef std::vector<cacheSet> setAssociation; //vector of cacheSets



int main()
{
    //Establish Configuration
    ReadConfig();
    PrintConfig();
    
    //Reads the Data Trace File and stores it.  Determines how many lines were sucessfully read.
    std::vector<std::string> traceDat;
    numDataLines = ReadDataTrace(traceDat);
    
    CacheBlock defaultBlock(0); //creates a cache block with all bit fields initialized to 0.
    setAssociation cacheAssociation(MAX_ASSOCIATIVITY, cacheSet(MAX_SETS,defaultBlock));
    
    char            mode;
    unsigned int    dataSize;
    unsigned int    address;
    bool            sizeError;
    bool            alignmentError;
    unsigned int    index;
    unsigned int    offset;
    unsigned int    tag;
    unsigned int    hitCounter;
    unsigned int    missCounter;
    unsigned int    memrefs;
    
    size_t refCounter = 0; //holds the number of references read
    
    //Output header
    std::cout << "Results for Each Reference\n\n";
    std::cout << "Ref  Access Address    Tag   Index Offset Result Memrefs\n";
    std::cout << "---- ------ -------- ------- ----- ------ ------ -------\n";

    //Main program Loop
    for (size_t i = 0; i < numDataLines; ++i)
    {
        sscanf(traceDat[i].c_str(),"%c:%d:%x", &mode, &dataSize, &address);
        
        //check if mode is 'R' or 'W', exit otherwise
        ModeValidityCheck(mode);
        
        //check if data size is 1,2,4, or 8
        sizeError = SizeCheck(dataSize);
        if(sizeError)
        {
            std::cout << "line " << (i + 1) << " has illegal size " << dataSize << "\n";
            continue; //continue to next line
        }
        
        alignmentError = AlignmentCheck(dataSize, address);
        if (alignmentError)
        {
            std::cout << "line " << (i + 1) << " has misaligned reference at address " << address << " for size " << dataSize << "\n";
            continue; //continue to next line
        }
        
        //no size or alignment errors, increment ref Counter
        ++refCounter;
        std::cout << "This was a valid line\n";
        
        
        
        
        
        
        /*Methodology for determining appropriate values for index, offset, and tag:
         The address is a 32-bit number represented as
         
         XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX
         
         In order to determine the appropriate index, offset, and tag value, we must first determine
         the number of bits that represent the index and offset.  Then, the reamining bits represent the
         tag.
         
         The structure of the address is as follows:
         
         |---------tag----------|--index---|--offset--|
         
         The number of bits for the index and offset are computed as:
         
         offset = log2(lineSize), e.g. log2(16) = 4
         index = log2(setSize) e.g. log2(8) = 3
         tag = any bits that remain
         */
        
        
        //Determine the index, offset, and tag
        unsigned int tempAddress = address; //to preserve oriignal address value
        
        unsigned int offsetBitMask = static_cast<unsigned int>(lineSize - 1);
        unsigned int offsetShamt = static_cast<unsigned int>(log2(lineSize));
        offset = tempAddress & offsetBitMask; //determine offset
        tempAddress = tempAddress >> offsetShamt;
        
        unsigned int indexBitMask = static_cast<unsigned int>(numSets - 1);
        unsigned int indexShamt = static_cast<unsigned int>(log2(numSets));
        index = tempAddress & indexBitMask; //determine index
        tempAddress = tempAddress >> indexShamt;
        
        tag = address; //determine tag from remaining bits
        
    
        //Write the result to the cache
        //We don't need to worry about numSets; this is taken care of by Index.
        //Line size is irrelevant for this portion
        //We only have to worry about associativity level
        
        //Relevant varibles: mode
        //                   refCounter
        //                   hitCounter
        //                   missCounter
        //                   memrefs
        
        
        
        
        
        
        
        
        
        
        
        
        
        
    }
    
    

    
    std::cout << "Hold program\n";
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

//checks to see if there is a problem with the input based on configuration properties.

void ModeValidityCheck(const char mode)
{
    if (mode != 'R' && mode != 'W' && mode != 'r' && mode != 'w')
    {
        std::cerr << "Mode is invalid.  Exiting.\n";
        exit(EXIT_FAILURE);
    }
}

//returns TRUE if there is an error with the data size
bool SizeCheck(const unsigned int size)
{
    bool sizeError = 0; //set default to no problem
    if (size != 1 && size != 2 && size != 4 && size != 8)
    {
        sizeError = 1;
    }
    return sizeError;
}

//returns TRUE if there is an error with the alignment
bool AlignmentCheck(const unsigned int size, const unsigned int address)
{
    //if size is not multiple of the address then error
    if ((address % size) != 0)
        return 1;
    else
        return 0;
}








