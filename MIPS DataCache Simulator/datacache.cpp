/*
 Andrew J Wood
 CDA3101 Project 6 - MIPS DataCache Simulator
 November 16, 2017
 
 The program performs the following in order:
 
 1) Reads the configuration file using the ReadConfig() function to determine the set configuration and line size.
 2) Prints the configuration using the PrintConfig() function.
 3) Builds the sets assuming the maximum number and associativity. The program may or may not use all of them depending on the configuration.
 4) Enteres the main loop and interprets the data line by line.  Note that the data is extraced once from the cin stream and then stored in an 
    array "TraceDat" for future use.
 5) Outputs an error to cerr if a size or alignment problem is detected.  In this case no counters are incremented.
 5) Outputs each line based on the results and increments the appropriate counters (hit or miss) as well as the access counter.
 
 Compilation statement:
 g++ -I. -Wall -Wextra -o datacache_i.x datacache.cpp
 
 Note that the code is self-documenting.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
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
const size_t MAX_SETS = 8192;
const size_t MAX_ASSOCIATIVITY = 8;

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
    
}; //end class CacheBlock


//Important typeDefs
typedef std::vector<CacheBlock> cacheSet; //vector of cacheBlocks of 8192 sets
typedef std::vector<cacheSet> setAssociation; //vector of cacheSets


//Function prototypes
void ReadConfig();
void PrintConfig();
size_t ReadDataTrace(std::vector<std::string> &);
void ParseDataTrace(size_t);
void ModeValidityCheck(const char);
bool SizeCheck(const unsigned int);
bool AlignmentCheck(const unsigned int,const unsigned int);
void PrintSummary(size_t,size_t,size_t);
void DumpCache(setAssociation &, size_t);


//Begin main program
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
    size_t          hitCounter;
    size_t          missCounter;
    size_t          refCounter = 0; //holds the number of references read
    
    //Output header
    std::cout << "Results for Each Reference\n\n";
    std::cout << "Ref  Access Address    Tag   Index Offset Result Memrefs\n";
    std::cout << "---- ------ -------- ------- ----- ------ ------ -------\n";

    //Main program Loop
    for (size_t programLine = 0; programLine < numDataLines; ++programLine)
    {
        sscanf(traceDat[programLine].c_str(),"%c:%d:%x", &mode, &dataSize, &address);
        
        //check if mode is 'R' or 'W', exit otherwise
        ModeValidityCheck(mode);
        
        //check if data size is 1,2,4, or 8
        sizeError = SizeCheck(dataSize);
        if(sizeError)
        {
            std::cerr << "line " << (programLine + 1) << " has illegal size " << dataSize << "\n";
            continue; //continue to next line
        }
        
        alignmentError = AlignmentCheck(dataSize, address);
        if (alignmentError)
        {
            std::cerr << "line " << (programLine + 1) << " has misaligned reference at address " << std::hex << address << " for size " <<
                std::dec << dataSize << "\n";
            continue; //continue to next line
        }
        
        //no size or alignment errors, increment ref Counter
        ++refCounter;

        //Determine the index, offset, and tag
        unsigned int tempAddress = address; //to preserve oriignal address value
        
        unsigned int offsetBitMask = static_cast<unsigned int>(lineSize - 1);
        unsigned int offsetShamt = static_cast<unsigned int>(log2(lineSize));
        offset = tempAddress & offsetBitMask;       //determine offset
        tempAddress = tempAddress >> offsetShamt;
        
        unsigned int indexBitMask = static_cast<unsigned int>(numSets - 1);
        unsigned int indexShamt = static_cast<unsigned int>(log2(numSets));
        index = tempAddress & indexBitMask;         //determine index
        tempAddress = tempAddress >> indexShamt;
        
        tag = tempAddress; //determine tag from remaining bits
        
        //Write the result to the cache
        //We don't need to worry about numSets; this is taken care of by Index.
        //Line size is irrelevant for this portion
        //We only have to worry about associativity level
        
        bool isThere = 0;
        unsigned int memrefs = 0;
        unsigned int tempTag = 0;
        bool isValid = 0;
        size_t hitSet = 0;
        
        //Relevant varibles: mode
        //                   refCounter
        //                   hitCounter
        //                   missCounterisVal
        //                   memrefs
        //                   isValid
        //                   associativityLevel
        //                   cacheAssociation[index is 0 to (associativityLevel -1)]
        
        memrefs = 0; //set memrefs to 0 to start with
        
        
        //check to see if tag is already in corresponding index. "isThere" tells us if it is a hit or miss
        for (size_t i = 0; i < associativityLevel; ++i)
        {
            isValid = (cacheAssociation[i])[index].blockData.validBit;
            tempTag = (cacheAssociation[i])[index].blockData.Tag;
            if (isValid && (tempTag == tag))
            {
                isThere = 1;
                hitSet = i;
            }
        }
        
        //HIT**********
        //Now includes update LRU.
        if (isThere) //if we found a hit, behavior will vary depending on whether it is a READ or a WRITE
        {
            if (mode == 'R' || mode == 'r') //if we are in read mode, memref will be 0.
            {
                if (cacheAssociation[hitSet][index].blockData.LRU != (associativityLevel - 1)) //if the LRU bit is not the highest
                {
                    unsigned int LRU_Test = cacheAssociation[hitSet][index].blockData.LRU; //get current LRU value
                    for (size_t i = 0; i < associativityLevel; ++i) //subtract 1 from LRUs that are greater than LRU_Test
                    {
                        if (cacheAssociation[i][index].blockData.LRU > LRU_Test)
                            --(cacheAssociation[i][index].blockData.LRU);
                    }
                    cacheAssociation[hitSet][index].blockData.LRU = (associativityLevel - 1); //now, set LRU of hit block to highest
                }
            }
            
            else if (mode == 'W' || mode == 'w') // if we are in write mode, memref will be 0.
            {
                cacheAssociation[hitSet][index].blockData.dirtyBit = 1; //set dirty bit to 1, since we're writing to the block
                cacheAssociation[hitSet][index].blockData.validBit = 1; //set valid bit to 1, regardless of previous state
                cacheAssociation[hitSet][index].blockData.Tag      = tag; //set tag to computed value
                cacheAssociation[hitSet][index].blockData.Data     = 0; //set data to 0, irrelevant for simulation
                
                //Now, determine what the LRU bits should be set to.
                if (cacheAssociation[hitSet][index].blockData.LRU != (associativityLevel - 1)) //if the LRU bit is not the highest
                {
                    unsigned int LRU_Test = cacheAssociation[hitSet][index].blockData.LRU; //get current LRU value
                    for (size_t i = 0; i < associativityLevel; ++i) //subtract 1 from LRUs that are greater than LRU_Test
                    {
                        if (cacheAssociation[i][index].blockData.LRU > LRU_Test)
                            --(cacheAssociation[i][index].blockData.LRU);
                    }
                    cacheAssociation[hitSet][index].blockData.LRU = (associativityLevel - 1); //now, set LRU of hit block to highest
                }
            } //else if - write mode
            
            else //program should never get here
            {
                std::cerr << "A fatal error occurred in HIT where the mode was neither Read nor Write.\n";
                exit(EXIT_FAILURE);
            }
        } //HIT
        
        else //MISS********** - we did not find a match, now determine which to overwrite.  Memref value will vary.
        {
            memrefs = 1; //memrefs will be at least 1.
            
            if (mode == 'R' || mode == 'r') //if we are in read mode with a miss
            {
                size_t indexToUse = 0; //first, determine which index to use.  Assume we start with 0.
                for (size_t i = 0; i < associativityLevel; ++i)
                {
                    if (cacheAssociation[i][index].blockData.LRU == 0)
                    {
                        indexToUse = i;
                        break;  //once 0 is found, break out of loop.
                    }
                }
                
                if (cacheAssociation[indexToUse][index].blockData.dirtyBit == 1) //if the block we're about to overwrite is dirty
                {
                    memrefs = 2; //set memrefs =2
                    cacheAssociation[indexToUse][index].blockData.dirtyBit = 0; //set dirtyBit back to 0, this is read mode
                }
                
                //Now, update the remaining portions of the block.
                cacheAssociation[indexToUse][index].blockData.validBit = 1; //in case it is already not 1
                cacheAssociation[indexToUse][index].blockData.Tag = tag;
                cacheAssociation[indexToUse][index].blockData.Data = 0; //we don't care about the data
                
                //Now, determine what the LRU bits should be set to.
                for (unsigned int i = 0; i < associativityLevel; ++i)
                {
                    if (cacheAssociation[i][index].blockData.LRU > 0) //if the LRU bit is greater than 0
                        --(cacheAssociation[i][index].blockData.LRU); //subract 1.
                }
                
                //Finally, update the LRU of the replaced block.
                cacheAssociation[indexToUse][index].blockData.LRU = (associativityLevel - 1); //set max val to associativity level
            } //MISS - read mode
            
            else if (mode == 'W' || mode == 'w') //if we are in write mode with a miss
            {
                size_t indexToUse = 0; //first, determine which index to use.  Assume we start with 0.
                for (size_t i = 0; i < associativityLevel; ++i)
                {
                    if (cacheAssociation[i][index].blockData.LRU == 0)
                    {
                        indexToUse = i;
                        break;  //once 0 is found, break out of loop.
                    }
                }
                
                if (cacheAssociation[indexToUse][index].blockData.dirtyBit == 1) //if the block we're about to overwrite is dirty
                {
                    memrefs = 2; //set memrefs =2
                }
                
                cacheAssociation[indexToUse][index].blockData.dirtyBit = 1; //maintain the dirty bit set, as we are writing to the block.
                
                //Now, update the remaining portions of the block.
                cacheAssociation[indexToUse][index].blockData.validBit = 1; //in case it is already not 1
                cacheAssociation[indexToUse][index].blockData.Tag = tag;
                cacheAssociation[indexToUse][index].blockData.Data = 0; //we don't care about the data
                
                //Now, determine what the LRU bits should be set to.
                for (unsigned int i = 0; i < associativityLevel; ++i)
                {
                    if (cacheAssociation[i][index].blockData.LRU > 0) //if the LRU bit is greater than 0
                        --(cacheAssociation[i][index].blockData.LRU); //subract 1.
                }
                
                //Finally, update the LRU of the replaced block.
                cacheAssociation[indexToUse][index].blockData.LRU = (associativityLevel - 1); //set max val to associativity level
            } //MISS - write mode
            
            else //program should never get here
            {
                std::cerr << "A fatal error occurred in MISS where the mode was neither R nor W.\n";
                exit(EXIT_FAILURE);
            }
        } //MISS
        
        //output results
        std::cout << std::right; //right align
        std::cout << std::setw(4) << refCounter;
        std::cout << std::setw(7);
        if (mode == 'R' || mode == 'r')
            std::cout << "read";
        if (mode == 'W' || mode == 'w')
            std::cout << "write";
        std::cout << std::setw(9) << std::hex << address;
        std::cout << std::setw(8) << std::hex << tag;
        std::cout << std::setw(6) << std::dec << index;
        std::cout << std::setw(7) << offset;
        std::cout << std::setw(7);
        if (isThere) //if it was a hit
        {
            std::cout << "hit";
            ++hitCounter;
        }
        else
        {
            std::cout << "miss";
            ++missCounter;
        }
        std::cout << std::setw(8) << memrefs;
        std::cout << "\n"; //newline

    //DumpCache(cacheAssociation,refCounter);
    } //Main Program Loop
    
    PrintSummary(hitCounter,missCounter,refCounter);
} //end main


/* Function Definitions and Implementations */

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



//Prints the summary statistics
void PrintSummary(size_t hitCounter, size_t missCounter, size_t refCounter)
{
    std::cout << "\n\nSimulation Summary Statistics\n";
    std::cout << "-----------------------------\n";
    std::cout << "Total hits       : " << hitCounter << "\n";
    std::cout << "Total misses     : " << missCounter << "\n";
    std::cout << "Total accesses   : " << refCounter << "\n";
    float hitRatio = static_cast<float>(hitCounter) / refCounter;
    float missRatio = static_cast<float>(missCounter) / refCounter;
    std::cout << "Hit ratio        : " << std::fixed << std::setprecision(6) << hitRatio << "\n";
    std::cout << "Miss ratio       : " << std::fixed << std::setprecision(6) << missRatio << "\n\n";
}



//Reads trace.dat file and stores it in a vector for later use
size_t ReadDataTrace(std::vector<std::string> & traceDat)
{
    //NOTE - This will need to be changed for final program when input will be directed via stdin on command line.
    //This program was developed and tested using XCode which does not directly support re-directed input
    std::streambuf *cinBuf, *datBuf;
    cinBuf = std::cin.rdbuf(); //backs up the std::cin buffer.
    
    
    //*************comment out this section to prepare for cin read
    std::ifstream inDatFile("test.dat",std::ios::in);
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

//dumpCache - Used for Debugging
void DumpCache(setAssociation & cacheAssociation, size_t refCounter)
{
    std::cout << "\n\nCache state after reference: " << refCounter << "\n";
    std::cout << "----------" << "\n";
    for (size_t i = 0; i < numSets; ++i)
    {
        std::cout << "Index: " << i << "\n";
        for (size_t j = 0; j < associativityLevel; ++j)
        {
            std::cout << "Valid Bit: " << cacheAssociation[j][i].blockData.validBit << "\t";
        }
        std::cout << std::flush << "\n";
        for (size_t j = 0; j < associativityLevel; ++j)
        {
            std::cout << "Dirty Bit: " << cacheAssociation[j][i].blockData.dirtyBit << "\t";
        }
        std::cout << std::flush << "\n";
        for (size_t j = 0; j < associativityLevel; ++j)
        {
            std::cout << "Tag      : " << cacheAssociation[j][i].blockData.Tag << "\t";
        }
        std::cout << std::flush << "\n";
        for (size_t j = 0; j < associativityLevel; ++j)
        {
            std::cout << "Data     : " << cacheAssociation[j][i].blockData.Data << "\t";
        }
        std::cout << std::flush << "\n";
        for (size_t j = 0; j < associativityLevel; ++j)
        {
            std::cout << "Valid Bit: " << cacheAssociation[j][i].blockData.validBit << "\t";
        }
        std::cout << std::flush << "\n";
        for (size_t j = 0; j < associativityLevel; ++j)
        {
            std::cout << "LRU Bit  : " << cacheAssociation[j][i].blockData.LRU << "\t";
        }
        std::cout << std::flush << "\n";
        std::cout << "\n\n";
    }
}
