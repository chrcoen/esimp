#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

#include <iostream>

namespace {
struct InstructionCount : public FunctionPass {
  static char ID;
  InstructionCount() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {
    // Get the function to call from our runtime library.
    LLVMContext &Ctx = F.getContext();

    auto name = F.getName();
    std::string filename = F.getParent()->getSourceFileName();
    StringRef filenameref = StringRef(filename);

    /* Ignore special file names */
    if (filenameref.endswith("esimp.cpp") ||
        filenameref.endswith("_noinstr.c") ||
        filenameref.endswith("_noinstr.cpp")) {
      return false;
    }

    /* Ignore special function names */
    if (name.endswith("_noinstr") || name.startswith("_GLOBAL__sub_I_") ||
        name.startswith("__cxx_global_var_init")) {
      return false;
    }

    // std::cout << "Instrumenting " << filename << ":" << name.str().c_str()
    //           << std::endl;

    std::vector<Type *> paramTypes = {Type::getInt32Ty(Ctx)};
    Type *retType = Type::getVoidTy(Ctx);
    FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);
    FunctionCallee resultFunc =
        F.getParent()->getOrInsertFunction("simulation_step", logFuncType);

    for (auto &B : F) {
      int count = 0;
      count += B.size();

      IRBuilder<> builder(&B);
      builder.SetInsertPoint(B.getTerminator());

      Value *args[] = {builder.getInt32(count)};
      builder.CreateCall(resultFunc, args);
    }
    return false;
  }
};
}  // namespace

char InstructionCount::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerInstructionCount(const PassManagerBuilder &m,
                                     legacy::PassManagerBase &PM) {
  PM.add(new InstructionCount());
}
static RegisterStandardPasses RegisterMyPass(
    PassManagerBuilder::EP_EarlyAsPossible, registerInstructionCount);
