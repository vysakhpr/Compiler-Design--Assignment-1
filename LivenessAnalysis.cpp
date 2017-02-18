#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Use.h"

using namespace llvm;

namespace 
{
  	struct Live : public FunctionPass 
  	{
    	static char ID;
    	Live() : FunctionPass(ID) {}
   		bool runOnFunction(Function &F) override 
   		{
	   		for (Function::iterator block_iterator = F.begin(), block_iterator_end = F.end(); block_iterator != block_iterator_end; ++block_iterator)
   			{
   				//errs() << "Basic block_iterator (name=" << block_iterator->getName() << ") has "<< block_iterator->size() << " instructions.\n------------------------------------------------\n\n";
   				BasicBlock& block= *block_iterator;				
             	for (BasicBlock::iterator i = block.begin(), e = block.end(); i != e ; ++i)
             	{
             		errs() << *i << "\n";
             		Instruction *pi=&*i;
             		errs() << "\t"<< pi->getName();
             		for (Use &U : pi->operands()) 
             		{
  						Value *v = U.get();
  						if(isa<Instruction>(v))
  							errs() << "\t"<< v->getName();
					}
					errs()<<"\n";
             		//errs()<<"\t\t\t\t\t\t"<<vi->getName()<<"\t"<<(pi->getOperand(0))->getName()<<"\n";

             	}

   			}
   			return false;
   		}
	};
}	

char Live::ID = 0;
static RegisterPass<Live> X("live", "Liveness Analysis Pass", false, true);
