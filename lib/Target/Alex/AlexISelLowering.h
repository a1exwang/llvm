#ifndef LLVM_LIB_TARGET_ALEX_ALEXISELLOWERING_H
#define LLVM_LIB_TARGET_ALEX_ALEXISELLOWERING_H

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/Function.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/MC/MCExpr.h"
#include "AlexTargetMachine.h"
#include <deque>
#include <MCTargetDesc/AlexBaseInfo.h>

namespace llvm {
namespace AlexISD {
enum NodeType {
  // Start the numbering from where ISD NodeType finishes.
          FIRST_NUMBER = ISD::BUILTIN_OP_END,

  // Jump and link (call)
          JmpLink,

  // Tail call
          TailCall,

  // Get the Higher 16 bits from a 32-bit immediate
  // No relation with Alex Hi register
          Hi,
  // Get the Lower 16 bits from a 32-bit immediate
  // No relation with Alex Lo register
          Lo,

  // Handle gp_rel (small data/bss sections) relocation.
          GPRel,

  // Thread Pointer
          ThreadPointer,

  // Return
          Ret,

  EH_RETURN,

  // DivRem(u)
          DivRem,
  DivRemU,

  Wrapper,
  DynAlloc,

  Sync,
  Push,
  Pop,
  LI32
};
}

//===--------------------------------------------------------------------===//
// TargetLowering Implementation
//===--------------------------------------------------------------------===//
class AlexFunctionInfo;

class AlexSubtarget;

class AlexTargetMachine;

//@class AlexTargetLowering
class AlexTargetLowering : public TargetLowering {

public:
  explicit AlexTargetLowering(const AlexTargetMachine *TM,
                              const AlexSubtarget *STI,
                              const AlexRegisterInfo *
  );

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  /// getTargetNodeName - This method returns the name of a target specific
  //  DAG node.
  const char *getTargetNodeName(unsigned Opcode) const override;

protected:
  const AlexSubtarget *subtarget;

  SDValue lowerGlobalAddress(SDValue Op,
                             SelectionDAG &DAG) const;

  SDValue getGlobalReg(SelectionDAG &DAG, EVT Ty) const;

  SDValue getTargetNode(GlobalAddressSDNode *N, EVT Ty,
                        SelectionDAG &DAG,
                        unsigned Flag) const;

  SDValue getTargetNode(ExternalSymbolSDNode *N, EVT Ty,
                        SelectionDAG &DAG,
                        unsigned Flag) const;

  template<class NodeTy>
  SDValue getAddrGlobal(NodeTy *N, EVT Ty, SelectionDAG &DAG,
                        unsigned Flag, SDValue Chain,
                        const MachinePointerInfo &PtrInfo) const;

protected:

  /// ByValArgInfo - Byval argument information.
  struct ByValArgInfo {
    unsigned FirstIdx; // Index of the first register used.
    unsigned NumRegs;  // Number of registers used for this argument.
    unsigned Address;  // Offset of the stack area used to pass this argument.

    ByValArgInfo() : FirstIdx(0), NumRegs(0), Address(0) { }
  };

  class AlexCC {
  public:
    enum SpecialCallingConvType {
      NoSpecialCallingConv
    };

    AlexCC(CallingConv::ID CallConv, bool IsO32, CCState &Info,
           SpecialCallingConvType SpecialCallingConv = NoSpecialCallingConv);

    void analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins,
                           bool IsSoftFloat, const SDNode *CallNode,
                           const Type *RetTy) const;

    void analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs,
                       bool IsSoftFloat, const Type *RetTy) const;

    void analyzeFormalArguments(const SmallVectorImpl<ISD::InputArg> &Args,
                                bool IsSoftFloat, Function::const_arg_iterator FuncArg);

    const CCState &getCCInfo() const { return CCInfo; }

    void analyzeCallOperands(const SmallVectorImpl<ISD::OutputArg> &Args,
                             bool IsVarArg, bool IsSoftFloat, const SDNode *CallNode,
                             std::vector<ArgListEntry> &FuncArgs);

    void handleByValArg(unsigned ValNo, MVT ValVT,
                        MVT LocVT,
                        CCValAssign::LocInfo LocInfo,
                        ISD::ArgFlagsTy ArgFlags);

    const ArrayRef<MCPhysReg> intArgRegs() const;

    /// hasByValArg - Returns true if function has byval arguments.
    bool hasByValArg() const { return !ByValArgs.empty(); }

    unsigned regSize() const { return 4; }

    unsigned numIntArgRegs() const { return 0; }

    /// reservedArgArea - The size of the area the caller reserves for
    /// register arguments. This is 16-byte if ABI is O32.
    unsigned reservedArgArea() const {
      return 0;
        //return (CallConv != CallingConv::Fast) ? 8 : 0;
    }

    typedef SmallVectorImpl<ByValArgInfo>::const_iterator byval_iterator;

    byval_iterator byval_begin() const { return ByValArgs.begin(); }

    byval_iterator byval_end() const { return ByValArgs.end(); }

  private:

    /// Return the type of the register which is used to pass an argument or
    /// return a value. This function returns f64 if the argument is an i64
    /// value which has been generated as a result of softening an f128 value.
    /// Otherwise, it just returns VT.
    MVT getRegVT(MVT VT, const Type *OrigTy, const SDNode *CallNode,
                 bool IsSoftFloat) const {
        return VT;
    }

    template<typename Ty>
    void analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
                       const SDNode *CallNode, const Type *RetTy) const;

    CCState &CCInfo;
    CallingConv::ID CallConv;
    bool IsO32;
    SmallVector<ByValArgInfo, 2> ByValArgs;
  };

private:
  SDValue LowerFormalArguments(SDValue Chain,
                               CallingConv::ID CallConv, bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               SDLoc dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue chain,
                      CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &outs,
                      const SmallVectorImpl<SDValue> &outVals,
                      SDLoc dl, SelectionDAG &dag) const override;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;
  void passByValArg(SDValue Chain, SDLoc DL,
                    std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
                    SmallVectorImpl<SDValue> &MemOpChains, SDValue StackPtr,
                    MachineFrameInfo *MFI, SelectionDAG &DAG, SDValue Arg,
                    const AlexCC &CC, const ByValArgInfo &ByVal,
                    const ISD::ArgFlagsTy &Flags, bool isLittle) const;
  void copyByValRegs(SDValue Chain, SDLoc DL, std::vector<SDValue> &OutChains,
                     SelectionDAG &DAG, const ISD::ArgFlagsTy &Flags,
                     SmallVectorImpl<SDValue> &InVals, const Argument *FuncArg,
                     const AlexCC &CC, const ByValArgInfo &ByVal) const;

  SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                          CallingConv::ID CallConv, bool IsVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          SDLoc DL, SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals,
                          const SDNode *CallNode,
                          const Type *RetTy) const;
  void getOpndList(SmallVectorImpl<SDValue> &Ops,
                   std::deque<std::pair<unsigned, SDValue> > &RegsToPass,
                   bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
                   CallLoweringInfo &CLI, SDValue Callee, SDValue Chain) const;

  SDValue passArgOnStack(SDValue StackPtr, unsigned Offset,
                         SDValue Chain, SDValue Arg, SDLoc DL,
                         bool IsTailCall, SelectionDAG &DAG) const;

  SDValue lowerJumpTable(SDValue Op, SelectionDAG &DAG) const;

  SDValue getTargetNode(JumpTableSDNode *N, EVT Ty, SelectionDAG &DAG,
                        unsigned Flag) const;

  SDValue getTargetNode(BlockAddressSDNode *N, EVT Ty, SelectionDAG &DAG,
                        unsigned Flag) const;

  SDValue lowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;

  template<class NodeTy>
  SDValue getAddrNonPIC(NodeTy *N, EVT Ty, SelectionDAG &DAG) const {
      SDLoc DL(N);
      SDValue Lo = getTargetNode(N, Ty, DAG, AlexII::MO_ABS_LO);
      SDValue Hi = getTargetNode(N, Ty, DAG, AlexII::MO_ABS_HI);
      return DAG.getNode(ISD::ADD, DL, Ty,
                         DAG.getNode(AlexISD::Hi, DL, Ty, Hi),
                         DAG.getNode(AlexISD::Lo, DL, Ty, Lo));
  }

  void writeVarArgRegs(std::vector<SDValue> &OutChains,
                       const AlexCC &CC, SDValue Chain,
                       SDLoc DL, SelectionDAG &DAG) const;

  SDValue lowerVASTART(SDValue Op, SelectionDAG &DAG) const;

  SDValue lowerSELECT(SDValue Op, SelectionDAG &DAG) const;

  bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const override {
      return false;
  }
  // Inline asm support
  ConstraintType getConstraintType(StringRef Constraint) const override;

  /// Examine constraint string and operand type and determine a weight value.
  /// The operand object must already have been set up with the operand type.
  ConstraintWeight getSingleConstraintMatchWeight(
          AsmOperandInfo &info, const char *constraint) const override;

  /// This function parses registers that appear in inline-asm constraints.
  /// It returns pair (0, 0) on failure.
  std::pair<unsigned, const TargetRegisterClass *>
          parseRegForInlineAsmConstraint(const StringRef &C, MVT VT) const;

  std::pair<unsigned, const TargetRegisterClass *>
          getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                       StringRef Constraint, MVT VT) const override;

  /// LowerAsmOperandForConstraint - Lower the specified operand into the Ops
  /// vector.  If it is invalid, don't add anything to Ops. If hasMemory is
  /// true it means one of the asm constraint of the inline asm instruction
  /// being processed is 'm'.
  void LowerAsmOperandForConstraint(SDValue Op,
                                    std::string &Constraint,
                                    std::vector<SDValue> &Ops,
                                    SelectionDAG &DAG) const override;

  bool isLegalAddressingMode(const DataLayout &DL, const AddrMode &AM,
                             Type *Ty, unsigned AS) const override;
};
}

#endif // AlexISELLOWERING_H