; RUN: llc < %s | FileCheck %s --check-prefix=ASM
; RUN: llc < %s -filetype=obj | llvm-dwarfdump - | FileCheck %s --check-prefix=DWARF

; Values in registers should be clobbered by calls, which use a regmask instead
; of individual register def operands.

; ASM: main: # @main
; ASM: #DEBUG_VALUE: main:argc <- %ECX
; ASM: movl $1, x(%rip)
; ASM: callq clobber
; ASM-NEXT: [[argc_range_end:.Ltmp[0-9]+]]:
; Previously LiveDebugValues would claim argc was still in ECX after the call.
; ASM-NOT: #DEBUG_VALUE: main:argc

; argc is the first debug location.
; ASM: .Ldebug_loc1:
; ASM-NEXT: .quad   .Lfunc_begin0-.Lfunc_begin0
; ASM-NEXT: .quad   [[argc_range_end]]-.Lfunc_begin0
; ASM-NEXT: .short  3                       # Loc expr size
; ASM-NEXT: .byte   82                      # super-register DW_OP_reg2
; ASM-NEXT: .byte   147                     # DW_OP_piece
; ASM-NEXT: .byte   4                       # 4

; argc is the first formal parameter.
; DWARF: .debug_info contents:
; DWARF:  DW_TAG_formal_parameter
; DWARF-NEXT:    DW_AT_location [DW_FORM_sec_offset]   ([[argc_loc_offset:0x.*]])
; DWARF-NEXT:    DW_AT_name [DW_FORM_strp]     {{.*}} "argc"

; DWARF: .debug_loc contents:
; DWARF: [[argc_loc_offset]]: Beginning address offset: 0x0000000000000000
; DWARF-NEXT:                    Ending address offset: 0x0000000000000013
; DWARF-NEXT:                     Location description: 52 93 04

; ModuleID = 't.cpp'
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc18.0.0"

@x = common global i32 0, align 4

; Function Attrs: nounwind uwtable
define i32 @main(i32 %argc, i8** nocapture readnone %argv) #0 !dbg !4 {
entry:
  tail call void @llvm.dbg.value(metadata i8** %argv, i64 0, metadata !12, metadata !21), !dbg !22
  tail call void @llvm.dbg.value(metadata i32 %argc, i64 0, metadata !13, metadata !21), !dbg !23
  store volatile i32 1, i32* @x, align 4, !dbg !24, !tbaa !25
  tail call void @clobber() #3, !dbg !29
  store volatile i32 2, i32* @x, align 4, !dbg !30, !tbaa !25
  %0 = load volatile i32, i32* @x, align 4, !dbg !31, !tbaa !25
  %tobool = icmp eq i32 %0, 0, !dbg !31
  br i1 %tobool, label %if.else, label %if.then, !dbg !33

if.then:                                          ; preds = %entry
  store volatile i32 3, i32* @x, align 4, !dbg !34, !tbaa !25
  br label %if.end, !dbg !36

if.else:                                          ; preds = %entry
  store volatile i32 4, i32* @x, align 4, !dbg !37, !tbaa !25
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret i32 0, !dbg !39
}

declare void @clobber()

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #2

attributes #0 = { nounwind uwtable }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!17, !18, !19}
!llvm.ident = !{!20}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.9.0 (trunk 260617) (llvm/trunk 260619)", isOptimized: true, runtimeVersion: 0, emissionKind: 1, enums: !2, subprograms: !3, globals: !14)
!1 = !DIFile(filename: "t.cpp", directory: "D:\5Csrc\5Cllvm\5Cbuild")
!2 = !{}
!3 = !{!4}
!4 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 4, type: !5, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: true, variables: !11)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !7, !8}
!7 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!8 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64, align: 64)
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64, align: 64)
!10 = !DIBasicType(name: "char", size: 8, align: 8, encoding: DW_ATE_signed_char)
!11 = !{!12, !13}
!12 = !DILocalVariable(name: "argv", arg: 2, scope: !4, file: !1, line: 4, type: !8)
!13 = !DILocalVariable(name: "argc", arg: 1, scope: !4, file: !1, line: 4, type: !7)
!14 = !{!15}
!15 = !DIGlobalVariable(name: "x", scope: !0, file: !1, line: 1, type: !16, isLocal: false, isDefinition: true, variable: i32* @x)
!16 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !7)
!17 = !{i32 2, !"Dwarf Version", i32 4}
!18 = !{i32 2, !"Debug Info Version", i32 3}
!19 = !{i32 1, !"PIC Level", i32 2}
!20 = !{!"clang version 3.9.0 (trunk 260617) (llvm/trunk 260619)"}
!21 = !DIExpression()
!22 = !DILocation(line: 4, column: 27, scope: !4)
!23 = !DILocation(line: 4, column: 14, scope: !4)
!24 = !DILocation(line: 5, column: 5, scope: !4)
!25 = !{!26, !26, i64 0}
!26 = !{!"int", !27, i64 0}
!27 = !{!"omnipotent char", !28, i64 0}
!28 = !{!"Simple C/C++ TBAA"}
!29 = !DILocation(line: 6, column: 3, scope: !4)
!30 = !DILocation(line: 7, column: 5, scope: !4)
!31 = !DILocation(line: 8, column: 7, scope: !32)
!32 = distinct !DILexicalBlock(scope: !4, file: !1, line: 8, column: 7)
!33 = !DILocation(line: 8, column: 7, scope: !4)
!34 = !DILocation(line: 9, column: 7, scope: !35)
!35 = distinct !DILexicalBlock(scope: !32, file: !1, line: 8, column: 10)
!36 = !DILocation(line: 10, column: 3, scope: !35)
!37 = !DILocation(line: 11, column: 7, scope: !38)
!38 = distinct !DILexicalBlock(scope: !32, file: !1, line: 10, column: 10)
!39 = !DILocation(line: 13, column: 1, scope: !4)
