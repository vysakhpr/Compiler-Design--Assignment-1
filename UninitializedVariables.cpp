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
    std::set<StringRef> GEN, USE, IN, OUT;
    Instruction *inst;
  };

  struct  UnInit
  {
    StringRef s;
    std::set<Instruction*> u;
  };
  struct Uninitvars : public FunctionPass 
  	{
    	static char ID;
      UnInit* uv_set=NULL;
      InstructionSets* IS= NULL;
      int instruction_count;
      int uv_count=0,uv_set_size=0;
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
        for(Argument &A : F.getArgumentList())
          IS[count].GEN.insert(A.getName());
        for(inst_iterator i = inst_begin(F);i!=inst_end(F);i++,count++)
        {
          Instruction *pi=&*i;
          IS[count].inst=pi;
          //errs()<<*pi<<"\n";  
          std::string s(pi->getOpcodeName());
          if(pi->getName().compare(""))
            if(s.compare("alloca"))
              IS[count].GEN.insert(pi->getName());
          if (!(s.compare("store")))
          {
            //errs()<<*pi<<"\n";
            Value *v = pi->getOperand(0);
            if(v->getName().compare(""))
              IS[count].USE.insert(v->getName());
           // errs()<<v->getName()<<"\t";
            v= pi->getOperand(1);
            if(v->getName().compare(""))
              IS[count].GEN.insert(v->getName());
           // errs()<<v->getName()<<"\n";
          }
          else
          {
            for (Use &U : pi->operands()) 
            {
              Value *v = U.get();
              if(isa<Instruction>(v) || isa<Argument>(v))
              {
                if(v->getName().compare(""))
                  IS[count].USE.insert(v->getName());
              }
            }
          }
        }

        std::set<StringRef> ALL_DEF;
        for (int count=0;count<instruction_count;count++)
        {
          ALL_DEF.insert(IS[count].GEN.begin(), IS[count].GEN.end());
        }

        for (int count=0;count<instruction_count;count++)
        {
          IS[count].OUT.insert(ALL_DEF.begin(),ALL_DEF.end());
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
                  BasicBlock::iterator pr=pred->end();
                  pr--;
                  Instruction *qi=&*(pr);
                  int k = findInstructionNumber(qi);
                  temp.insert(IS[k].OUT.begin(),IS[k].OUT.end());  
                  break;
                }
                
                for(BasicBlock *pred : predecessors(&block))
                {
                  BasicBlock::iterator pr=pred->end();
                  pr--;
                  Instruction *pi=&*(pr);
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

        
        for (count = 0; count < instruction_count; ++count)
        {
          //errs()<<*(IS[count].inst)<<"\n";
          for(std::set<StringRef>::iterator it=IS[count].USE.begin();it!=IS[count].USE.end();it++)
          {
            
            if(IS[count].IN.find(*it)==IS[count].IN.end())
              uv_count++;
              //errs()<<*it<<"\t";
          }
          //errs()<<"\n";
        }

        uv_set=new UnInit[uv_count];

        for (count = 0; count < instruction_count; ++count)
        {
          for(std::set<StringRef>::iterator it=IS[count].USE.begin();it!=IS[count].USE.end();it++)
          {
            
            if(IS[count].IN.find(*it)==IS[count].IN.end())
              {
                StringRef sr=*it;
                if(findUnInitElement(sr)==-1)
                {
                  uv_set[uv_set_size].s=sr;
                  uv_set[uv_set_size].u.insert(IS[count].inst);
                  uv_set_size++;
                }
                else
                {
                  uv_set[findUnInitElement(sr)].u.insert(IS[count].inst);
                } 
              }
          }
        }

        for (int l = 0; l < uv_set_size; ++l)
        {
          errs()<<uv_set[l].s<<"\t\t\t";
          for(std::set<Instruction*>::iterator it=uv_set[l].u.begin();it!=uv_set[l].u.end();it++)
            errs()<<**it<<"\t";
          errs()<<"\n";
        }
        // count=0;
        // std::set<StringRef>::iterator it;
        // for(inst_iterator i = inst_begin(F);i!=inst_end(F);i++,count++)
        // {
        //   Instruction *pi=&*i;
        //   errs()<<*pi<<"\n";
        //   for(it=IS[count].OUT.begin();it!=IS[count].OUT.end();++it)
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

      int findUnInitElement(StringRef s)
      {
        for (int i = 0; i < uv_set_size; ++i)
        {
          if(!(uv_set[i].s.compare(s)))
            return i;
        }
        return -1;
      }
	};
}	

char Uninitvars::ID = 0;
static RegisterPass<Uninitvars> X("uninitvars", "Uninitialized Variable Analysis Pass", false, true);
