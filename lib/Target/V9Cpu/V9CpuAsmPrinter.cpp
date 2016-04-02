#include "V9CpuAsmPrinter.h"

#include "InstPrinter/V9CpuInstPrinter.h"
#include "V9Cpu.h"
#include "V9CpuInstrInfo.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "v9cpu-asm-printer"

bool V9CpuAsmPrinter::runOnMachineFunction(MachineFunction &MF) {
    V9CpuFI = MF.getInfo<V9CpuFunctionInfo>();
    AsmPrinter::runOnMachineFunction(MF);
    return true;
}

//@EmitInstruction {
//- EmitInstruction() must exists or will have run time error.
void V9CpuAsmPrinter::EmitInstruction(const MachineInstr *MI) {
//@EmitInstruction body {
    if (MI->isDebugValue()) {
        SmallString<128> Str;
        raw_svector_ostream OS(Str);

        PrintDebugValueComment(MI, OS);
        return;
    }

    //@print out instruction:
    //  Print out both ordinary instruction and boudle instruction
    MachineBasicBlock::const_instr_iterator I(MI);
    MachineBasicBlock::const_instr_iterator E = MI->getParent()->instr_end();

    do {

        if (I->isPseudo())
            llvm_unreachable("Pseudo opcode found in EmitInstruction()");

        MCInst TmpInst0;
        MCInstLowering.Lower(&(*I), TmpInst0);
        OutStreamer->EmitInstruction(TmpInst0, getSubtargetInfo());
    } while ((++I != E) && I->isInsideBundle()); // Delay slot check
}
//@EmitInstruction }

//===----------------------------------------------------------------------===//
//
//  V9Cpu Asm Directives
//
//  -- Frame directive "frame Stackpointer, Stacksize, RARegister"
//  Describe the stack frame.
//
//  -- Mask directives "(f)mask  bitmask, offset"
//  Tells the assembler which registers are saved and where.
//  bitmask - contain a little endian bitset indicating which registers are
//            saved on function prologue (e.g. with a 0x80000000 mask, the
//            assembler knows the register 31 (RA) is saved at prologue.
//  offset  - the position before stack pointer subtraction indicating where
//            the first saved register on prologue is located. (e.g. with a
//
//  Consider the following function prologue:
//
//    .frame  $fp,48,$ra
//    .mask   0xc0000000,-8
//       addiu $sp, $sp, -48
//       st $ra, 40($sp)
//       st $fp, 36($sp)
//
//    With a 0xc0000000 mask, the assembler knows the register 31 (RA) and
//    30 (FP) are saved at prologue. As the save order on prologue is from
//    left to right, RA is saved first. A -8 offset means that after the
//    stack pointer subtration, the first register in the mask (RA) will be
//    saved at address 48-8=40.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Mask directives
//===----------------------------------------------------------------------===//
//	.frame	$sp,8,$lr
//->	.mask 	0x00000000,0
//	.set	noreorder
//	.set	nomacro

// Create a bitmask with all callee saved registers for CPU or Floating Point
// registers. For CPU registers consider LR, GP and FP for saving if necessary.
void V9CpuAsmPrinter::printSavedRegsBitmask(raw_ostream &O) {
    // CPU and FPU Saved Registers Bitmasks
    unsigned CPUBitmask = 0;
    int CPUTopSavedRegOff;

    // Set the CPU and FPU Bitmasks
    const MachineFrameInfo *MFI = MF->getFrameInfo();
    const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
    const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
    // size of stack area to which FP callee-saved regs are saved.
    unsigned CPURegSize = V9Cpu::ARegsRegClass.getSize();
    unsigned i = 0, e = CSI.size();

    // Set CPU Bitmask.
    for (; i != e; ++i) {
        unsigned Reg = CSI[i].getReg();
        unsigned RegNum = TRI->getEncodingValue(Reg);
        CPUBitmask |= (1 << RegNum);
    }

    CPUTopSavedRegOff = CPUBitmask ? -CPURegSize : 0;

    // Print CPUBitmask
    O << "\t.mask \t"; printHex32(CPUBitmask, O);
    O << ',' << CPUTopSavedRegOff << '\n';
}

// Print a 32 bit hex number with all numbers.
void V9CpuAsmPrinter::printHex32(unsigned Value, raw_ostream &O) {
    O << "0x";
    for (int i = 7; i >= 0; i--)
        O.write_hex((Value & (0xF << (i*4))) >> (i*4));
}

//===----------------------------------------------------------------------===//
// Frame and Set directives
//===----------------------------------------------------------------------===//
//->	.frame	$sp,8,$lr
//	.mask 	0x00000000,0
//	.set	noreorder
//	.set	nomacro
/// Frame Directive
void V9CpuAsmPrinter::emitFrameDirective() {
    const TargetRegisterInfo &RI = *MF->getSubtarget().getRegisterInfo();

    unsigned stackReg  = RI.getFrameRegister(*MF);
    unsigned returnReg = RI.getRARegister();
    unsigned stackSize = MF->getFrameInfo()->getStackSize();

    if (OutStreamer->hasRawTextSupport())
        OutStreamer->EmitRawText("\t.frame\t$" +
                                 StringRef(V9CpuInstPrinter::getRegisterName(stackReg)).lower() +
                                 "," + Twine(stackSize) + ",$" +
                                 StringRef(V9CpuInstPrinter::getRegisterName(returnReg)).lower());
}

/// Emit Set directives.
const char *V9CpuAsmPrinter::getCurrentABIString() const {
    switch (static_cast<V9CpuTargetMachine &>(TM).getABI().GetEnumValue()) {
        case V9CpuABIInfo::ABI::O32:  return "abiO32";
        case V9CpuABIInfo::ABI::S32:  return "abiS32";
        default: llvm_unreachable("Unknown V9Cpu ABI");
    }
}

//		.type	main,@function
//->		.ent	main                    # @main
//	main:
void V9CpuAsmPrinter::EmitFunctionEntryLabel() {
    if (OutStreamer->hasRawTextSupport())
        OutStreamer->EmitRawText("\t.ent\t" + Twine(CurrentFnSym->getName()));
    OutStreamer->EmitLabel(CurrentFnSym);
}

//  .frame  $sp,8,$pc
//  .mask   0x00000000,0
//->  .set  noreorder
//@-> .set  nomacro
/// EmitFunctionBodyStart - Targets can override this to emit stuff before
/// the first basic block in the function.
void V9CpuAsmPrinter::EmitFunctionBodyStart() {
    MCInstLowering.Initialize(&MF->getContext());

    emitFrameDirective();

    if (OutStreamer->hasRawTextSupport()) {
        SmallString<128> Str;
        raw_svector_ostream OS(Str);
        printSavedRegsBitmask(OS);
        OutStreamer->EmitRawText(OS.str());
        OutStreamer->EmitRawText(StringRef("\t.set\tnoreorder"));
        OutStreamer->EmitRawText(StringRef("\t.set\tnomacro"));
        //if (V9CpuFI->getEmitNOAT())
        //    OutStreamer->EmitRawText(StringRef("\t.set\tnoat"));
    }
}

//->	.set	macro
//->	.set	reorder
//->	.end	main
/// EmitFunctionBodyEnd - Targets can override this to emit stuff after
/// the last basic block in the function.
void V9CpuAsmPrinter::EmitFunctionBodyEnd() {
    // There are instruction for this macros, but they must
    // always be at the function end, and we can't emit and
    // break with BB logic.
    if (OutStreamer->hasRawTextSupport()) {
    //    if (V9CpuFI->getEmitNOAT())
    //        OutStreamer->EmitRawText(StringRef("\t.set\tat"));
        OutStreamer->EmitRawText(StringRef("\t.set\tmacro"));
        OutStreamer->EmitRawText(StringRef("\t.set\treorder"));
        OutStreamer->EmitRawText("\t.end\t" + Twine(CurrentFnSym->getName()));
    }
}

//	.section .mdebug.abi32
//	.previous
void V9CpuAsmPrinter::EmitStartOfAsmFile(Module &M) {
    // FIXME: Use SwitchSection.

    // Tell the assembler which ABI we are using
    if (OutStreamer->hasRawTextSupport())
        OutStreamer->EmitRawText("\t.section .mdebug." +
                                 Twine(getCurrentABIString()));

    // return to previous section
    if (OutStreamer->hasRawTextSupport())
        OutStreamer->EmitRawText(StringRef("\t.previous"));
}

void V9CpuAsmPrinter::PrintDebugValueComment(const MachineInstr *MI,
                                            raw_ostream &OS) {
    // TODO: implement
    OS << "PrintDebugValueComment()";
}

// Force static initialization.
extern "C" void LLVMInitializeV9CpuAsmPrinter() {
    RegisterAsmPrinter<V9CpuAsmPrinter> X(TheV9CpuTarget);
    //RegisterAsmPrinter<V9CpuAsmPrinter> Y(TheV9CpuelTarget);
}