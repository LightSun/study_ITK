

export OUT_FILE=/home/heaven7/heaven7/libs/ITK/sample/test_out/smooth
export IN_FILE=/media/heaven7/h7/3d_recreate/1-1_mask.nii
./test1 --mode test_smoothing_binomial_blur ${IN_FILE} ${OUT_FILE} 6 .nii
./test1 --mode test_smoothing_binomial_blur ${IN_FILE} ${OUT_FILE} 10 .nii
./test1 --mode test_smoothing_binomial_blur ${IN_FILE} ${OUT_FILE} 20 .nii
./test1 --mode test_smoothing_binomial_blur ${IN_FILE} ${OUT_FILE} 30 .nii
./test1 --mode test_smoothing_binomial_blur ${IN_FILE} ${OUT_FILE} 40 .nii
./test1 --mode test_smoothing_binomial_blur ${IN_FILE} ${OUT_FILE} 60 .nii
#./test1 --mode test_smoothing_binomial_blur ${IN_FILE} ${OUT_FILE} 80 .nii
#./test1 --mode test_smoothing_binomial_blur ${IN_FILE} ${OUT_FILE} 100 .nii
#./test1 --mode test_itk_to_vtk ${OUT_FILE}
