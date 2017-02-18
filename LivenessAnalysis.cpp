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

	struct Bblocks
	{
		std::set<StringRef> use_set, def_set, IN, OUT;
		BasicBlock *bb;

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
   				errs() << "Basic block_iterator (name=" << block_iterator->getName() << ") has "<< block_iterator->size() << " instructions.\n------------------------------------------------\n\n";
   				BasicBlock& block= *block_iterator;				
   				BB[count].bb=&block;
             	for (BasicBlock::iterator i = block.begin(), e = block.end(); i != e ; ++i)
             	{
             		errs() << "\t\t"<<*i << "\n";
             		Instruction *pi=&*i;
             		if(pi->getName().compare(""))
	             		BB[count].def_set.insert(pi->getName());
             		for (Use &U : pi->operands()) 
             		{
  						Value *v = U.get();
  						if(isa<Instruction>(v))
  						{
  							BB[count].use_set.insert(v->getName());
  						}
					}
             	}
   			}

   			//int k=0;
   			bool flag=false;
   			do
   			{
   				flag=false;
   				for (count=0;count<block_count;count++)
   				{
   					BasicBlock* current_block=BB[count].bb;
   					//errs() << current_block ->getName()<<"\n";

   					for (succ_iterator succ = succ_begin(current_block); succ != succ_end(current_block); ++succ) 
   					{
 						 BasicBlock *successor = *succ;
 						 //errs() << current_block ->getName()<<"------>"<<successor->getName()<<"\n";
 						 int succ_index=findBlockNumber(successor);
 						 //errs()<<succ_index<<"\n";
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
   				//errs()<<"Pass "<<k++<<":\n";
   			}while(flag);


			   			



   			std::set<StringRef>::iterator it;
   			for (int j = 0; j < block_count; ++j)
   			{
   				errs() << "IN Set of block" <<j <<"\n------------\n";
   				for (it = BB[j].IN.begin(); it != BB[j].IN.end(); ++it)
				{
    				errs() <<*it <<"\n";
				}

				errs() <<"OUT Set of block" <<j <<"\n------------\n";
   				for (it=BB[j].OUT.begin(); it != BB[j].OUT.end(); ++it)
				{
    				errs() <<*it <<"\n";
				}	
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
static RegisterPass<Live> X("live", "Liveness Analysis Pass", false, true);
