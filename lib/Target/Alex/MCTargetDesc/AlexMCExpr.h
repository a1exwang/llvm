//===-- AlexMCExpr.h - Alex specific MC expression classes ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCEXPR_H
#define LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCEXPR_H


#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCValue.h"

namespace llvm {

class AlexMCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_Alex_None,
    VK_Alex_LO,
    VK_Alex_HI
  };

private:
  const VariantKind Kind;
  const MCExpr *Expr;

  explicit AlexMCExpr(VariantKind Kind, const MCExpr *Expr)
          : Kind(Kind), Expr(Expr) {}

public:
  static bool isSupportedBinaryExpr(MCSymbolRefExpr::VariantKind VK,
                                    const MCBinaryExpr *BE);

  static const AlexMCExpr *create(MCSymbolRefExpr::VariantKind VK,
                                  const MCExpr *Expr, MCContext &Ctx);

  /// getOpcode - Get the kind of this expression.
  VariantKind getKind() const { return Kind; }

  /// getSubExpr - Get the child of this expression.
  const MCExpr *getSubExpr() const { return Expr; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res,
                                 const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  // There are no TLS AlexMCExprs at the moment.
  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override {}

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }
};
} // end namespace llvm

#endif