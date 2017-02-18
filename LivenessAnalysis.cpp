#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Use.h"
#include <set>

using namespace llvm;

namespace 
{

	struct Bblocks
	{
		std::set<StringRef> use_set, def_set;
	};

  	struct Live : public FunctionPass 
  	{
    	static char ID;
    	Live() : FunctionPass(ID) {}
    	Bblocks* BB=NULL;
   		bool runOnFunction(Function &F) override 
   		{
   			BB=new Bblocks[F.size()];
   			int count=0;
	   		for (Function::iterator block_iterator = F.begin(), block_iterator_end = F.end(); block_iterator != block_iterator_end; ++block_iterator,count++)
   			{
   				//errs() << "Basic block_iterator (name=" << block_iterator->getName() << ") has "<< block_iterator->size() << " instructions.\n------------------------------------------------\n\n";
   				BasicBlock& block= *block_iterator;				
             	for (BasicBlock::iterator i = block.begin(), e = block.end(); i != e ; ++i)
             	{
             		errs() << *i << "\n";
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

   			/*std::set<StringRef>::iterator it;
   			for (int j = 0; j < count; ++j)
   			{
   				errs() << "Use Set of block" <<j <<"\n------------\n";
   				for (it = BB[j].use_set.begin(); it != BB[j].use_set.end(); ++it)
				{
    				errs() <<*it <<"\n";
				}

				errs() <<"Def Set of block" <<j <<"\n------------\n";
   				for (it=BB[j].def_set.begin(); it != BB[j].def_set.end(); ++it)
				{
    				errs() <<*it <<"\n";
				}	
   			}*/
			
   			return false;
   		}
	};
}	

char Live::ID = 0;
static RegisterPass<Live> X("live", "Liveness Analysis Pass", false, true);
