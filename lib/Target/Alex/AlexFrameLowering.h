#ifndef LLVM_LIB_TARGET_ALEX_ALEXFRAMELOWERING_H
#define LLVM_LIB_TARGET_ALEX_ALEXFRAMELOWERING_H

#include "llvm/Target/TargetFrameLowering.h"

constexpr int AlexStackAlignment = 8;

namespace llvm {
    class AlexSubtarget;
    class AlexFrameLowering : public TargetFrameLowering {
    protected:
        const AlexSubtarget *subtarget;
    public:
        explicit AlexFrameLowering(const AlexSubtarget *sti);
        virtual void emitPrologue(MachineFunction &MF,
                                  MachineBasicBlock &MBB) const override { }
        virtual void emitEpilogue(MachineFunction &MF,
                                  MachineBasicBlock &MBB) const override { }

        bool hasFP(const MachineFunction &MF) const override {
            return false;
        }
    };
} // End llvm namespace

#endif