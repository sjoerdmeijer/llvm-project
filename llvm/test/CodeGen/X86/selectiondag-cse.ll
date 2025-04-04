; RUN: llc < %s
; PR12599
;
; This bitcode causes the X86 target to make changes to the DAG during
; selection in MatchAddressRecursively. The edit triggers CSE which causes both
; the current node and yet-to-be-selected nodes to be deleted.
;
; SelectionDAGISel::DoInstructionSelection must handle that.
;
target triple = "x86_64-apple-macosx"

%0 = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, float, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, ptr, i32, ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [9 x [16 x [16 x i16]]], [5 x [16 x [16 x i16]]], [9 x [8 x [8 x i16]]], [2 x [4 x [16 x [16 x i16]]]], [16 x [16 x i16]], [16 x [16 x i32]], ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i32, i32, [4 x [4 x i32]], i32, i32, i32, i32, i32, double, i32, i32, i32, i32, ptr, ptr, ptr, ptr, [15 x i16], i32, i32, i32, i32, i32, i32, i32, i32, [6 x [32 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [1 x i32], i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [2 x i32], i32, i32, i16, i32, i32, i32, i32, i32 }
%1 = type { i32, i32, [100 x ptr], i32, float, float, float }
%2 = type { i32, i32, i32, i32, i32, i32, ptr, ptr, ptr, i32, ptr, ptr, ptr, i32, ptr, ptr, ptr, ptr, [3 x [2 x i32]] }
%3 = type { ptr, %5, %5 }
%4 = type { i32, i32, i8, i32, i32, i8, i8, i32, i32, ptr, i32 }
%5 = type { i32, i32, i32, i32, i32, ptr, ptr, i32, i32 }
%6 = type { [3 x [11 x %7]], [2 x [9 x %7]], [2 x [10 x %7]], [2 x [6 x %7]], [4 x %7], [4 x %7], [3 x %7] }
%7 = type { i16, i8, i64 }
%8 = type { [2 x %7], [4 x %7], [3 x [4 x %7]], [10 x [4 x %7]], [10 x [15 x %7]], [10 x [15 x %7]], [10 x [5 x %7]], [10 x [5 x %7]], [10 x [15 x %7]], [10 x [15 x %7]] }
%9 = type { i32, i32, i32, [2 x i32], i32, [8 x i32], ptr, ptr, i32, [2 x [4 x [4 x [2 x i32]]]], [16 x i8], [16 x i8], i32, i64, [4 x i32], [4 x i32], i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, double, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%10 = type { i32, i32, i32, i32, i32, ptr }

@images = external hidden global %0, align 8

define hidden fastcc void @Mode_Decision_for_4x4IntraBlocks(i1 %arg) nounwind uwtable ssp {
bb4:
  %tmp = or i208 undef, 0
  br i1 %arg, label %bb35, label %bb5

bb5:
  %tmp6 = add i32 0, 2
  %tmp7 = lshr i208 %tmp, 80
  %tmp8 = trunc i208 %tmp7 to i32
  %tmp9 = and i32 %tmp8, 65535
  %tmp10 = shl nuw nsw i32 %tmp9, 1
  %tmp11 = add i32 0, 2
  %tmp12 = add i32 %tmp11, 0
  %tmp13 = add i32 %tmp12, %tmp10
  %tmp14 = lshr i32 %tmp13, 2
  %tmp15 = trunc i32 %tmp14 to i16
  store i16 %tmp15, ptr getelementptr inbounds (%0, ptr @images, i64 0, i32 47, i64 3, i64 0, i64 3), align 2
  %tmp16 = lshr i208 %tmp, 96
  %tmp17 = trunc i208 %tmp16 to i32
  %tmp18 = and i32 %tmp17, 65535
  %tmp19 = add i32 %tmp18, 2
  %tmp20 = add i32 %tmp19, 0
  %tmp21 = add i32 %tmp20, 0
  %tmp22 = lshr i32 %tmp21, 2
  %tmp23 = trunc i32 %tmp22 to i16
  store i16 %tmp23, ptr getelementptr inbounds (%0, ptr @images, i64 0, i32 47, i64 3, i64 2, i64 3), align 2
  %tmp24 = add i32 %tmp6, %tmp9
  %tmp25 = add i32 %tmp24, 0
  %tmp26 = lshr i32 %tmp25, 2
  %tmp27 = trunc i32 %tmp26 to i16
  store i16 %tmp27, ptr getelementptr inbounds (%0, ptr @images, i64 0, i32 47, i64 7, i64 1, i64 2), align 4
  %tmp28 = lshr i208 %tmp, 80
  %tmp29 = shl nuw nsw i208 %tmp28, 1
  %tmp30 = trunc i208 %tmp29 to i32
  %tmp31 = and i32 %tmp30, 131070
  %tmp32 = add i32 %tmp12, %tmp31
  %tmp33 = lshr i32 %tmp32, 2
  %tmp34 = trunc i32 %tmp33 to i16
  store i16 %tmp34, ptr getelementptr inbounds (%0, ptr @images, i64 0, i32 47, i64 7, i64 1, i64 3), align 2
  br label %bb35

bb35:                                             ; preds = %bb5, %bb4
  unreachable
}
