#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUISELDAGTODAG_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUISELDAGTODAG_H
#include "V9Cpu.h"
#include "V9CpuSubtarget.h"
#include "V9CpuTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// V9CpuDAGToDAGISel - V9CPU specific code to select V9CPU machine
// instructions for SelectionDAG operations.
//===----------------------------------------------------------------------===//
namespace llvm {

    class V9CpuDAGToDAGISel : public SelectionDAGISel {
    public:
        explicit V9CpuDAGToDAGISel(V9CpuTargetMachine &TM)
                : SelectionDAGISel(TM), Subtarget(nullptr) {}

        // Pass Name
        const char *getPassName() const override {
            return "V9CPU DAG->DAG Pattern Instruction Selection";
        }

        bool runOnMachineFunction(MachineFunction &MF) override;

    protected:

        /// Keep a pointer to the V9CpuSubtarget around so that we can make the right
        /// decision when generating code for different targets.
        const V9CpuSubtarget *Subtarget;

    private:
        // Include the pieces autogenerated from the target description.
#include "V9CpuGenDAGISel.inc"

        /// getTargetMachine - Return a reference to the TargetMachine, casted
        /// to the target-specific type.
        const V9CpuTargetMachine &getTargetMachine() {
            return static_cast<const V9CpuTargetMachine &>(TM);
        }

        SDNode *Select(SDNode *N) override;

        virtual std::pair<bool, SDNode*> selectNode(SDNode *Node) = 0;

        // Complex Pattern.
        bool SelectAddr(SDNode *Parent, SDValue N, SDValue &Base, SDValue &Offset);

        // getImm - Return a target constant with the specified value.
        inline SDValue getImm(const SDNode *Node, unsigned Imm) {
            return CurDAG->getTargetConstant(Imm, SDLoc(Node), Node->getValueType(0));
        }

        virtual void processFunctionAfterISel(MachineFunction &MF) = 0;

    };

}

#endif