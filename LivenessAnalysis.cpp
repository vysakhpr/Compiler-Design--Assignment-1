#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Use.h"

using namespace llvm;

namespace {
  struct Live : public FunctionPass {
    static char ID;
    Live() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
    	for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
  			errs() << "\t\t\t" << *I << "\n";
  			Instruction *pi= &*I;
  			for (Use &U: pi->operands()){
  				Value *v = U.get();
  				errs()<< *U <<"\t";	
  				}
  			errs()<<"\n";
  			}
      	return false;	
   		}
  	};
}

char Live::ID = 0;
static RegisterPass<Live> X("live", "Liveness Analysis Pass", false, true);
