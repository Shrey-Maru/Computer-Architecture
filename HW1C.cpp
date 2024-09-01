using namespace std;

#include <iostream>
#include <fstream>
#include "pin.H"
#include <vector>
#include <map>
#include <set>

std::ostream* out;

std::map<ADDRINT, UINT32> instructionSizes; // Map instruction address to size
std::map<ADDRINT, UINT32> dataAccesses;     // Map data address to accessed size



// // Function to capture data memory access
// VOID InstrumentMemoryAccess(ADDRINT addr, UINT32 size)
// {
//     // Capture data address and access size for data accesses
//     dataAccesses[addr] = size;
// }


ofstream OutFile;
//UINT64 fast_forward_count;	// Should be a command line input to your PIN tool
UINT64 icount = 0;

KNOB<UINT64> KnobFastForwardIns(KNOB_MODE_WRITEONCE, "pintool", "f", "0", "Specify the value for fast_forward_ins");
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

UINT64 fast_forward_count = 0;

// Analysis routine to track number of instructions executed, and check the exit condition
ADDRINT InsCount()	{
	icount++;
	//*out<<"i "<<icount<<endl;
	return 0;
}

ADDRINT Terminate(void)
{	//*out<<"t"<<endl;
        return (icount >= fast_forward_count + 1000000000);
}

// Analysis routine to check fast-forward condition
ADDRINT FastForward(void) {
	//*out<<"f"<<endl;
	return ((icount >= fast_forward_count) && icount);
}

// Analysis routine to exit the application
void MyExitRoutine() {

    
     *out<<"hello"<<endl;
     std::set<ADDRINT> ins_foot;

     std::set<ADDRINT> data_foot;

     for (const auto& entry : instructionSizes) {
        ADDRINT addr = entry.first;
        
        UINT32 size = entry.second;
        ins_foot.insert(addr / 32);
        ins_foot.insert((addr + size) / 32);
    }

    for (const auto& entry : dataAccesses) {
        ADDRINT addr = entry.first;
        UINT32 size = entry.second;
        data_foot.insert(addr / 32);
        data_foot.insert((addr + size) / 32);
    }
   
    
    size_t s1 = ins_foot.size();
    size_t s2 = data_foot.size();
	
																								
    *out<<"Ins_foot "<<s1<<endl;
    *out<<"Data_foot "<<s2<<endl;
    
*out<<"success"<<endl;

}

// Predicated analysis routine
void MyPredicatedAnalysis(INS ins,ADDRINT data_addr, UINT32 data_size,ADDRINT ins_addr, UINT32 ins_size ) {
	//*out<<"mypredAna"<<endl;
 
    instructionSizes[ins_addr] = ins_size;
    dataAccesses[data_addr] = data_size;
	// *out<<"p1"<<endl;
    // if(data_size != 0){
    // 	//*out<<"p2"<<endl;
    //     dataAccesses[data_addr] = data_size;
    // }
	
	//*out<<"p3"<<endl;


}

VOID MyPredicatedAnalysis1(INS ins,ADDRINT ins_addr, UINT32 ins_size ) {
	
 
    instructionSizes[ins_addr] = ins_size;



}


VOID Instruction(INS ins, VOID *v){
	// Instrumentation routine
	INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) Terminate, IARG_END);
	//*out<<"1"<<endl;
	// MyExitRoutine() is called only when the last call returns a non-zero value.
	INS_InsertThenCall(ins, IPOINT_BEFORE, MyExitRoutine, IARG_END);
	//*out<<"2"<<endl;
	// FastForward() is called for every instruction executed
	INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);
	//*out<<"3"<<endl;
    

    // Insert instrumentation to capture data memory read accesses
    if (INS_IsMemoryRead(ins))
    {	//*out<<"4"<<endl;
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis,IARG_INST_PTR,
                       IARG_MEMORYREAD_EA,      // Data address
                       IARG_MEMORYREAD_SIZE,    // Data access size in bytes
                       IARG_ADDRINT, INS_Address(ins),
                       IARG_UINT32, INS_Size(ins),
                       IARG_END);
    }

    // Insert instrumentation to capture data memory write accesses
    else if (INS_IsMemoryWrite(ins))
    {	//*out<<"5"<<endl;
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis,IARG_INST_PTR,
                       IARG_MEMORYWRITE_EA,     // Data address
                       IARG_MEMORYWRITE_SIZE,   // Data access size in bytes
                       IARG_ADDRINT, INS_Address(ins),
                       IARG_UINT32, INS_Size(ins),
                       IARG_END);
    }

    else {	//*out<<"6"<<endl;
	    // MyPredicatedAnalysis() is called only when the last FastForward() returns a non-zero value.
	    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis1,IARG_INST_PTR,
                       IARG_ADDRINT, INS_Address(ins),IARG_UINT32, INS_Size(ins),IARG_END);
    }
	//*out<<"7"<<endl;
	// Instrumentation routine
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsCount, IARG_END);
	//*out<<"8"<<endl;
}


VOID Fini(INT32 code, VOID* v)
{
 MyExitRoutine();   
}





int main(int argc, char * argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    fast_forward_count = KnobFastForwardIns.Value();

    out = new std::ofstream(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}