#!/bin/bash



# clear & create folders
rm -rf Video_validation
mkdir Video_validation
mkdir Video_validation/p1_chair_2_{f,l,r,s}
mkdir Video_validation/p1_grab_3_{f,l,r,s}
mkdir Video_validation/p1_orthosyn_1_{f,l,r,s}
mkdir Video_validation/p1_table_2_{f,l,r,s}
mkdir Video_validation/p1_triangle_1_{f,l,r,s}

rm -rf Video_training
mkdir Video_training
mkdir Video_training/p1_chair_2_{f,l,r,s}
mkdir Video_training/p1_grab_3_{f,l,r,s}
mkdir Video_training/p1_orthosyn_1_{f,l,r,s}
mkdir Video_training/p1_table_2_{f,l,r,s}
mkdir Video_training/p1_triangle_1_{f,l,r,s}

rm -rf Video_test
mkdir Video_test
mkdir Video_test/p1_chair_2_{f,l,r,s}
mkdir Video_test/p1_grab_3_{f,l,r,s}
mkdir Video_test/p1_orthosyn_1_{f,l,r,s}
mkdir Video_test/p1_table_2_{f,l,r,s}
mkdir Video_test/p1_triangle_1_{f,l,r,s}

# move training samples
printf "\nMoving training samples..."
mv Video/p1_chair_2_f/im{0001..0715}.png Video_training/p1_chair_2_f/
printf "."
mv Video/p1_chair_2_l/im{0001..0715}.png Video_training/p1_chair_2_l/
printf "."
mv Video/p1_chair_2_r/im{0001..0715}.png Video_training/p1_chair_2_r/
printf "."
mv Video/p1_chair_2_s/im{0001..0715}.png Video_training/p1_chair_2_s/
printf "."

mv Video/p1_grab_3_f/im{0001..0840}.png Video_training/p1_grab_3_f/
printf "."
mv Video/p1_grab_3_l/im{0001..0840}.png Video_training/p1_grab_3_l/
printf "."
mv Video/p1_grab_3_r/im{0001..0840}.png Video_training/p1_grab_3_r/
printf "."
mv Video/p1_grab_3_s/im{0001..0840}.png Video_training/p1_grab_3_s/
printf "."

mv Video/p1_orthosyn_1_f/im{0001..0700}.png Video_training/p1_orthosyn_1_f/
printf "."
mv Video/p1_orthosyn_1_l/im{0001..0700}.png Video_training/p1_orthosyn_1_l/
printf "."
mv Video/p1_orthosyn_1_r/im{0001..0700}.png Video_training/p1_orthosyn_1_r/
printf "."
mv Video/p1_orthosyn_1_s/im{0001..0700}.png Video_training/p1_orthosyn_1_s/
printf "."

mv Video/p1_table_2_f/im{1511..2500}.png Video_training/p1_table_2_f/
printf "."
mv Video/p1_table_2_l/im{1511..2500}.png Video_training/p1_table_2_l/
printf "."
mv Video/p1_table_2_r/im{1511..2500}.png Video_training/p1_table_2_r/
printf "."
mv Video/p1_table_2_s/im{1511..2500}.png Video_training/p1_table_2_s/
printf "."

mv Video/p1_triangle_1_f/im{0001..0700}.png Video_training/p1_triangle_1_f/
printf "."
mv Video/p1_triangle_1_l/im{0001..0700}.png Video_training/p1_triangle_1_l/
printf "."
mv Video/p1_triangle_1_r/im{0001..0700}.png Video_training/p1_triangle_1_r/
printf "."
mv Video/p1_triangle_1_s/im{0001..0700}.png Video_training/p1_triangle_1_s/
printf "."

# move validation samples
printf "\nMoving validation samples..."
mv Video/p1_chair_2_f/im{0716..1400}.png Video_validation/p1_chair_2_f/
printf "."
mv Video/p1_chair_2_l/im{0716..1400}.png Video_validation/p1_chair_2_l/
printf "."
mv Video/p1_chair_2_r/im{0716..1400}.png Video_validation/p1_chair_2_r/
printf "."
mv Video/p1_chair_2_s/im{0716..1400}.png Video_validation/p1_chair_2_s/
printf "."

mv Video/p1_grab_3_f/im{0841..1375}.png Video_validation/p1_grab_3_f/
printf "."
mv Video/p1_grab_3_l/im{0841..1375}.png Video_validation/p1_grab_3_l/
printf "."
mv Video/p1_grab_3_r/im{0841..1375}.png Video_validation/p1_grab_3_r/
printf "."
mv Video/p1_grab_3_s/im{0841..1375}.png Video_validation/p1_grab_3_s/
printf "."

mv Video/p1_orthosyn_1_f/im{0701..1400}.png Video_validation/p1_orthosyn_1_f/
printf "."
mv Video/p1_orthosyn_1_l/im{0701..1400}.png Video_validation/p1_orthosyn_1_l/
printf "."
mv Video/p1_orthosyn_1_r/im{0701..1400}.png Video_validation/p1_orthosyn_1_r/
printf "."
mv Video/p1_orthosyn_1_s/im{0701..1400}.png Video_validation/p1_orthosyn_1_s/
printf "."

mv Video/p1_table_2_f/im{0001..0575}.png Video_validation/p1_table_2_f/
printf "."
mv Video/p1_table_2_l/im{0001..0575}.png Video_validation/p1_table_2_l/
printf "."
mv Video/p1_table_2_r/im{0001..0575}.png Video_validation/p1_table_2_r/
printf "."
mv Video/p1_table_2_s/im{0001..0575}.png Video_validation/p1_table_2_s/
printf "."

mv Video/p1_triangle_1_f/im{0701..1400}.png Video_validation/p1_triangle_1_f/
printf "."
mv Video/p1_triangle_1_l/im{0701..1400}.png Video_validation/p1_triangle_1_l/
printf "."
mv Video/p1_triangle_1_r/im{0701..1400}.png Video_validation/p1_triangle_1_r/
printf "."
mv Video/p1_triangle_1_s/im{0701..1400}.png Video_validation/p1_triangle_1_s/
printf "."

# move test samples
printf "\nMoving test samples..."
mv Video/p1_chair_2_f/im{1401..2570}.png Video_test/p1_chair_2_f/
printf "."
mv Video/p1_chair_2_l/im{1401..2570}.png Video_test/p1_chair_2_l/
printf "."
mv Video/p1_chair_2_r/im{1401..2570}.png Video_test/p1_chair_2_r/
printf "."
mv Video/p1_chair_2_s/im{1401..2570}.png Video_test/p1_chair_2_s/
printf "."

mv Video/p1_grab_3_f/im{1376..2827}.png Video_test/p1_grab_3_f/
printf "."
mv Video/p1_grab_3_l/im{1376..2827}.png Video_test/p1_grab_3_l/
printf "."
mv Video/p1_grab_3_r/im{1376..2827}.png Video_test/p1_grab_3_r/
printf "."
mv Video/p1_grab_3_s/im{1376..2827}.png Video_test/p1_grab_3_s/
printf "."

mv Video/p1_orthosyn_1_f/im{1401..2480}.png Video_test/p1_orthosyn_1_f/
printf "."
mv Video/p1_orthosyn_1_l/im{1401..2480}.png Video_test/p1_orthosyn_1_l/
printf "."
mv Video/p1_orthosyn_1_r/im{1401..2480}.png Video_test/p1_orthosyn_1_r/
printf "."
mv Video/p1_orthosyn_1_s/im{1401..2480}.png Video_test/p1_orthosyn_1_s/
printf "."

mv Video/p1_table_2_f/im{0576..1510}.png Video_test/p1_table_2_f/
printf "."
mv Video/p1_table_2_l/im{0576..1510}.png Video_test/p1_table_2_l/
printf "."
mv Video/p1_table_2_r/im{0576..1510}.png Video_test/p1_table_2_r/
printf "."
mv Video/p1_table_2_s/im{0576..1510}.png Video_test/p1_table_2_s/
printf "."
mv Video/p1_table_2_f/im{2501..2866}.png Video_test/p1_table_2_f/
printf "."
mv Video/p1_table_2_l/im{2501..2866}.png Video_test/p1_table_2_l/
printf "."
mv Video/p1_table_2_r/im{2501..2866}.png Video_test/p1_table_2_r/
printf "."
mv Video/p1_table_2_s/im{2501..2866}.png Video_test/p1_table_2_s/
printf "."

mv Video/p1_triangle_1_f/im{1401..2471}.png Video_test/p1_triangle_1_f/
printf "."
mv Video/p1_triangle_1_l/im{1401..2471}.png Video_test/p1_triangle_1_l/
printf "."
mv Video/p1_triangle_1_r/im{1401..2471}.png Video_test/p1_triangle_1_r/
printf "."
mv Video/p1_triangle_1_s/im{1401..2471}.png Video_test/p1_triangle_1_s/
printf ".\nDone.\n"
