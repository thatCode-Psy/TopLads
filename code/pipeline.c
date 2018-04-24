/************************************************************************************************/
/* Pipeline Functions ***************************************************************************/
/************************************************************************************************/

/*
 * Dump the current contents of our pipeline.
 */
void iplc_sim_dump_pipeline()
{
    int i;
    
    for (i = 0; i < MAX_STAGES; i++) {
        switch(i) {
            case FETCH:
                printf("(cyc: %u) FETCH:\t %d: 0x%x \t", pipeline_cycles, pipeline[i].itype, pipeline[i].instruction_address);
                break;
            case DECODE:
                printf("DECODE:\t %d: 0x%x \t", pipeline[i].itype, pipeline[i].instruction_address);
                break;
            case ALU:
                printf("ALU:\t %d: 0x%x \t", pipeline[i].itype, pipeline[i].instruction_address);
                break;
            case MEM:
                printf("MEM:\t %d: 0x%x \t", pipeline[i].itype, pipeline[i].instruction_address);
                break;
            case WRITEBACK:
                printf("WB:\t %d: 0x%x \n", pipeline[i].itype, pipeline[i].instruction_address);
                break;
            default:
                printf("DUMP: Bad stage!\n" );
                exit(-1);
        }
    }
}

/*
 * Check if various stages of our pipeline require stalls, forwarding, etc.
 * Then push the contents of our various pipeline stages through the pipeline.
 */
void iplc_sim_push_pipeline_stage()
{
    int i;
    int data_hit=1;
    
    /* 1. Count WRITEBACK stage is "retired" -- This I'm giving you */
    if (pipeline[WRITEBACK].instruction_address) {
        instruction_count++;
        if (debug)
            printf("DEBUG: Retired Instruction at 0x%x, Type %d, at Time %u \n",
                   pipeline[WRITEBACK].instruction_address, pipeline[WRITEBACK].itype, pipeline_cycles);
    }
    
    /* 2. Check for BRANCH and correct/incorrect Branch Prediction */
    if (pipeline[DECODE].itype == BRANCH) {
		int branch_taken = 0; //taken or untaken
		branch_count++; //we have 1 more branch
		if(pipeline[FETCH].instruction_address) { // Not NOP in FETCH
			if(!(pipeline[FETCH].instruction_address - pipeline[DECODE].instruction_address == 4)) { //check if branch taken. AKA the next instruction is not adjacent
				branch_taken = 1;
			}
			if(branch_predict_taken == branch_taken) { //branch predicted correctly
				correct_branch_predictions++;
			} else { //branch predicted incorrectly  TODO:
				//push pipeline forward and insert NOP
				//add miss or not? ??
				iplc_sim_process_pipeline_nop();
			}
		}
    }
    
    /* 3. Check for LW delays due to use in ALU stage and if data hit/miss
     *    add delay cycles if needed.
     */
    int stalling = 0;//flag to skip normal push if pipeline needs to stall
    if (pipeline[MEM].itype == LW) {
        int inserted_nop = 0;
        //below if statements all do basically the same thing; however, different variable names means different if statements for each instruction
        if(pipeline[ALU].itype == SW)//check if instruction in ALU is save word 
        {
            if(pipeline[ALU].stage.sw.src_reg == pipeline[MEM].stage.lw.dest_reg)//check if the source reg is the same as the destination reg for what's in MEM
            {
                inserted_nop = 1//if so, flag that stall is needed
            }
        }
        else if(pipeline[ALU].itype == BRANCH)//check if instruction in ALU is branch
        {
            if(pipeline[ALU].stage.branch.reg1 == pipeline[MEM].stage.lw.dest_reg
                || pipeline[ALU].stage.branch.reg2 == pipeline[MEM].stage.lw.dest_reg)//check if either reg for branch comparison is dest reg for instruction in MEM
            {
                inserted_nop = 1//if so, flag that stall is needed
            }
        }
        else if(pipeline[ALU].itype == RTYPE)//check if instruction in ALU is rtype
        {
            if(pipeline[ALU].stage.rtype.reg1 == pipeline[MEM].stage.lw.dest_reg
                || pipeline[ALU].stage.rtype.reg2_or_constant == pipeline[MEM].stage.lw.dest_reg)//check if either non-dest reg for operation is dest reg for instruction in MEM
            {
                inserted_nop = 1//if so, flag that stall is needed
            }
        }
        else if(pipeline[ALU].itype == LW)//check if instruction in ALU is load word
        {
            if(pipeline[ALU].stage.lw.base_reg == pipeline[MEM].stage.lw.dest_reg)//check if the source reg is the same as the destination reg for what's in MEM
            {
                inserted_nop = 1//if so, flag that stall is needed
            }
        }
        if(iplc_sim_trap_address(pipeline[MEM].stage.lw.data_address) == 0)//check if the adress is a miss
            pipeline_cycles += CACHE_MISS_DELAY;//if so, add 10 to cycles for the stall penalty

        if(inserted_nop == 1)
        {
            pipeline[WRITEBACK] = pipeline[MEM];//push what's in MEM to WB
            pipeline[MEM].itype = NOP;//Make MEM stage instruction for nex cycle NOP
            //Leave other instructions unchanged
            stalling = 1; //update flag
        }

    }
    
    /* 4. Check for SW mem acess and data miss .. add delay cycles if needed */
    if (pipeline[MEM].itype == SW) {
		if(iplc_sim_trap_address(pipeline[MEM].stage.sw.data_address) == 0) 
			pipeline_cycles += CACHE_MISS_DELAY;
    }
    
    /* 5. Increment pipe_cycles 1 cycle for normal processing */
    pipeline_cycles++;
    /* 6. push stages thru MEM->WB, ALU->MEM, DECODE->ALU, FETCH->ALU */
    if(stalling == 0)
    {   
        pipeline[WRITEBACK] = pipeline[MEM];
        pipeline[MEM] = pipeline[ALU];
        pipeline[ALU] = pipeline[DECODE];
        pipeline[DECODE] = pipeline[FETCH];
    }
    // 7. This is a give'me -- Reset the FETCH stage to NOP via bezero */
    bzero(&(pipeline[FETCH]), sizeof(pipeline_t));
}

/*
 * This function is fully implemented.  You should use this as a reference
 * for implementing the remaining instruction types.
 */
void iplc_sim_process_pipeline_rtype(char *instruction, int dest_reg, int reg1, int reg2_or_constant)
{
    /* This is an example of what you need to do for the rest */
    iplc_sim_push_pipeline_stage();
    
    pipeline[FETCH].itype = RTYPE;
    pipeline[FETCH].instruction_address = instruction_address;
    
    strcpy(pipeline[FETCH].stage.rtype.instruction, instruction);
    pipeline[FETCH].stage.rtype.reg1 = reg1;
    pipeline[FETCH].stage.rtype.reg2_or_constant = reg2_or_constant;
    pipeline[FETCH].stage.rtype.dest_reg = dest_reg;
}

void iplc_sim_process_pipeline_lw(int dest_reg, int base_reg, unsigned int data_address)
{
    /* You must implement this function */
	iplc_sim_push_pipeline_stage();
	
	pipeline[FETCH].itype = LW;
    pipeline[FETCH].instruction_address = instruction_address;

    pipeline[FETCH].stage.lw.data_address = data_address;
    pipeline[FETCH].stage.lw.dest_reg = dest_reg;
    pipeline[FETCH].stage.lw.base_reg = base_reg;


}

void iplc_sim_process_pipeline_sw(int src_reg, int base_reg, unsigned int data_address)
{
    /* You must implement this function */
	iplc_sim_push_pipeline_stage();

	pipeline[FETCH].itype = SW;
    pipeline[FETCH].instruction_address = instruction_address;

    pipeline[FETCH].stage.sw.data_address = data_address;
    pipeline[FETCH].stage.sw.src_reg = src_reg;
    pipeline[FETCH].stage.sw.base_reg = base_reg;
}

void iplc_sim_process_pipeline_branch(int reg1, int reg2)
{
    /* You must implement this function */
	iplc_sim_push_pipeline_stage();
	
	pipeline[FETCH].itype = BRANCH;
    pipeline[FETCH].instruction_address = instruction_address;

    pipeline[FETCH].stage.branch.reg1 = reg1;
    pipeline[FETCH].stage.branch.reg2 = reg2;
}

void iplc_sim_process_pipeline_jump(char *instruction)
{
    /* You must implement this function */
	iplc_sim_push_pipeline_stage();
	
	pipeline[FETCH].itype = JUMP;
    pipeline[FETCH].instruction_address = instruction_address;

    strcpy(pipeline[FETCH].stage.jump.instruction, instruction);
}

void iplc_sim_process_pipeline_syscall()
{
    /* You must implement this function */
	iplc_sim_push_pipeline_stage();
	
	pipeline[FETCH].itype = SYSCALL;
    pipeline[FETCH].instruction_address = instruction_address;
}

void iplc_sim_process_pipeline_nop()
{
    /* You must implement this function */
	iplc_sim_push_pipeline_stage();
	
	pipeline[FETCH].itype = NOP;
    pipeline[FETCH].instruction_address = instruction_address;
}