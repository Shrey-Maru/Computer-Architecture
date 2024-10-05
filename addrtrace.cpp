
using namespace std;
#include <stdio.h>
#include "pin.H"
#include <vector>
#include <iostream>
#include <fstream>
FILE * trace;



UINT64 fast_forward_count = 0;



struct CacheBlock1 {
        int state; // 0 for INVALID, 1 for VALID
        unsigned int tag;
        long int hit;
        CacheBlock1() : state(0), tag(0), hit(0) {}
    };
// Cache Class
class CacheL1 {
private:
   
    int cacheSize;
    int blockSize;
    int numSets;
    int associativity;
    
    std::vector<std::vector<CacheBlock1>> sets;
public:long int filled=0;
    long int no_hit=0;
    long int two_hit=0;
    int tag_calc = 1 << (13);
 

    CacheL1(int size, int block_size, int ways)
        : cacheSize(size), blockSize(block_size), associativity(ways) {
        // Calculate the number of sets in the cache
        numSets = cacheSize / (blockSize * associativity);
        sets.resize(numSets, std::vector<CacheBlock1>(associativity, CacheBlock1())); // Initialize all blocks to 0
    }

    // Function to check if a given address is in the cache
    bool isAddressInCache(unsigned int address) {
        int setIndex = (address / blockSize) % numSets;
        // unsigned int tag = address / tag_calc;
        unsigned int tag = address / blockSize;

        for (int i = 0; i < associativity; ++i) {
            if (sets[setIndex][i].state == 1 && sets[setIndex][i].tag == tag) {
                // Cache hit, move the accessed block to the front
                sets[setIndex][i].hit++;
                if (i != 0) {
                    CacheBlock1 accessedBlock = sets[setIndex][i];
                    for (int j = i; j > 0; --j) {
                        sets[setIndex][j] = sets[setIndex][j - 1];
                    }
                    sets[setIndex][0] = accessedBlock;
                }
                return true;
            }
        }

        return false; // Cache miss
    }

    // Function to add a block to the cache using LRU replacement
    CacheBlock1 addBlock(unsigned int address) {
        filled++;
        int setIndex = (address / blockSize) % numSets;
        // unsigned int tag = address / tag_calc;
        unsigned int tag = address / blockSize;
        CacheBlock1 c = sets[setIndex][associativity - 1];
        if(c.state==1 && c.hit == 0) no_hit++;
        if(c.state==1 && c.hit >= 2) two_hit++;
        // unsigned int tag_evicted = sets[setIndex][associativity - 1].tag;
        // int st = sets[setIndex][associativity - 1].state;

        // Check if the cache set is full
        //if (sets[setIndex][associativity - 1].state == 1) {
            // Cache set is full, remove the least recently used block
            for (int i = associativity - 1; i > 0; --i) {
                sets[setIndex][i] = sets[setIndex][i - 1];
            }
        //}

        // Add the new block at the front
        sets[setIndex][0].state = 1;
        sets[setIndex][0].tag = tag;
        sets[setIndex][0].hit = 0;

        return c;
        
    }

    void evictBlockByTag(unsigned int tag) {
        //unsigned int tag = address/tag_calc;
        int setIndex = tag % numSets;
        // for (int setIndex = 0; setIndex < numSets; ++setIndex) {
            for (int way = 0; way < associativity; ++way) {
                if (sets[setIndex][way].state == 1 && sets[setIndex][way].tag == tag) {
                    // Mark the cache block as INVALID
                    if(sets[setIndex][way].state==1 && sets[setIndex][way].hit == 0) no_hit++;
                    if(sets[setIndex][way].state==1 && sets[setIndex][way].hit >= 2) two_hit++;

                    sets[setIndex][way].state = 0;
                    sets[setIndex][way].tag = 0;

                    // Find the first zero block
                    // int zeroBlockIndex = 0;
                    // while (zeroBlockIndex < associativity && sets[setIndex][zeroBlockIndex].state == 1) {
                    //     zeroBlockIndex++;
                    // }

                    // Shift the other blocks to the left
                    for (int i = way; i < associativity - 1; ++i) {
                        sets[setIndex][i] = sets[setIndex][i + 1];
                    }

                    // Place the zero block at the end
                    sets[setIndex][associativity - 1].state = 0;
                    sets[setIndex][associativity - 1].tag = 0;

                    return; // Block found and evicted
                }
            }
        // }
	return;
    }
};

class CacheL2 {
private:
   
    int cacheSize;
    int blockSize;
    int numSets;
    int associativity;
    std::vector<std::vector<CacheBlock1>> sets;
    

public:long int filled=0;
    long int no_hit=0;
    long int two_hit=0;
    int tag_calc = 1 << (16);
    

    CacheL2(int size, int block_size, int ways)
        : cacheSize(size), blockSize(block_size), associativity(ways) {
        // Calculate the number of sets in the cache
        numSets = cacheSize / (blockSize * associativity);
        sets.resize(numSets, std::vector<CacheBlock1>(associativity, CacheBlock1())); // Initialize all blocks to 0
        cout<<numSets<<endl;
    }

    // Function to check if a given address is in the cache
    bool isAddressInCache(unsigned int address) {
        int setIndex = (address / blockSize) % numSets;
        // unsigned int tag = address / tag_calc;
        unsigned int tag = address / blockSize;
        // cout<<"set "<<setIndex<<" tag "<<tag<<" add "<<address<<endl;
        for (int i = 0; i < associativity; ++i) {
            if (sets[setIndex][i].state == 1 && sets[setIndex][i].tag == tag) {
                // Cache hit, move the accessed block to the front
                sets[setIndex][i].hit++;
                if (i != 0) {
                    CacheBlock1 accessedBlock = sets[setIndex][i];
                    for (int j = i; j > 0; --j) {
                        sets[setIndex][j] = sets[setIndex][j - 1];
                    }
                    sets[setIndex][0] = accessedBlock;
                }
                return true;
            }
        }

        return false; // Cache miss
    }

    // Function to add a block to the cache using LRU replacement
    CacheBlock1 addBlock(unsigned int address) {
        filled++;
        int setIndex = (address / blockSize) % numSets;
        // unsigned int tag = address / tag_calc;
        unsigned int tag = address / blockSize;

        CacheBlock1 c = sets[setIndex][associativity - 1];
        if(c.state==1 && c.hit == 0) no_hit++;
        if(c.state==1 && c.hit >= 2) two_hit++;
        

        
            for (int i = associativity - 1; i > 0; --i) {
                sets[setIndex][i] = sets[setIndex][i - 1];
            }
        

        // Add the new block at the front
        sets[setIndex][0].state = 1;
        sets[setIndex][0].tag = tag;
        sets[setIndex][0].hit = 0;

        return c;
        
    }

    
};




class CacheL2_SRRIP {
private:
    
    int cacheSize;
    int blockSize;
    int numSets;
    int associativity;
    

public:
    struct CacheBlock {
        int state; // 0 for INVALID, 1 for VALID
        unsigned int tag;
        long int hit;
        int age;
        CacheBlock() : state(0), tag(0), hit(0), age(3) {}
    };
    std::vector<std::vector<CacheBlock>> sets;
    long int filled=0;
    long int no_hit=0;
    long int two_hit=0;
    int tag_calc = 1 << (16);
    CacheL2_SRRIP(int size, int block_size, int ways)
        : cacheSize(size), blockSize(block_size), associativity(ways) {
        // Calculate the number of sets in the cache
        numSets = cacheSize / (blockSize * associativity);
        sets.resize(numSets, std::vector<CacheBlock>(associativity, CacheBlock())); // Initialize all blocks to 0
    }

    // Function to check if a given address is in the cache
    bool isAddressInCache(unsigned int address) {
        int setIndex = (address / blockSize) % numSets;
        // unsigned int tag = address / tag_calc;
        unsigned int tag = address / blockSize;
        int hit = -1;
        for (int i = 0; i < associativity; i++) {
            if (sets[setIndex][i].state == 1 && sets[setIndex][i].tag == tag) {
                // Cache hit
                hit = i;
                
            }
            if(sets[setIndex][i].age < 3) sets[setIndex][i].age++;
        }
        if(hit>=0){
            sets[setIndex][hit].age=0;
            return true;
        }

        return false; // Cache miss
    }

    // Function to add a block to the cache using LRU replacement
    CacheBlock addBlock(unsigned int address) {
        filled++;
        int setIndex = (address / blockSize) % numSets;
        // unsigned int tag = address / tag_calc;
        unsigned int tag = address / blockSize;
        while(1){
            for(int i=0;i<associativity;i++){

                if(sets[setIndex][i].state == 0){
                    CacheBlock c = sets[setIndex][i];
                    sets[setIndex][i].state =1;
                    sets[setIndex][i].tag = tag;
                    sets[setIndex][i].age = 2;

                    // c.hit = (address % tag_calc) / (1<<13);
                    return c;
                }


                if(sets[setIndex][i].age == 3){
                    CacheBlock c = sets[setIndex][i];
                    sets[setIndex][i].state =1;
                    sets[setIndex][i].tag = tag;
                    sets[setIndex][i].age = 2;

                    // c.hit = (address % tag_calc) / (1<<13);
                    return c;
                }
            }

            for(int i=0;i<associativity;i++){
                sets[setIndex][i].age++;
            }
        }
        // CacheBlock c = sets[setIndex][associativity - 1];     
    }

};

class CacheL2_NRU {
private:
    
    int cacheSize;
    int blockSize;
    int numSets;
    int associativity;
    

public:
    struct CacheBlock {
        int state; // 0 for INVALID, 1 for VALID
        unsigned int tag;
        long int hit;
        int ref;
        CacheBlock() : state(0), tag(0), hit(0), ref(0) {}
    };
    std::vector<std::vector<CacheBlock>> sets;

    long int filled=0;
    long int no_hit=0;
    long int two_hit=0;
    int tag_calc = 1 << (16);

    CacheL2_NRU(int size, int block_size, int ways)
        : cacheSize(size), blockSize(block_size), associativity(ways) {
        // Calculate the number of sets in the cache
        numSets = cacheSize / (blockSize * associativity);
        sets.resize(numSets, std::vector<CacheBlock>(associativity, CacheBlock())); // Initialize all blocks to 0
    }

    // Function to check if a given address is in the cache
    bool isAddressInCache(unsigned int address) {
        int setIndex = (address / blockSize) % numSets;
        // unsigned int tag = address / tag_calc;
        unsigned int tag = address / blockSize;

        for (int i = 0; i < associativity; ++i) {
            if (sets[setIndex][i].state == 1 && sets[setIndex][i].tag == tag) {
                // Cache hit, move the accessed block to the front
                sets[setIndex][i].hit++;
                sets[setIndex][i].ref = 1;
                if (i != 0) {
                    CacheBlock accessedBlock = sets[setIndex][i];
                    for (int j = i; j > 0; --j) {
                        sets[setIndex][j] = sets[setIndex][j - 1];
                    }
                    sets[setIndex][0] = accessedBlock;
                }
                
                for (int i = 0; i < associativity; i++){
                    if(sets[setIndex][i].ref ==0 ) return true;
                }

                for (int i = 1; i < associativity; i++){
                    sets[setIndex][i].ref = 0 ;
                }

                return true;
            }
        }

        return false; // Cache miss
    }

    // Function to add a block to the cache using LRU replacement
    CacheBlock addBlock(unsigned int address) {
        filled++;
        int setIndex = (address / blockSize) % numSets;
        // unsigned int tag = address / tag_calc;
        unsigned int tag = address / blockSize;

        int x=0;CacheBlock c;
        for (int i = 0; i < associativity; i++) {
            if(sets[setIndex][i].state == 0){
                x=i;
                c = sets[setIndex][i];
                sets[setIndex][i].state =1;
                sets[setIndex][i].tag = tag;
                sets[setIndex][i].ref = 1;


                // c.hit = (address % tag_calc) / (1<<13);
                // return c;
            }


            if(sets[setIndex][i].ref == 0){
                x=i;
                c = sets[setIndex][i];
                sets[setIndex][i].state =1;
                sets[setIndex][i].tag = tag;
                sets[setIndex][i].ref = 1;

                // c.hit = (address % tag_calc) / (1<<13);
                // return c;
            }
            if (x != 0) {
                CacheBlock accessedBlock = sets[setIndex][x];
                for (int j = x; j > 0; --j) {
                    sets[setIndex][j] = sets[setIndex][j - 1];
                }
                sets[setIndex][0] = accessedBlock;
            }
        }
        for (int i = 0; i < associativity; i++){
            if(sets[setIndex][i].ref ==0 ){
                // c.hit = (address % tag_calc) / (1<<13);
                return c;
            }
        }

        for (int i = 1; i < associativity; i++){
            sets[setIndex][i].ref = 0 ;
        }
        // c.hit = (address % tag_calc) / (1<<13);
        return c;
    }


};

class Inclusive {
public:
    
    CacheL1 L1;
    CacheL2 L2;

    long int L1_miss=0;
    long int L2_miss=0;
    long int L1_access=0;
    long int L2_access=0;

    Inclusive()
        : L1(64 * 1024, 64, 8), L2(1 * 1024 * 1024, 64, 16) {}



    bool isAddressInCache(unsigned int address) {
        bool in_L1 = L1.isAddressInCache(address); 
        L1_access++;
        bool in_L2 ;
        if(in_L1) return true;
        
        if(!in_L1){
            L1_miss++;
            L2_access++;
            in_L2 = L2.isAddressInCache(address);
        }

        if(in_L2){
            L1.addBlock(address);
            return true;
        }

        if(!in_L1 && !in_L2){
            L2_miss++;
            L1.addBlock(address);
            //L2.addBlock(address);
            CacheBlock1 c = L2.addBlock(address);
            if(c.state){
                //unsigned int tag = (c.tag << 3) | c.hit; 
                L1.evictBlockByTag(c.tag);
            }

            return false; // Cache miss
        }
        return false;
    }
    
};

class Inclusive2 {
public:
    CacheL1 L1;
    CacheL2_SRRIP L2;

    long int L1_miss=0;
    long int L2_miss=0;
    long int L1_access=0;
    long int L2_access=0;

    Inclusive2()
        : L1(64 * 1024, 64, 8), L2(1 * 1024 * 1024, 64, 16) {}



    bool isAddressInCache(unsigned int address) {
        bool in_L1 = L1.isAddressInCache(address); 
        L1_access++;
        bool in_L2 ;
        if(in_L1) return true;
        
        else{
            L1_miss++;
            L2_access++;
            in_L2 = L2.isAddressInCache(address);
        }

        if(in_L2){
            L1.addBlock(address);
            return true;
        }

        if(!in_L1 && !in_L2){
            L2_miss++;
            L1.addBlock(address);
            CacheL2_SRRIP::CacheBlock c = L2.addBlock(address);
            if(c.state){
                // unsigned int tag = (c.tag << 3) | c.hit; 
                L1.evictBlockByTag(c.tag);
            }

            return false; // Cache miss
        }
        return false;
    }
    
};

class Inclusive3 {
public:
    CacheL1 L1;
    CacheL2_NRU L2;

    long int L1_miss=0;
    long int L2_miss=0;
    long int L1_access=0;
    long int L2_access=0;

    Inclusive3()
        : L1(64 * 1024, 64, 8), L2(1 * 1024 * 1024, 64, 16) {}



    bool isAddressInCache(unsigned int address) {
        bool in_L1 = L1.isAddressInCache(address); 
        L1_access++;
        bool in_L2 ;
        if(in_L1) return true;
        
        else{
            L1_miss++;
            L2_access++;
            in_L2 = L2.isAddressInCache(address);
        }

        if(in_L2){
            L1.addBlock(address);
            return true;
        }

        if(!in_L1 && !in_L2){
            L2_miss++;
            L1.addBlock(address);
            CacheL2_NRU::CacheBlock c = L2.addBlock(address);
            if(c.state){
                // unsigned int tag = (c.tag << 3) | c.hit; 
                L1.evictBlockByTag(c.tag);
            }

            return false; // Cache miss
        }
        return false;
    }
    
};
// Print a memory read record
Inclusive A;
Inclusive2 B;
Inclusive3 C;

VOID RecordMemRead(VOID * ip, ADDRINT addr, UINT32 size)
{
    fprintf(trace,"%p: R %p %u\n", ip, (void*)addr, size);
    if(addr / 64 != (addr + size)/64){
        A.isAddressInCache(addr);
        B.isAddressInCache(addr);
        C.isAddressInCache(addr);

        A.isAddressInCache(addr + size);
        B.isAddressInCache(addr + size);
        C.isAddressInCache(addr + size);
    }
    else{
        A.isAddressInCache(addr);
        B.isAddressInCache(addr);
        C.isAddressInCache(addr);
    }
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, ADDRINT  addr, UINT32 size)
{
    fprintf(trace,"%p: W %p %u\n", ip, (void*)addr, size);
    if(addr / 64 != (addr + size)/64){
        A.isAddressInCache(addr);
        B.isAddressInCache(addr);
        C.isAddressInCache(addr);

        A.isAddressInCache(addr + size);
        B.isAddressInCache(addr + size);
        C.isAddressInCache(addr + size);
    }
    else{
        A.isAddressInCache(addr);
        B.isAddressInCache(addr);
        C.isAddressInCache(addr);
    }
}

std::ostream* out;
UINT64 icount = 0;

KNOB<UINT64> KnobFastForwardIns(KNOB_MODE_WRITEONCE, "pintool", "f", "0", "Specify the value for fast_forward_ins");
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

// Analysis routine to track number of instructions executed, and check the exit condition
ADDRINT InsCount()	{
	icount++;
	if(icount % 10000000 ==0) cout<<icount<<endl;
	return 0;
}

ADDRINT Terminate(void)
{
        return (icount >= fast_forward_count + 1000000000);
}

// Analysis routine to check fast-forward condition
ADDRINT FastForward(void) {
	return ((icount >= fast_forward_count) && icount);
}

// Analysis routine to exit the application
void MyExitRoutine() {
	*out<<"ins: "<<icount<<endl;
	*out<<"-----------------------------"<<endl;
    *out<<"Part A"<<endl;
	*out<<"L1 access: "<<A.L1_access<<endl;
	*out<<"L2 access: "<<A.L2_access<<endl;
	*out<<"L1 miss: "<<A.L1_miss<<endl;
	*out<<"L2 miss: "<<A.L2_miss<<endl;
    *out<<"L1"<<endl;
    *out<<"Filled: "<< A.L1.filled <<endl;
	*out<<"No hit: "<<A.L1.no_hit<<endl;
	*out<<"Two hit: "<<A.L1.two_hit<<endl;
    *out<<"L2"<<endl;
	*out<<"Filled: "<< A.L2.filled <<endl;
	*out<<"No hit: "<<A.L2.no_hit<<endl;
	*out<<"Two hit: "<<A.L2.two_hit<<endl;

    *out<<"-----------------------------"<<endl;
    *out<<"Part B"<<endl;
	*out<<"L1 access: "<<B.L1_access<<endl;
	*out<<"L2 access: "<<B.L2_access<<endl;
	*out<<"L1 miss: "<<B.L1_miss<<endl;
	*out<<"L2 miss: "<<B.L2_miss<<endl;

    *out<<"-----------------------------"<<endl;
    *out<<"Part C"<<endl;
	*out<<"L1 access: "<<C.L1_access<<endl;
	*out<<"L2 access: "<<C.L2_access<<endl;
	*out<<"L1 miss: "<<C.L1_miss<<endl;
	*out<<"L2 miss: "<<C.L2_miss<<endl;
	
	

	exit(0);
}


// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    // Instrumentation routine
	INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) Terminate, IARG_END);

	// MyExitRoutine() is called only when the last call returns a non-zero value.
	INS_InsertThenCall(ins, IPOINT_BEFORE, MyExitRoutine, IARG_END);

    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            UINT32 size = INS_MemoryOperandSize(ins, memOp);
            INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);
            INS_InsertThenPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, size,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            UINT32 size = INS_MemoryOperandSize(ins, memOp);
            INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);
            INS_InsertThenPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, size,
                IARG_END);
        }
    }

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsCount, IARG_END);
}

VOID Fini(INT32 code, VOID *v)
{
    fprintf(trace, "#eof\n");
    fclose(trace);
    MyExitRoutine();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

    trace = fopen("addrtrace.out", "w");
    out = new std::ofstream(KnobOutputFile.Value().c_str());
    
    //cout<<KnobFastForwardIns.Value()<<endl;
    fast_forward_count = KnobFastForwardIns.Value();
    cout<<"f = "<<fast_forward_count<<endl; 

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
