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

  struct BlockInstructions
  {
    std::set<StringRef> use_set,def_set,IN,OUT,LIVE;
  };

	struct Bblocks
	{
		std::set<StringRef> use_set, def_set, IN, OUT;
		BasicBlock *bb;
    BlockInstructions *bi;
	};
  	
  struct Live : public FunctionPass 
  	{
    	static char ID;
    	Live() : FunctionPass(ID) {}
    	Bblocks* BB=NULL;
    	int block_count;
   		bool runOnFunction(Function &F) override 
   		{
   			BB=new Bblocks[F.size()];
   			block_count=F.size();
   			int count=0;
	   		for (Function::iterator block_iterator = F.begin(), block_iterator_end = F.end(); block_iterator != block_iterator_end; ++block_iterator,count++)
   			{
   				BasicBlock& block= *block_iterator;				
          BB[count].bb=&block;
          BB[count].bi=new BlockInstructions[block.size()];
          int instruction_count=0;
          for (BasicBlock::iterator i = block.begin(), e = block.end(); i != e ; ++i,++instruction_count)
          {
            if(block_iterator==F.begin())
            {
              if(i==block.begin())
              {
                for(Argument &A : F.getArgumentList())
                {
                  BB[0].bi[0].def_set.insert(A.getName()); 
                  BB[0].def_set.insert(A.getName());  
                }
              }
            }
            Instruction *pi=&*i;
            std::string s(pi->getOpcodeName());
            if(pi->getName().compare(""))
            {
              if(s.compare("alloca"))
              {
	             BB[count].bi[instruction_count].def_set.insert(pi->getName());
                BB[count].def_set.insert(pi->getName());
              }
            }

            if (!(s.compare("store")))
            {
            //errs()<<*pi<<"\n";
              Value *v = pi->getOperand(0);
              if(v->getName().compare(""))
              {
                BB[count].bi[instruction_count].use_set.insert(v->getName());
                //errs()<<v->getName()<<"\t";
              }
            
              v= pi->getOperand(1);
              if(v->getName().compare(""))
              {
                BB[count].bi[instruction_count].def_set.insert(v->getName());
                BB[count].def_set.insert(v->getName()); 
                //errs()<<v->getName()<<"\n";
              }
            
            }
            else
            {
              for (Use &U : pi->operands()) 
              {
  				   	  Value *v = U.get();
  						  if(isa<Instruction>(v))
  						  {
  							 BB[count].bi[instruction_count].use_set.insert(v->getName());
  						  }
              }
         	  }
          }
          for(instruction_count=block.size()-1;instruction_count>=0;instruction_count--)
          {
            std::set<StringRef> diff;
            std::set_difference(BB[count].bi[instruction_count].OUT.begin(),BB[count].bi[instruction_count].OUT.end(),BB[count].bi[instruction_count].def_set.begin(),BB[count].bi[instruction_count].def_set.end(),std::inserter(diff,diff.end()));
            BB[count].bi[instruction_count].IN=std::set<StringRef>();
            BB[count].bi[instruction_count].IN.insert(BB[count].bi[instruction_count].use_set.begin(),BB[count].bi[instruction_count].use_set.end());
            BB[count].bi[instruction_count].IN.insert(diff.begin(),diff.end());
            if(instruction_count!=0)
            {
              BB[count].bi[instruction_count-1].OUT=BB[count].bi[instruction_count].IN;
            }
          }
          BB[count].use_set=BB[count].bi[0].IN;
   			}        
   			bool flag=false;
   			do
   			{
   				flag=false;
   				for (count=0;count<block_count;count++)
   				{
   					BasicBlock* current_block=BB[count].bb;

   					for (succ_iterator succ = succ_begin(current_block); succ != succ_end(current_block); ++succ) 
   					{
              BasicBlock *successor = *succ;
              int succ_index=findBlockNumber(successor);
 						  if (succ_index==-1)
 						 	   errs()<<"ERROR FINDING OUT SET OF BLOCK"<<"\n";
 						  else
 						  {
 						   	BB[count].OUT.insert(BB[succ_index].IN.begin(),BB[succ_index].IN.end());
 						  }
				  	}

            std::set<StringRef> diff,temp;
 					  std::set_difference(BB[count].OUT.begin(),BB[count].OUT.end(),BB[count].def_set.begin(),BB[count].def_set.end(),std::inserter(diff,diff.end()));
 					  temp=BB[count].IN;
 					  BB[count].IN=std::set<StringRef>();
 					  BB[count].IN.insert(BB[count].use_set.begin(),BB[count].use_set.end());
 					  BB[count].IN.insert(diff.begin(),diff.end());
 					  if(temp!=BB[count].IN)
 					  {
 					    flag=true;
 					  }	
   				}
   			}while(flag);




        count=0;
        unsigned int max=0;
			  for (Function::iterator block_iterator = F.begin(), block_iterator_end = F.end(); block_iterator != block_iterator_end; ++block_iterator,count++)
        {
          BasicBlock& block= *block_iterator;
          std::set<StringRef> LIVE=BB[count].OUT;       
          
          for (int instruction_count=block.size()-1;instruction_count>=0;instruction_count--)
          {
            BB[count].bi[instruction_count].LIVE=LIVE;

            std::set<StringRef> diff;
            std::set_difference(LIVE.begin(),LIVE.end(),BB[count].bi[instruction_count].def_set.begin(),BB[count].bi[instruction_count].def_set.end(),std::inserter(diff,diff.end()));
            LIVE=std::set<StringRef>();
            LIVE.insert(BB[count].bi[instruction_count].use_set.begin(),BB[count].bi[instruction_count].use_set.end());
            LIVE.insert(diff.begin(),diff.end());            
          }

          int instruction_count=0;
          std::set<StringRef>::iterator it;
          for (BasicBlock::iterator i = block.begin(), e = block.end(); i != e ; ++i,instruction_count++)
          {
            errs() << "\t\t"<<*i << "\n\t";
            for (it = BB[count].bi[instruction_count].LIVE.begin(); it != BB[count].bi[instruction_count].LIVE.end(); ++it)
            { 
              if(BB[count].bi[instruction_count].LIVE.size()>max)
                max=BB[count].bi[instruction_count].LIVE.size();
              if(it==BB[count].bi[instruction_count].LIVE.begin())
                errs() <<*it;
              else
                errs() << ", " <<*it;
            }
            errs() <<"\n";
          }  
        }
        max++;
        int* histogram=new int[max]; 			  

        for(unsigned int i=0;i<max;i++)
          histogram[i]=0;

        count=0;
        for (Function::iterator block_iterator = F.begin(), block_iterator_end = F.end(); block_iterator != block_iterator_end; ++block_iterator,count++)
        {
          BasicBlock& block= *block_iterator;
          for (unsigned int i = 0; i <block.size() ; ++i)
          {
            histogram[BB[count].bi[i].LIVE.size()]++;
          }
        }
        errs()<<"\n\n";
        for(unsigned int i=0;i<max;i++)
        {
          errs() << i << "\t" << histogram[i] <<"\n";
        }

   			return false;
   		}

   		int findBlockNumber(BasicBlock* b)
   		{
   			for (int i = 0; i < block_count; ++i)
   			{
   				if(BB[i].bb==b)
   				{
   					return i;
   				}
   			}
   			return -1;
   		}
	};
}	

char Live::ID = 0;
static RegisterPass<Live> X("liveness", "Liveness Analysis Pass", false, true);
