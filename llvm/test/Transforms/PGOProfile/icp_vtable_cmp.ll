; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --version 5

; Tests that ICP compares vtables by checking IR.
; RUN: opt < %s -passes='pgo-icall-prom' -pass-remarks=pgo-icall-prom -enable-vtable-profile-use -icp-max-num-vtable-last-candidate=2 -S 2>&1 | FileCheck %s --check-prefixes=VTABLE-COMMON,VTABLE-CMP
; Require exactly one vtable candidate for each function candidate. Tests that ICP compares function by checking IR.
; RUN: opt < %s -passes='pgo-icall-prom' -pass-remarks=pgo-icall-prom -enable-vtable-profile-use -icp-max-num-vtable-last-candidate=1 -S 2>&1 | FileCheck %s --check-prefixes=VTABLE-COMMON,FUNC-CMP
; On top of line 4, ignore 'Base1' and its derived types for vtable-based comparison. Tests that ICP compares functions.
; RUN: opt < %s -passes='pgo-icall-prom' -pass-remarks=pgo-icall-prom -enable-vtable-profile-use -icp-max-num-vtable-last-candidate=2 -icp-ignored-base-types='Base1' -S 2>&1 | FileCheck %s --check-prefixes=VTABLE-COMMON,FUNC-CMP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@Base1 = constant { [4 x ptr] } { [4 x ptr] [ptr null, ptr null, ptr @Base1_foo, ptr @Base1_bar] }, !type !0
@Base2 = constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @Base2_foo] }, !type !2
@Base3 = constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @Base3_foo] }, !type !6

@Derived1 = constant { [3 x ptr], [4 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @Base2_foo], [4 x ptr] [ptr null, ptr null, ptr @Base1_foo, ptr @Derived1_bar] }, !type !1, !type !2, !type !3
@Derived2 = constant { [3 x ptr], [3 x ptr], [4 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @Base3_foo], [3 x ptr] [ptr null, ptr null, ptr @Base2_foo], [4 x ptr] [ptr null, ptr null, ptr @Base1_foo, ptr @Derived2_bar] }, !type !4, !type !5, !type !6, !type !7
@Derived3 = constant { [4 x ptr] } { [4 x ptr] [ptr null, ptr null, ptr @Base1_foo, ptr @Base1_bar] }, !type !0, !type !8

; VTABLE-CMP: remark: <unknown>:0:0: Promote indirect call to Derived1_bar with count 600 out of 1600, sink 2 instruction(s) and compare 1 vtable(s): {Derived1}
; VTABLE-CMP: remark: <unknown>:0:0: Promote indirect call to Derived2_bar with count 500 out of 1000, sink 2 instruction(s) and compare 1 vtable(s): {Derived2}
; VTABLE-CMP: remark: <unknown>:0:0: Promote indirect call to Base1_bar with count 400 out of 500, sink 2 instruction(s) and compare 2 vtable(s): {Derived3, Base1}

define void @test(ptr %d) {
; VTABLE-CMP-LABEL: define void @test(
; VTABLE-CMP-SAME: ptr [[D:%.*]]) {
; VTABLE-CMP-NEXT:  [[ENTRY:.*:]]
; VTABLE-CMP-NEXT:    [[VTABLE:%.*]] = load ptr, ptr [[D]], align 8, !prof [[PROF9:![0-9]+]]
; VTABLE-CMP-NEXT:    [[TMP0:%.*]] = tail call i1 @llvm.type.test(ptr [[VTABLE]], metadata !"Base1")
; VTABLE-CMP-NEXT:    tail call void @llvm.assume(i1 [[TMP0]])
; VTABLE-CMP-NEXT:    [[TMP1:%.*]] = icmp eq ptr [[VTABLE]], getelementptr inbounds (i8, ptr @Derived1, i32 40)
; VTABLE-CMP-NEXT:    br i1 [[TMP1]], label %[[IF_TRUE_DIRECT_TARG:.*]], label %[[IF_FALSE_ORIG_INDIRECT:.*]], !prof [[PROF10:![0-9]+]]
; VTABLE-CMP:       [[IF_TRUE_DIRECT_TARG]]:
; VTABLE-CMP-NEXT:    call void @Derived1_bar(ptr [[D]])
; VTABLE-CMP-NEXT:    br label %[[IF_END_ICP:.*]]
; VTABLE-CMP:       [[IF_FALSE_ORIG_INDIRECT]]:
; VTABLE-CMP-NEXT:    [[TMP2:%.*]] = icmp eq ptr [[VTABLE]], getelementptr inbounds (i8, ptr @Derived2, i32 64)
; VTABLE-CMP-NEXT:    br i1 [[TMP2]], label %[[IF_TRUE_DIRECT_TARG1:.*]], label %[[IF_FALSE_ORIG_INDIRECT2:.*]], !prof [[PROF11:![0-9]+]]
; VTABLE-CMP:       [[IF_TRUE_DIRECT_TARG1]]:
; VTABLE-CMP-NEXT:    call void @Derived2_bar(ptr [[D]])
; VTABLE-CMP-NEXT:    br label %[[IF_END_ICP3:.*]]
; VTABLE-CMP:       [[IF_FALSE_ORIG_INDIRECT2]]:
; VTABLE-CMP-NEXT:    [[TMP3:%.*]] = icmp eq ptr [[VTABLE]], getelementptr inbounds (i8, ptr @Base1, i32 16)
; VTABLE-CMP-NEXT:    [[TMP4:%.*]] = icmp eq ptr [[VTABLE]], getelementptr inbounds (i8, ptr @Derived3, i32 16)
; VTABLE-CMP-NEXT:    [[TMP5:%.*]] = or i1 [[TMP3]], [[TMP4]]
; VTABLE-CMP-NEXT:    br i1 [[TMP5]], label %[[IF_TRUE_DIRECT_TARG4:.*]], label %[[IF_FALSE_ORIG_INDIRECT5:.*]], !prof [[PROF12:![0-9]+]]
; VTABLE-CMP:       [[IF_TRUE_DIRECT_TARG4]]:
; VTABLE-CMP-NEXT:    call void @Base1_bar(ptr [[D]])
; VTABLE-CMP-NEXT:    br label %[[IF_END_ICP6:.*]]
; VTABLE-CMP:       [[IF_FALSE_ORIG_INDIRECT5]]:
; VTABLE-CMP-NEXT:    [[VFN:%.*]] = getelementptr inbounds ptr, ptr [[VTABLE]], i64 1
; VTABLE-CMP-NEXT:    [[TMP6:%.*]] = load ptr, ptr [[VFN]], align 8
; VTABLE-CMP-NEXT:    call void [[TMP6]](ptr [[D]])
; VTABLE-CMP-NEXT:    br label %[[IF_END_ICP6]]
; VTABLE-CMP:       [[IF_END_ICP6]]:
; VTABLE-CMP-NEXT:    br label %[[IF_END_ICP3]]
; VTABLE-CMP:       [[IF_END_ICP3]]:
; VTABLE-CMP-NEXT:    br label %[[IF_END_ICP]]
; VTABLE-CMP:       [[IF_END_ICP]]:
; VTABLE-CMP-NEXT:    ret void
;
; FUNC-CMP-LABEL: define void @test(
; FUNC-CMP-SAME: ptr [[D:%.*]]) {
; FUNC-CMP-NEXT:  [[ENTRY:.*:]]
; FUNC-CMP-NEXT:    [[VTABLE:%.*]] = load ptr, ptr [[D]], align 8, !prof [[PROF9:![0-9]+]]
; FUNC-CMP-NEXT:    [[TMP0:%.*]] = tail call i1 @llvm.type.test(ptr [[VTABLE]], metadata !"Base1")
; FUNC-CMP-NEXT:    tail call void @llvm.assume(i1 [[TMP0]])
; FUNC-CMP-NEXT:    [[VFN:%.*]] = getelementptr inbounds ptr, ptr [[VTABLE]], i64 1
; FUNC-CMP-NEXT:    [[TMP1:%.*]] = load ptr, ptr [[VFN]], align 8
; FUNC-CMP-NEXT:    [[TMP2:%.*]] = icmp eq ptr [[TMP1]], @Derived1_bar
; FUNC-CMP-NEXT:    br i1 [[TMP2]], label %[[IF_TRUE_DIRECT_TARG:.*]], label %[[IF_FALSE_ORIG_INDIRECT:.*]], !prof [[PROF10:![0-9]+]]
; FUNC-CMP:       [[IF_TRUE_DIRECT_TARG]]:
; FUNC-CMP-NEXT:    call void @Derived1_bar(ptr [[D]])
; FUNC-CMP-NEXT:    br label %[[IF_END_ICP:.*]]
; FUNC-CMP:       [[IF_FALSE_ORIG_INDIRECT]]:
; FUNC-CMP-NEXT:    [[TMP3:%.*]] = icmp eq ptr [[TMP1]], @Derived2_bar
; FUNC-CMP-NEXT:    br i1 [[TMP3]], label %[[IF_TRUE_DIRECT_TARG1:.*]], label %[[IF_FALSE_ORIG_INDIRECT2:.*]], !prof [[PROF11:![0-9]+]]
; FUNC-CMP:       [[IF_TRUE_DIRECT_TARG1]]:
; FUNC-CMP-NEXT:    call void @Derived2_bar(ptr [[D]])
; FUNC-CMP-NEXT:    br label %[[IF_END_ICP3:.*]]
; FUNC-CMP:       [[IF_FALSE_ORIG_INDIRECT2]]:
; FUNC-CMP-NEXT:    [[TMP4:%.*]] = icmp eq ptr [[TMP1]], @Base1_bar
; FUNC-CMP-NEXT:    br i1 [[TMP4]], label %[[IF_TRUE_DIRECT_TARG4:.*]], label %[[IF_FALSE_ORIG_INDIRECT5:.*]], !prof [[PROF12:![0-9]+]]
; FUNC-CMP:       [[IF_TRUE_DIRECT_TARG4]]:
; FUNC-CMP-NEXT:    call void @Base1_bar(ptr [[D]])
; FUNC-CMP-NEXT:    br label %[[IF_END_ICP6:.*]]
; FUNC-CMP:       [[IF_FALSE_ORIG_INDIRECT5]]:
; FUNC-CMP-NEXT:    call void [[TMP1]](ptr [[D]])
; FUNC-CMP-NEXT:    br label %[[IF_END_ICP6]]
; FUNC-CMP:       [[IF_END_ICP6]]:
; FUNC-CMP-NEXT:    br label %[[IF_END_ICP3]]
; FUNC-CMP:       [[IF_END_ICP3]]:
; FUNC-CMP-NEXT:    br label %[[IF_END_ICP]]
; FUNC-CMP:       [[IF_END_ICP]]:
; FUNC-CMP-NEXT:    ret void
;
entry:
  %vtable = load ptr, ptr %d, !prof !9
  %0 = tail call i1 @llvm.type.test(ptr %vtable, metadata !"Base1")
  tail call void @llvm.assume(i1 %0)
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 1
  %1 = load ptr, ptr %vfn
  call void %1(ptr %d), !prof !10
  ret void
}

define void @Base1_bar(ptr %this) {
  ret void
}

define void @Derived1_bar(ptr %this) {
  ret void
}

define void @Derived2_bar(ptr %this) {
  ret void
}


declare i1 @llvm.type.test(ptr, metadata)
declare void @llvm.assume(i1)
declare i32 @Base2_foo(ptr)
declare i32 @Base1_foo(ptr)
declare void @Base3_foo(ptr)

!llvm.module.flags = !{!11}
!0 = !{i64 16, !"Base1"}
!1 = !{i64 40, !"Base1"}
!2 = !{i64 16, !"Base2"}
!3 = !{i64 16, !"Derived1"}
!4 = !{i64 64, !"Base1"}
!5 = !{i64 40, !"Base2"}
!6 = !{i64 16, !"Base3"}
!7 = !{i64 16, !"Derived2"}
!8 = !{i64 16, !"Derived3"}
!9 = !{!"VP", i32 2, i64 1600, i64 -4123858694673519054, i64 600, i64 -7211198353767973908, i64 500, i64 -3574436251470806727, i64 200, i64 6288809125658696740, i64 200, i64 12345678, i64 100}
!10 = !{!"VP", i32 0, i64 1600, i64 3827408714133779784, i64 600, i64 5837445539218476403, i64 500, i64 -9064955852395570538, i64 400,  i64 56781234, i64 100}

!11 = !{i32 1, !"ProfileSummary", !12}
!12 = !{!13, !14, !15, !16, !17, !18, !19, !20}
!13 = !{!"ProfileFormat", !"InstrProf"}
!14 = !{!"TotalCount", i64 10000}
!15 = !{!"MaxCount", i64 10}
!16 = !{!"MaxInternalCount", i64 1}
!17 = !{!"MaxFunctionCount", i64 1000}
!18 = !{!"NumCounts", i64 3}
!19 = !{!"NumFunctions", i64 3}
!20 = !{!"DetailedSummary", !21}
!21 = !{!22, !23, !24}
!22 = !{i32 10000, i64 101, i32 1}
!23 = !{i32 990000, i64 101, i32 1}
!24 = !{i32 999999, i64 1, i32 2}


;.
; VTABLE-COMMON: [[PROF9]] = !{!"VP", i32 2, i64 100, i64 12345678, i64 100}
; VTABLE-COMMON: [[PROF10]] = !{!"branch_weights", i32 600, i32 1000}
; VTABLE-COMMON: [[PROF11]] = !{!"branch_weights", i32 500, i32 500}
; VTABLE-COMMON: [[PROF12]] = !{!"branch_weights", i32 400, i32 100}

