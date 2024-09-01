using namespace std;

#include <iostream>
#include <fstream>
#include "pin.H"
#include <vector>
#include <map>


std::ostream* out;


// Define data structures to store information
//std::map<UINT32, UINT64> instructionLengthDistribution;
UINT64 Length[30] = {0};
UINT64 Operand[30]={0};
UINT64 Reg_Read[30]={0};
UINT64 Reg_Write[30]={0};
UINT64 Mem_Opr[50]={0};
UINT64 Mem_R_Opr[50]={0};
UINT64 Mem_W_Opr[50]={0};

UINT64 mem_ins_count = 0; 
UINT64 max_mem_byte_touched =0 ;
UINT64 sum_mem_byte_touched =0 ;

std::vector<INT32> immediateValues;
std::vector<ADDRDELTA> displacementValues;

// Instrumentation functions

// Instrumentation function to collect instruction length data
// VOID InstrumentInstructionLength(INS ins, VOID *v){
//     UINT32 instructionLength = INS_Size(ins);
//     length[instructionLength]++;
// }

// // Instrumentation function to collect operand count data
// VOID InstrumentOperandCount(INS ins, VOID *v)
// {
//     UINT32 operandCount = INS_OperandCount(ins);
//     operandCountDistribution[operandCount]++;
// }

// // Instrumentation function to collect max number of register read operands data
// VOID InstrumentMaxNumRRegs(INS ins, VOID *v)
// {
//     UINT32 maxNumRRegs = INS_MaxNumRRegs(ins);
//     maxNumRRegsDistribution[maxNumRRegs]++;
// }

// // Instrumentation function to collect max number of register write operands data
// VOID InstrumentMaxNumWRegs(INS ins, VOID *v)
// {
//     UINT32 maxNumWRegs = INS_MaxNumWRegs(ins);
//     maxNumWRegsDistribution[maxNumWRegs]++;
// }

// // Instrumentation function to collect memory operand data
// VOID InstrumentMemoryOperands(INS ins, VOID *v)
// {
//     UINT32 memoryOperandCount = INS_MemoryOperandCount(ins);
//     memoryOperandCountDistribution[memoryOperandCount]++;

//     if (memoryOperandCount > 0) {
//         // Instrument only instructions with memory operands
//         for (UINT32 opIdx = 0; opIdx < memoryOperandCount; opIdx++) {
//             if (INS_MemoryOperandIsRead(ins, opIdx)) {
//                 memoryReadOperandCountDistribution[opIdx]++;
//             }
//             if (INS_MemoryOperandIsWritten(ins, opIdx)) {
//                 memoryWriteOperandCountDistribution[opIdx]++;
//             }
//         }
        
//         // Calculate and collect max and average memory bytes touched
//         UINT32 maxBytesTouched = INS_MaxMemoryReadSize(ins) + INS_MaxMemoryWriteSize(ins);
//         memoryBytesTouched.push_back(maxBytesTouched);
//     }
// }

// // Instrumentation function to collect immediate and displacement values
// VOID InstrumentImmediateAndDisplacement(INS ins, VOID *v)
// {
//     for (UINT32 opIdx = 0; opIdx < INS_OperandCount(ins); opIdx++) {
//         if (INS_OperandIsImmediate(ins, opIdx)) {
//             INT32 immediateValue = INS_OperandImmediate(ins, opIdx);
//             immediateValues.push_back(immediateValue);
//         }
//         if (INS_OperandIsMemory(ins, opIdx)) {
//             ADDRDELTA displacementValue = INS_OperandMemoryDisplacement(ins, opIdx);
//             displacementValues.push_back(displacementValue);
//         }
//     }
// }


//UINT64 fast_forward_count;	// Should be a command line input to your PIN tool
UINT64 icount = 0;

KNOB<UINT32> KnobFastForwardIns(KNOB_MODE_WRITEONCE, "pintool", "f", "0", "Specify the value for fast_forward_ins");
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

UINT64 fast_forward_count = 0;

// Analysis routine to track number of instructions executed, and check the exit condition
ADDRINT InsCount()	{
	icount++;
    *out<<icount<<endl;
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

    *out << "Length :"<<endl;
    for(int i=0; i<20; i++){
        *out<<i<<":"<<Length[i]<<endl;
    }
    *out << "Operand :"<<endl;
    for(int i=0; i<20; i++){
        *out<<i<<":"<<Operand[i]<<endl;
    }
    *out << "Reg Read :"<<endl;
    for(int i=0; i<20; i++){
        *out<<i<<":"<<Reg_Read[i]<<endl;
    }
    *out << "Reg Write:"<<endl;
    for(int i=0; i<20; i++){
        *out<<i<<":"<<Reg_Write[i]<<endl;
    }

    *out << "Mem Operand :"<<endl;
    for(int i=0; i<20; i++){
        *out<<i<<":"<<Mem_Opr[i]<<endl;
    }*out << "Mem read op :"<<endl;
    for(int i=0; i<20; i++){
        *out<<i<<":"<<Mem_R_Opr[i]<<endl;
    }*out << "Mem write op :"<<endl;
    for(int i=0; i<20; i++){
        *out<<i<<":"<<Mem_W_Opr[i]<<endl;
    }

    // for (const auto& entry : instructionLengthDistribution) {
    //     *out << "Length " << entry.first << ": Count " << entry.second << "\n";
    // }
    // for(int i=0;i<21;i++){
    //     *out<<i<<":"<<length[i]<<endl;
    // }
    // // Print operand count distribution
    // std::cout << "\nOperand Count Distribution:\n";
    // for (const auto& entry : operandCountDistribution) {
    //     std::cout << "Count " << entry.first << ": Count " << entry.second << "\n";
    // }

    // // Print max number of register read operands distribution
    // std::cout << "\nMax Num RRegs Distribution:\n";
    // for (const auto& entry : maxNumRRegsDistribution) {
    //     std::cout << "Max RRegs " << entry.first << ": Count " << entry.second << "\n";
    // }

    // // Print max number of register write operands distribution
    // std::cout << "\nMax Num WRegs Distribution:\n";
    // for (const auto& entry : maxNumWRegsDistribution) {
    //     std::cout << "Max WRegs " << entry.first << ": Count " << entry.second << "\n";
    // }

    // // Print memory operand count distribution
    // std::cout << "\nMemory Operand Count Distribution:\n";
    // for (const auto& entry : memoryOperandCountDistribution) {
    //     std::cout << "Count " << entry.first << ": Count " << entry.second << "\n";
    // }

    // // Print memory read operand count distribution
    // std::cout << "\nMemory Read Operand Count Distribution:\n";
    // for (const auto& entry : memoryReadOperandCountDistribution) {
    //     std::cout << "Count " << entry.first << ": Count " << entry.second << "\n";
    // }

    // // Print memory write operand count distribution
    // std::cout << "\nMemory Write Operand Count Distribution:\n";
    // for (const auto& entry : memoryWriteOperandCountDistribution) {
    //     std::cout << "Count " << entry.first << ": Count " << entry.second << "\n";
    // }

    // // Print the minimum and maximum immediate values
    // if (!immediateValues.empty()) {
    //     INT32 minImmediate = *std::min_element(immediateValues.begin(), immediateValues.end());
    //     INT32 maxImmediate = *std::max_element(immediateValues.begin(), immediateValues.end());
    //     std::cout << "\nMinimum Immediate Value: " << minImmediate << "\n";
    //     std::cout << "Maximum Immediate Value: " << maxImmediate << "\n";
    // }

    // // Print the minimum and maximum displacement values
    // if (!displacementValues.empty()) {
    //     ADDRDELTA minDisplacement = *std::min_element(displacementValues.begin(), displacementValues.end());
    //     ADDRDELTA maxDisplacement = *std::max_element(displacementValues.begin(), displacementValues.end());
    //     std::cout << "\nMinimum Displacement Value: " << minDisplacement << "\n";
    //     std::cout << "Maximum Displacement Value: " << maxDisplacement << "\n";
    // }
	

}

// Predicated analysis routine
void MyAnalysis(INS ins,UINT32 is, UINT32 iop, UINT32 rr, UINT32 rw) {
	
    Length[is]++;
    Operand[iop]++;
    Reg_Read[rr]++;
    Reg_Write[rw]++;


}
void MyPredicatedAnalysis(INS ins,UINT32 mop_count, UINT32 mor, UINT32 mow, UINT32 mread, UINT32 mwrite) {
	mem_ins_count++;
    Mem_Opr[mop_count]++;
    Mem_R_Opr[mor]++;
    Mem_W_Opr[mow]++;
    UINT64 x = mread + mwrite;
    sum_mem_byte_touched = sum_mem_byte_touched + x;
    max_mem_byte_touched = std::max(x,max_mem_byte_touched);


}

VOID Instruction(INS ins, VOID *v){
    UINT32 ins_size = INS_Size(ins) 
    UINT32 ins_op = INS_OperandCount(ins);
    UINT32 reg_read = INS_MaxNumRRegs(ins);
    UINT32 reg_write = INS_MaxNumWRegs(ins);
	// Instrumentation routine
	INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) Terminate, IARG_END);

	// MyExitRoutine() is called only when the last call returns a non-zero value.
	INS_InsertThenCall(ins, IPOINT_BEFORE, MyExitRoutine, IARG_END);

	// FastForward() is called for every instruction executed
	INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);

    

	// MyPredicatedAnalysis() is called only when the last FastForward() returns a non-zero value.
	INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)MyAnalysis,IARG_INST_PTR,
                            IARG_UINT32,ins_size,
                            IARG_UINT32,ins_op,
                            IARG_UINT32,reg_read,
                            IARG_UINT32,reg_write,
                             IARG_END);


    UINT32 mem_op_count = INS_MemoryOperandCount(ins);
    UINT32 mem_op_r =0;
    UINT32 mem_op_w = 0;
    //UINT32 mem_read_size = 
    if(mem_op_count){
        
        for(UINT32 memop = 0; memop<mem_op_count;memop++){
            if(INS_MemoryOperandIsRead(ins,memop)){
                mem_op_r++;
            }
            else if(INS_MemoryOperandIsWritten(ins,memop)){
                mem_op_w++;
            }
        }
        INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis,IARG_INST_PTR,
                            IARG_UINT32,mem_op_count,
                            IARG_UINT32,mem_op_r,
                            IARG_UINT32,mem_op_w,
                            IARG_MEMORYREAD_SIZE, IARG_MEMORYWRITE_SIZE,
                             IARG_END);
    }
    //INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);


    


	// Instrumentation routine
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsCount, IARG_END);

}


VOID Fini(INT32 code, VOID* v)
{
    MyExitRoutine();
}





int main(int argc, char * argv[])
{
   

	if (PIN_Init(argc, argv))
    {
        //return Usage();
    }

	//fast_forward_count = KnobFastForwardIns.Value() * 1000000000;
	out = new std::ofstream(KnobOutputFile.Value().c_str());
    
    //cout<<KnobFastForwardIns.Value()<<endl;
    fast_forward_count = KnobFastForwardIns.Value();
    //cout<<"f = "<<fast_forward_count<<endl; 
    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}