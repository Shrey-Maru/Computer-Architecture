using namespace std;

#include <iostream>
#include <fstream>
#include "pin.H"


std::ostream* out;
//UINT64 fast_forward_count;	// Should be a command line input to your PIN tool
UINT64 icount = 0;
UINT64 ins_count[18] = {0};
KNOB<UINT64> KnobFastForwardIns(KNOB_MODE_WRITEONCE, "pintool", "f", "0", "Specify the value for fast_forward_ins");
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

UINT64 fast_forward_count = 0;

// Analysis routine to track number of instructions executed, and check the exit condition
ADDRINT InsCount()	{
	icount++;
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
	// Do an exit system call to exit the application.
	// As we are calling the exit system call PIN would not be able to instrument application end.
	// Because of this, even if you are instrumenting the application end, the Fini function would not
	// be called. Thus you should report the statistics here, before doing the exit system call.

	// Results etc
	
	for(int i =1;i<18;i++){
		*out<<ins_count[i]<<endl;
	}
	for(int i =1;i<18;i++){
		cout<<ins_count[i]<<endl;
	}
	exit(0);
}

// Predicated analysis routine
void MyPredicatedAnalysis(INS ins,UINT32 id,UINT32 size) {
	ins_count[id] = ins_count[id] + size;
	//if(icount== 1){
	//	cout<<"f = "<<fast_forward_count<<endl;
	//}
	if(icount % 100000000 == 0){
		cout<<icount<<endl;
	}
	// analysis code
	
}


VOID Instruction(INS ins, VOID *v){
	UINT32 id = -1;
	UINT32 mem_count = INS_MemoryOperandCount(ins);
	//if(mem_count >0){
	//*out<<"m "<<mem_count<<endl;
	//}
	UINT32 size = 1;
	// Instrumentation routine
	INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) Terminate, IARG_END);

	// MyExitRoutine() is called only when the last call returns a non-zero value.
	INS_InsertThenCall(ins, IPOINT_BEFORE, MyExitRoutine, IARG_END);

	// FastForward() is called for every instruction executed
	INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);

	if(mem_count){
		for(UINT32 memOp = 0; memOp < mem_count; memOp++){
			size = INS_MemoryOperandSize(ins,memOp);
			
			UINT32 size_mod = size % 4;
			size = size/4;
			if(size_mod) size++;
			
			if (INS_MemoryOperandIsRead(ins, memOp)){
				
				id = 1; 
				INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis,IARG_INST_PTR,IARG_UINT32, id,IARG_UINT32, size, IARG_END);
				INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);
				
			}
			//else if (INS_MemoryOperandIsWritten(ins, memOp)){
			else{
				id = 2;
				INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis,IARG_INST_PTR,IARG_UINT32, id,IARG_UINT32, size, IARG_END);
				INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);
			}
		}
	}
	
	
	
	if(INS_Category(ins) == XED_CATEGORY_NOP){
		id=3;size = 1;
	}
	else if(INS_Category(ins) == XED_CATEGORY_CALL) {
	   	if (INS_IsDirectCall(ins)) {
	      		// Increment direct call count by one
	      		id=4;size = 1;
	   	}
	   	else {
	      		// Increment indirect call count by one
	      		id=5;size = 1;
	   	}
	}
	else if(INS_Category(ins) == XED_CATEGORY_RET){
		id=6;size = 1;
	}
	else if(INS_Category(ins) == XED_CATEGORY_UNCOND_BR){
		id=7;size = 1;
	}
	else if(INS_Category(ins) == XED_CATEGORY_COND_BR){
		id=8;size = 1;
	}
	else if(INS_Category(ins) == XED_CATEGORY_LOGICAL){
		id=9;size = 1;
	}
	else if((INS_Category(ins) == XED_CATEGORY_ROTATE) || (INS_Category(ins) == XED_CATEGORY_SHIFT)){
		id=10;size = 1;
	}
	else if(INS_Category(ins) == XED_CATEGORY_FLAGOP){
		id=11;size = 1;
	}
	else if((INS_Category(ins) == XED_CATEGORY_AVX) || (INS_Category(ins) == XED_CATEGORY_AVX2) || (INS_Category(ins) == XED_CATEGORY_AVX2GATHER) || (INS_Category(ins) == XED_CATEGORY_AVX512)){
		id=12;size = 1;
	}
	else if(INS_Category(ins) == XED_CATEGORY_CMOV){
		id=13;size = 1;
	}
	else if((INS_Category(ins) == XED_CATEGORY_MMX) || (INS_Category(ins) == XED_CATEGORY_SSE)){
		id=14;size = 1;
	}
	else if(INS_Category(ins) == XED_CATEGORY_SYSCALL){
		id=15;size = 1;
	}
	else if(INS_Category(ins) == XED_CATEGORY_X87_ALU){
		id=16;size = 1;
	}
	else{
		id=17;size = 1;
	}


	// MyPredicatedAnalysis() is called only when the last FastForward() returns a non-zero value.
	//INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis,IARG_INST_PTR,IARG_PTR, v, IARG_END);
	INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis,IARG_INST_PTR,IARG_UINT32, id,IARG_UINT32, size, IARG_END);
	// Instrumentation routine
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsCount, IARG_END);

}


VOID Fini(INT32 code, VOID* v)
{
    for(int i=1;i<18;i++){
    	*out<<ins_count[i]<<endl;
    }
}
 
INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}                                                 



int main(int argc, char * argv[])
{
    // Initialize pin
    //PIN_Init(argc, argv);

	if (PIN_Init(argc, argv))
    {
        //return Usage();
    }

	//fast_forward_count = KnobFastForwardIns.Value() * 1000000000;
	out = new std::ofstream(KnobOutputFile.Value().c_str());
    
    cout<<KnobFastForwardIns.Value()<<endl;
    fast_forward_count = KnobFastForwardIns.Value();
    cout<<"f = "<<fast_forward_count<<endl; 
    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}