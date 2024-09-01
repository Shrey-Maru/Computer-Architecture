using namespace std;

#include <iostream>
#include <fstream>
#include "pin.H"


UINT32 GHR=0;
// Constants
const int NUM_SETS = 128;
const int WAYS_PER_SET = 4;

UINT64 for_br=0;
UINT64 back_br=0;

// Struct representing a BTB entry
struct BTBEntry {
    bool valid;
    ADDRINT tag;
    ADDRINT target;
    INT64 lru; // LRU counter
};

// Branch Target Buffer (BTB) class
class BTBCache {
public:
	UINT64 pred=0;
	UINT64 mispred=0;
	UINT64 miss=0;
	UINT64 hit=0;
	UINT64 miss_mispred =0;
	BTBEntry cache[NUM_SETS][WAYS_PER_SET];

    BTBCache() {
        for (int i = 0; i < NUM_SETS; ++i) {
            for (int j = 0; j < WAYS_PER_SET; ++j) {
                cache[i][j].valid = false;
                cache[i][j].tag = 0;
                cache[i][j].target = 0;
                cache[i][j].lru = 0;
            }
        }
    }

    
    ADDRINT lookup(ADDRINT pc, ADDRINT actual,UINT32 size ) {
		pred++;
        int setIndex = pc % NUM_SETS;

        // Check if the entry is in the cache
        for (int i = 0; i < WAYS_PER_SET; i++) {
            if (cache[setIndex][i].valid && cache[setIndex][i].tag == pc) {
                // Hit: Update LRU and return target
				hit++;
                cache[setIndex][i].lru++;
				if(cache[setIndex][i].target != actual){
					mispred++;
					update(pc,actual,setIndex,i);
				}
                return cache[setIndex][i].target;
            }
        }

		//MISS
		miss++;
		int min_id = 0;
		for (int i = 0; i < WAYS_PER_SET; i++) {
			if (cache[setIndex][i].lru < cache[setIndex][min_id].lru){
				min_id = i;
			}
            if (cache[setIndex][i].valid  == false) {
                update(pc,actual,setIndex,i);
				if((pc+size)!=actual)miss_mispred++;
                return pc+size;
            }
        }


		update(pc,actual,setIndex,min_id);
		if((pc+size)!=actual) miss_mispred++;
		
		return pc + size;
     
    }

	void update(ADDRINT pc, ADDRINT actual, int set_id, int way_id){
		cache[set_id][way_id].valid =true;
		cache[set_id][way_id].tag = pc;
		cache[set_id][way_id].target =actual;
		cache[set_id][way_id].lru = 0;

	}

};

class BTBCache2 {
public:
	UINT64 pred=0;
	UINT64 mispred=0;
	UINT64 miss=0;
	UINT64 hit=0;
	UINT64 miss_mispred =0;
	BTBEntry cache[NUM_SETS][WAYS_PER_SET];
    BTBCache2() {
        for (int i = 0; i < NUM_SETS; ++i) {
            for (int j = 0; j < WAYS_PER_SET; ++j) {
                cache[i][j].valid = false;
                cache[i][j].tag = 0;
                cache[i][j].target = 0;
                cache[i][j].lru = 0;
            }
        }
    }

    
    ADDRINT lookup(ADDRINT pc, ADDRINT actual,UINT32 size ) {
		pred++;
        int setIndex = (pc ^ GHR) ;
		setIndex &= ((1 << 7) - 1);

        // Check if the entry is in the cache
        for (int i = 0; i < WAYS_PER_SET; i++) {
            if (cache[setIndex][i].valid && cache[setIndex][i].tag == pc) {
                // Hit: Update LRU and return target
				hit++;
                cache[setIndex][i].lru++;
				if(cache[setIndex][i].target != actual){
					mispred++;
					update(pc,actual,setIndex,i);
				}
                return cache[setIndex][i].target;
            }
        }

		//MISS
		miss++;
		int min_id = 0;
		for (int i = 0; i < WAYS_PER_SET; i++) {
			if (cache[setIndex][i].lru < cache[setIndex][min_id].lru){
				min_id = i;
			}
            if (cache[setIndex][i].valid  == false) {
                update(pc,actual,setIndex,i);
				if((pc+size)!=actual)miss_mispred++;
                return pc+size;
            }
        }


		update(pc,actual,setIndex,min_id);
		if((pc+size)!=actual) miss_mispred++;
		
		return pc + size;;
    }

	void update(ADDRINT pc, ADDRINT actual, int set_id, int way_id){
		cache[set_id][way_id].valid =true;
		cache[set_id][way_id].tag = pc;
		cache[set_id][way_id].target =actual;
		cache[set_id][way_id].lru = 0;

	}

};



class BimodalPredictor {
public:
	UINT8 pht[512];  
    UINT64 predictions =0;   
    UINT64 mispredictions=0; 
	UINT64 mispredictions_forward=0;
	UINT64 mispredictions_backward=0;

    BimodalPredictor() {
        
		for (UINT32 i = 0; i < 512; ++i) {
            pht[i] = 0;  
        }
    }

    

    void UpdatePredictor(ADDRINT index, bool outcome) {
        UINT8& counter = pht[index];

        if (outcome) {
            if (counter < 3) {
                counter++;
            }
        } else {
            if (counter > 0) {
                counter--;
            }
        }
    }

    bool PredictOutcome(ADDRINT index) {
        return pht[index] >= 2;
    }

    void InstrumentBranch( ADDRINT index, bool outcome, int flag) {
        predictions++;
		
        bool predicted = PredictOutcome(index);
        UpdatePredictor(index, outcome);

        if (predicted != outcome) {
            ++mispredictions;
			if(flag) mispredictions_backward++;
			else mispredictions_forward++;
        }
    }
    
};

class SAg {
public:
	UINT32 bht[1024];
    UINT32 pht[512];  
    UINT64 predictions=0;   
    UINT64 mispredictions=0; 
	UINT64 mispredictions_forward=0;
	UINT64 mispredictions_backward=0;
    SAg() {
        
		for (UINT32 i = 0; i < 512; ++i) {
            pht[i] = 0;  
        }
		for (UINT32 i = 0; i < 1024; ++i) {
            bht[i] = 0;  
        }
    }

    

    void UpdatePredictor(ADDRINT index, bool outcome) { //index is ins%1024
        UINT32& counter = pht[bht[index]];
		

		bht[index] = (bht[index] << 1) | (outcome ? 1 : 0);
		bht[index] &= ((1 << 9) - 1);
        if (outcome) {
            if (counter < 3) {
                counter++;
            }
        } else {
            if (counter > 0) {
				counter--;
            }
        }
    }

    bool PredictOutcome(ADDRINT index) {
        return pht[bht[index]] >= 2;
    }

    void InstrumentBranch(ADDRINT index, bool outcome, int flag) {
        predictions++;
		
        
        bool predicted = PredictOutcome(index);

        UpdatePredictor(index, outcome);

        if (predicted != outcome) {
            mispredictions++;
			if(flag) mispredictions_backward++;
			else mispredictions_forward++;
        }
    }	
};

class GAg {
public:
	UINT32 bht;
    UINT32 pht[512];  // Pattern History Table (2-bit counters)
    UINT64 predictions=0;   // Total predictions
    UINT64 mispredictions=0; // Total mispredictions
	UINT64 mispredictions_forward=0;
	UINT64 mispredictions_backward=0;

    GAg() {
		for (UINT32 i = 0; i < 512; i++) {
            pht[i] = 0;  
        }
		bht =0;
    }


    void UpdatePredictor( bool outcome) {
        UINT32& counter = pht[bht];
		//UINT32& bht_c = bht[index];

		bht = (bht << 1) | (outcome ? 1 : 0);
		bht &= ((1 << 9) - 1);
        if (outcome) {
            if (counter < 7) {
				counter++;
            }
        } else {
            if (counter > 0) {
                counter--;
            }
        }
    }

    bool PredictOutcome() {
        return pht[bht] >= 4;
    }

    void InstrumentBranch( bool outcome, int flag) {
        predictions++;
		
        bool predicted = PredictOutcome();

        UpdatePredictor(outcome);

        // Check for misprediction
        if (predicted != outcome) {
            mispredictions++;
			if(flag) mispredictions_backward++;
			else mispredictions_forward++;
        }
    }

};

class Gshare {
public:
	UINT32 bht;
    UINT32 pht[512];  
    UINT64 predictions=0;   
    UINT64 mispredictions=0; 
	UINT64 mispredictions_forward=0;
	UINT64 mispredictions_backward=0;
    Gshare() {
        for (UINT32 i = 0; i < 512; i++) {
            pht[i] = 0;  // Initialize counters to 0
        }
		bht =0;
    }


    void UpdatePredictor(ADDRINT index, bool outcome) {
		ADDRINT ind = index ^ bht;
		ind &= ((1 << 9) - 1);
        UINT32& counter = pht[ind];
		//UINT32& bht_c = bht[index];

		bht = (bht << 1) | (outcome ? 1 : 0);
		bht &= ((1 << 9) - 1);
        if (outcome) {
            if (counter < 7) {
                counter++;
            }
        } else {
            if (counter > 0) {
                counter--;
            }
        }
    }

    bool PredictOutcome(ADDRINT index) {
		ADDRINT ind = index ^ bht;
		ind &= ((1 << 9) - 1); 
        return pht[ind] >= 4;
    }

    void InstrumentBranch( ADDRINT index, bool outcome, int flag) {
        predictions++;
		
        bool predicted = PredictOutcome(index);

        UpdatePredictor(index, outcome);

        if (predicted != outcome) {
            ++mispredictions;
			if(flag) mispredictions_backward++;
			else mispredictions_forward++;
        }
    }

};

class Hybrid1{
public:
	SAg s;
	GAg g;
	UINT64 predictions=0;   
    UINT64 mispredictions=0; 
	UINT64 mispredictions_forward=0;
	UINT64 mispredictions_backward=0;
	UINT32 meta[512];
	UINT32 gh=0;

	Hybrid1() {
      
		for (UINT32 i = 0; i < 512; ++i) {
            meta[i] = 0;
        }
    }


	void UpdatePredictor1(ADDRINT index, bool outcome) { //index is ins here

        s.UpdatePredictor((index%1024),outcome);
		g.UpdatePredictor(outcome);

		gh = (gh << 1) | (outcome ? 1 : 0); //update Global history reg 9 bit
		gh &= ((1 << 9) - 1);

		bool s1 = s.PredictOutcome(index%1024);
		bool g1 = g.PredictOutcome();

		UINT32& counter = meta[gh];
		if(s1 != g1){
			if(s1 == outcome){
				if(counter>0){
					counter--;
				}
			}
			else{
				if(counter<3){
					counter++;
				}
			}

		}
        
    }

	bool PredictOutcome1(ADDRINT index) {
        UINT32 c = meta[gh];
		if(c >= 2){
			return g.PredictOutcome();
		}
		else{
			return s.PredictOutcome(index%1024);  //sag requires index ie % 1024
		}
    }


	void InstrumentBranch(ADDRINT index, bool outcome, int flag) {
        predictions++;
		
        // Predict the outcome
        bool predicted = PredictOutcome1(index);

        // Update the predictor based on the actual outcome
        UpdatePredictor1(index, outcome);

        // Check for misprediction
        if (predicted != outcome) {
            mispredictions++;
			if(flag) mispredictions_backward++;
			else mispredictions_forward++;
        }
    }
	
};

class Hybrid2a{
public:
	SAg s;
	GAg g;
	Gshare gs;

	UINT64 predictions=0;   
    UINT64 mispredictions=0; 
	UINT64 mispredictions_forward=0;
	UINT64 mispredictions_backward=0;
	

	Hybrid2a() {
        
    }

    

	void UpdatePredictor1(ADDRINT index, bool outcome) { //index is ins here

        s.UpdatePredictor((index%1024),outcome);
		g.UpdatePredictor(outcome);
		gs.UpdatePredictor(index,outcome);
        
    }

	bool PredictOutcome1(ADDRINT index) {
		bool s1 = s.PredictOutcome(index%1024);
		bool g1 = g.PredictOutcome();
		bool gs1 = gs.PredictOutcome(index);

		int c=0;
		if(s1) c++;
		if(g1) c++;
		if(gs1) c++;

		if(c>=2) return true;
		else return false;
    }


	void InstrumentBranch(ADDRINT index, bool outcome, int flag) {
        predictions++;
		
        bool predicted = PredictOutcome1(index);

        UpdatePredictor1(index, outcome);

        if (predicted != outcome) {
            mispredictions++;
			if(flag) mispredictions_backward++;
			else mispredictions_forward++;
        }
    }
	
};

class Hybrid2b{
public:
	SAg s;
	GAg g;
	Gshare gs;

	UINT64 predictions=0;   
    UINT64 mispredictions=0; 
	UINT64 mispredictions_forward=0;
	UINT64 mispredictions_backward=0;
	UINT32 meta1[512]; //SAg , GAg
	UINT32 meta2[512]; //GAg , gshare
	UINT32 meta3[512]; //SAg , gshare
	UINT32 gh=0;

	Hybrid2b() {
        
		for (UINT32 i = 0; i < 512; ++i) {
            meta1[i] = 0;  
			meta2[i] = 0;
			meta3[i] = 0;
        }
    }



	void UpdatePredictor1(ADDRINT index, bool outcome) { //index is ins here

        s.UpdatePredictor((index%1024),outcome);
		g.UpdatePredictor(outcome);
		gs.UpdatePredictor(index,outcome);

		gh = (gh << 1) | (outcome ? 1 : 0); //update Global history reg 9 bit
		gh &= ((1 << 9) - 1);

		bool s1 = s.PredictOutcome(index%1024);
		bool g1 = g.PredictOutcome();
		bool gs1 = gs.PredictOutcome(index);

		UINT32& counter1 = meta1[gh];
		if(s1 != g1){
			if(s1 == outcome){
				if(counter1>0){
					counter1--;
				}
			}
			else{
				if(counter1<3){
					counter1++;
				}
			}

		}

		UINT32& counter2 = meta2[gh];
		if(g1 != gs1){
			if(g1 == outcome){
				if(counter2>0){
					counter2--;
				}
			}
			else{
				if(counter2<3){
					counter2++;
				}
			}

		}

		UINT32& counter3 = meta3[gh];
		if(s1 != gs1){
			if(s1 == outcome){
				if(counter3>0){
					counter3--;
				}
			}
			else{
				if(counter3<3){
					counter3++;
				}
			}

		}
        
    }

	bool PredictOutcome1(ADDRINT index) {
        UINT32 c = meta1[gh];
		//int W=0;      // SAg=1, GAg=2, Gshare=3

		bool s1 = s.PredictOutcome(index%1024);
		bool g1 = g.PredictOutcome();
		bool gs1 = gs.PredictOutcome(index);


		if(c >= 2){
			//W=2;
			if(meta2[gh]>=2) return gs1;
			else return g1;
		}
		else{
			//W=1;  //sag requires index ie % 1024
			if(meta3[gh]>=2) return gs1;
			else return s1;
		}

	
	
    }


	void InstrumentBranch(ADDRINT index, bool outcome, int flag) {
        predictions++;
		
        bool predicted = PredictOutcome1(index);

        UpdatePredictor1(index, outcome);

        // Check for misprediction
        if (predicted != outcome) {
            mispredictions++;
			if(flag) mispredictions_backward++;
			else mispredictions_forward++;
        }
    }
	
};

std::ostream* out;
UINT64 icount = 0;

KNOB<UINT64> KnobFastForwardIns(KNOB_MODE_WRITEONCE, "pintool", "f", "0", "Specify the value for fast_forward_ins");
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

UINT64 fast_forward_count = 0;

UINT64 cond_br =0;
UINT64 indirect =0;
UINT64 fnbt_pred = 0;
UINT64 fnbt_mispred = 0;
UINT64 fnbt_mispred_forward = 0;
UINT64 fnbt_mispred_backward = 0;

BimodalPredictor bimodalPredictor;
SAg sag;
GAg gag;
Gshare gshare;
Hybrid1 h1;
Hybrid2a h2a;
Hybrid2b h2b;
BTBCache b1;
BTBCache2 b2;
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
	*out<<"Ins count: "<<icount<<endl;
	*out<<"Cond br :"<<cond_br<<endl;
	*out<<"Indirect:"<<indirect<<endl;
	*out<<"Backward: "<<back_br<<endl;
	*out<<"Forward: "<< for_br <<endl;
	*out<<"-----------------------------"<<endl;
	*out<<"FNBT"<<endl;
	*out<<"Total Pred: "<<fnbt_pred<<endl;
	*out<<"Mispred: "<<fnbt_mispred<<endl;
	*out<<"Backward mispred: "<< fnbt_mispred_backward <<std::endl;
	*out<<"Forward mispred: "<< fnbt_mispred_forward <<std::endl;
	*out<<"-----------------------------"<<endl;
	*out<<"Bimodal"<<endl;
	*out<<"pred :"<<bimodalPredictor.predictions<<endl;
	*out<<"mispred:"<<bimodalPredictor.mispredictions<<endl;
	*out<<"mispred_back: "<<bimodalPredictor.mispredictions_backward<<endl;
	*out<<"mispred_forw: "<<bimodalPredictor.mispredictions_forward<<endl;

	*out<<"-----------------------------"<<endl;
	*out<<"SAg"<<endl;
	*out<<"pred :"<<sag.predictions<<endl;
	*out<<"mispred:"<<sag.mispredictions<<endl;
	*out<<"mispred_back: "<<sag.mispredictions_backward<<endl;
	*out<<"mispred_forw: "<<sag.mispredictions_forward<<endl;
	

	*out<<"-----------------------------"<<endl;
	*out<<"GAg"<<endl;
	*out<<"pred :"<<gag.predictions<<endl;
	*out<<"mispred:"<<gag.mispredictions<<endl;
	*out<<"mispred_back: "<<gag.mispredictions_backward<<endl;
	*out<<"mispred_forw: "<<gag.mispredictions_forward<<endl;

	*out<<"-----------------------------"<<endl;
	*out<<"Gshare"<<endl;
	*out<<"pred :"<<gshare.predictions<<endl;
	*out<<"mispred:"<<gshare.mispredictions<<endl;
	*out<<"mispred_back: "<<gshare.mispredictions_backward<<endl;
	*out<<"mispred_forw: "<<gshare.mispredictions_forward<<endl;

	*out<<"-----------------------------"<<endl;
	*out<<"Hybrid1"<<endl;
	*out<<"pred :"<<h1.predictions<<endl;
	*out<<"mispred:"<<h1.mispredictions<<endl;
	*out<<"mispred_back: "<<h1.mispredictions_backward<<endl;
	*out<<"mispred_forw: "<<h1.mispredictions_forward<<endl;

	*out<<"-----------------------------"<<endl;
	*out<<"Hybrid2a"<<endl;
	*out<<"pred :"<<h2a.predictions<<endl;
	*out<<"mispred:"<<h2a.mispredictions<<endl;
	*out<<"mispred_back: "<<h2a.mispredictions_backward<<endl;
	*out<<"mispred_forw: "<<h2a.mispredictions_forward<<endl;

	*out<<"-----------------------------"<<endl;
	*out<<"Hybrid2b"<<endl;
	*out<<"pred :"<<h2b.predictions<<endl;
	*out<<"mispred:"<<h2b.mispredictions<<endl;
	*out<<"mispred_back: "<<h2b.mispredictions_backward<<endl;
	*out<<"mispred_forw: "<<h2b.mispredictions_forward<<endl;

	*out<<"-----------------------------"<<endl;
	*out<<"BTB1"<<endl;
	*out<<"pred :"<<b1.pred<<endl;
	*out<<"mispred:"<<b1.mispred<<endl;
	*out<<"miss_mispred:"<<b1.miss_mispred<<endl;
	*out<<"hit: "<<b1.hit<<endl;
	*out<<"miss: "<<b1.miss<<endl;

	*out<<"-----------------------------"<<endl;
	*out<<"BTB2"<<endl;
	*out<<"pred :"<<b2.pred<<endl;
	*out<<"mispred:"<<b2.mispred<<endl;
	 *out<<"miss_mispred:"<<b2.miss_mispred<<endl;
	*out<<"hit: "<<b2.hit<<endl;
	*out<<"miss: "<<b2.miss<<endl;

	exit(0);
}


//function to instrument direction predictor
VOID FNBT(ADDRINT ins, ADDRINT br, bool actual){
	GHR = (GHR << 1) | (actual ? 1 : 0);
	GHR &= ((1 << 7) - 1);

	fnbt_pred++;
	ADDRINT index_bi = ins % 512;
	ADDRINT index_sag = ins % 1024;
	//ADDRINT index_gshare = br 
	//*out<<"INS "<<ins<<" fnbt_pred "<<fnbt_pred<<endl;
	if(ins > br){ //backward 
		back_br++;
		if(!actual){
			fnbt_mispred++;
			fnbt_mispred_backward++;
		}
		
		bimodalPredictor.InstrumentBranch(index_bi,actual,1);
		sag.InstrumentBranch(index_sag,actual,1);
		gag.InstrumentBranch(actual,1);
		gshare.InstrumentBranch(ins,actual,1);
		h1.InstrumentBranch(ins,actual,1);
		h2a.InstrumentBranch(ins,actual,1);
		h2b.InstrumentBranch(ins,actual,1);
	}
	else{
		for_br++;
		if(actual){
			fnbt_mispred++;
			fnbt_mispred_forward++;
		}
		
		bimodalPredictor.InstrumentBranch(index_bi,actual,0);
		sag.InstrumentBranch(index_sag,actual,0);
		gag.InstrumentBranch(actual,0);
		gshare.InstrumentBranch(ins,actual,0);
		h1.InstrumentBranch(ins,actual,0);
		h2a.InstrumentBranch(ins,actual,0);
		h2b.InstrumentBranch(ins,actual,0);
	}


}

VOID BTB(ADDRINT ins, ADDRINT br, UINT32 size){
		b1.lookup(ins,br,size);
		b2.lookup(ins,br,size);
}

VOID Instruction(INS ins, VOID *v){
	
	// Instrumentation routine
	INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) Terminate, IARG_END);

	// MyExitRoutine() is called only when the last call returns a non-zero value.
	INS_InsertThenCall(ins, IPOINT_BEFORE, MyExitRoutine, IARG_END);

	// FastForward() is called for every instruction executed
	//INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);

	if(INS_Category(ins) == XED_CATEGORY_COND_BR){
		cond_br++;
		INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);
		INS_InsertThenCall(ins, IPOINT_BEFORE,(AFUNPTR)FNBT, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
	}

	if(INS_IsIndirectControlFlow(ins)){
		indirect++;
		INS_InsertIfCall(ins, IPOINT_BEFORE,(AFUNPTR) FastForward, IARG_END);
		INS_InsertThenCall(ins, IPOINT_BEFORE,(AFUNPTR)BTB, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR,IARG_UINT32,INS_Size(ins),IARG_END);
	}


	// MyPredicatedAnalysis() is called only when the last FastForward() returns a non-zero value.
	//INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis,IARG_INST_PTR,IARG_PTR, v, IARG_END);
	//INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)MyPredicatedAnalysis,IARG_INST_PTR,IARG_UINT32, id,IARG_UINT32, size, IARG_END);
	// Instrumentation routine
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsCount, IARG_END);

}


VOID Fini(INT32 code, VOID* v)
{
    MyExitRoutine();
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
    
    //cout<<KnobFastForwardIns.Value()<<endl;
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



