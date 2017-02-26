#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/CFG.h"
#include <set>
#include <algorithm>
#include <iterator>

using namespace llvm;


namespace 
{ 	

  struct InstructionSets
  {
    std::set<StringRef> GEN, IN, OUT;
    Instruction *inst;
  };
  struct Uninitvars : public FunctionPass 
  	{
    	static char ID;
      InstructionSets* IS= NULL;
      int instruction_count;
    	Uninitvars() : FunctionPass(ID) {}
   		bool runOnFunction(Function &F) override 
   		{
        instruction_count=0;
        for(inst_iterator i = inst_begin(F);i!=inst_end(F);i++)
        {
          instruction_count++;
        }
        IS = new InstructionSets[instruction_count];
        int count=0;
        for(inst_iterator i = inst_begin(F);i!=inst_end(F);i++,count++)
        {
          Instruction *pi=&*i;
          IS[count].inst=pi;
          std::string s(pi->getOpcodeName());
          if(pi->getName().compare(""))
            if(s.compare("alloca"))
              IS[count].GEN.insert(pi->getName());
        }

        bool flag=false;
        do
        {
          count=0;
          flag=false;
          for (Function::iterator block_iterator = F.begin(), block_iterator_end = F.end(); block_iterator != block_iterator_end; ++block_iterator)
          {
            BasicBlock& block= *block_iterator;       
            for (BasicBlock::iterator i = block.begin(), e = block.end(); i != e ; ++i,count++)
            {
              if(i==block.begin())
              {
                std::set<StringRef> temp;
                for(BasicBlock *pred : predecessors(&block))
                {
                  Instruction *pi=&*(pred->begin());
                  int no=findInstructionNumber(pi);
                  if(no==-1)
                  {
                    errs()<<"ERROR"<<"\n";
                  }
                  else
                  {
                    std::set<StringRef> temp1;
                    std::set_intersection(temp.begin(),temp.end(),IS[no].OUT.begin(),IS[no].OUT.end(),std::inserter(temp1,temp1.end()));
                    temp=temp1;
                  }
                }
                IS[count].IN=temp;
              }
              else
              {
                IS[count].IN=IS[count-1].OUT;
              }
              std::set<StringRef> temp=IS[count].OUT;
              IS[count].OUT=std::set<StringRef>();
              IS[count].OUT.insert(IS[count].IN.begin(),IS[count].IN.end());
              IS[count].OUT.insert(IS[count].GEN.begin(), IS[count].GEN.end());
              if(temp!=IS[count].OUT)
                flag=true;
            }
          }
        }while(flag);

        for(count=0;count<instruction_count;count++)
        {
          errs()<<*(IS[count].inst)<<"\n";
          for(std::set<StringRef>::iterator i=IS[count].OUT.begin();i!=IS[count].OUT.end();i++)
          {
            errs()<<*i<<"\t";
          }
          errs()<<"\n";
        }
        //count=0;
        // std::set<StringRef>::iterator it;
        // for(inst_iterator i = inst_begin(F);i!=inst_end(F);i++,count++)
        // {
        //   Instruction *pi=&*i;
        //   errs()<<*i<<"\t";
        //   for(it=IS[count].GEN.begin();it!=IS[count].GEN.end();++it)
        //     errs()<<*it<<"\t";
        //   errs()<<"\n";
          
        // }


   			return false;
   		}

      int findInstructionNumber(Instruction* b)
      {
        for (int i = 0; i < instruction_count; ++i)
        {
          if(IS[i].inst==b)
          {
            return i;
          }
        }
        return -1;
      }
	};
}	

char Uninitvars::ID = 0;
static RegisterPass<Uninitvars> X("uninitvars", "Uninitialized Variable Analysis Pass", false, true);
