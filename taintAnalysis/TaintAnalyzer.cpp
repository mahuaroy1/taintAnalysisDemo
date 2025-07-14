#include "llvm/Transforms/Utils/TaintAnalyzer.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <set>
#include <chrono>
#include <thread>
#include <vector>

using namespace llvm;

// set of tainted variables for the analysis
std::set<StringRef> _taintedVars;

bool isSource_Function(StringRef name) {
  return name == "fgets" || name == "scanf";
}

bool isSink_Function(StringRef name) {
  return name == "strcpy" || name == "system";
}

// Propagate taint to if from is tainted
void propagate_Taint(Value *from, Value *to) {
  if (from->hasName() && to->hasName()) {
    _taintedVars.insert(to->getName());   
  }
}

PreservedAnalyses TaintAnalyzerPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  using namespace std::this_thread;     // sleep_for, sleep_until
  using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
  using std::chrono::system_clock;
 
  // For debugging increase the sleep time and attach to the process
  /* sleep_for(1s);
  //  sleep_until(system_clock::now() + 1s);
  llvm::outs() << "sleep over "  << "\n ";
  */
  
  _taintedVars.clear();
  
//  llvm::outs() << "Running taint analysis on function: " << F.getName() << "\n";

  for (auto &BB : F) {
    for (auto &I : BB) {

      // Source need to be compiler withn debug option
      const llvm::DebugLoc &debugInfo = I.getDebugLoc();
      unsigned line = 0;
      unsigned column = 0;
      if (debugInfo) {
        line = debugInfo->getLine();
        column = debugInfo->getColumn();
      } 

      unsigned opcode = I.getOpcode();
      switch (opcode) {
      case llvm::Instruction::Call: {
        CallInst *CI = dyn_cast<CallInst>(&I);
        Function *called = CI->getCalledFunction();
        if (called && isSource_Function(called->getName())) {
          // Loop through the arguments to find tainted arguments, adding the
          // tainted arguments to the taints set.
          int operandIndex = 0;
          for (auto calledArg = CI->arg_begin(); calledArg != CI->arg_end(); ++calledArg) {
            // Have to consider position of argument to reduce FP
            if (operandIndex == 0) {
              // insert 1st argument for now
              _taintedVars.insert(calledArg->get()->getName());
              llvm::outs() << "Found taint source " << calledArg->get()->getName() 
                  <<  " in call instruction in function " << F.getName()  << "\n";

            }
            operandIndex++;
          }
        }
        if (called && isSink_Function(called->getName())) {
          for (auto &Arg : CI->args()) {
            int operandIndex = 0;
            // Need to consider argument position to reduce FP
            auto it = _taintedVars.find(Arg->getName());
            // Check if the element was found
            if (it != _taintedVars.end()) { 
              llvm::outs() << "[WARNING] Tainted variable  "
                           << Arg->getName() <<
                      " reaching sink in call instruction in function " << F.getName() << " \n ";
            }
            operandIndex++;
          }
        }
        break;
      }
      case llvm::Instruction::Store: {
        StoreInst *SI = dyn_cast<StoreInst>(&I);
        propagate_Taint(SI->getValueOperand(), SI->getPointerOperand());
        break;
      }
      case llvm::Instruction::Load: {
        LoadInst *LI = dyn_cast<LoadInst>(&I);
        propagate_Taint(LI->getPointerOperand(), &I);
        break;
      }
      case llvm::Instruction::GetElementPtr: {
        GetElementPtrInst *gepinst = dyn_cast<llvm::GetElementPtrInst>(&I);
        StringRef operand = gepinst->getOperand(0)->getName();
        StringRef result = gepinst->getName();
        propagate_Taint(gepinst->getOperand(0), gepinst->getPointerOperand());
        break;
      }
      default:
        break;
      }
    }
  }

  return PreservedAnalyses::all();
}
