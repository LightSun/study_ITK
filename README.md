# study_ITK
- 8 threads
- BinomialBlur - rep2
```
//spacing = (2, 4, 6, 8, 10)
apply_filter >> cost 48.93 sec.

apply_filter >> cost 5.61 sec.

apply_filter >> cost 1.84 sec.

apply_filter >> cost 691.19 msec.

apply_filter >> cost 348.67 msec.
ps: resample cost 5s+ (2次和)

```
- BinomialBlur - rep4
```
//spacing = (2, 4, 6, 8, 10)
apply_filter >> cost 1.45 min.

apply_filter >> cost 11.04 sec.

apply_filter >> cost 3.20 sec.

apply_filter >> cost 1.35 sec.

apply_filter >> cost 766.91 msec.
ps: resample cost 5s+ (2次和)

```
- BinomialBlur - rep6
```
//spacing = (2, 4, 6, 8, 10)
apply_filter >> cost 2.27 min.

apply_filter >> cost 18.63 sec.

apply_filter >> cost 5.21 sec.

apply_filter >> cost 2.18 sec.

apply_filter >> cost 1.01 sec.
ps: resample cost 5s+ (2次和)
```
- BinomialBlur - rep100.  
```
效果比较好，迭代次数100. 40min
```

- CurvatureAnisotropicDiffusion - rep100
```
test5: 33min
```


- CurvatureFlow - rep2
```
//spacing = (2, 4, 6, 8, 10)
apply_filter >> cost 8.49 sec.

apply_filter >> cost 1.20 sec.

apply_filter >> cost 375.30 msec.

apply_filter >> cost 181.37 msec.

apply_filter >> cost 103.59 msec.
ps: resample cost 5s+ (2次和)
```

- CurvatureFlow - rep6
```
//spacing = (2, 4, 6, 8, 10)
apply_filter >> cost 25.33 sec.

apply_filter >> cost 3.46 sec.

apply_filter >> cost 1.12 sec.

apply_filter >> cost 544.87 msec.

apply_filter >> cost 266.09 msec.
ps: resample cost 5s+ (2次和)
```
- Mean - radius-2
```
//spacing = (2, 4, 6, 8)
apply_filter >> cost 49.61 sec.

apply_filter >> cost 6.53 sec.

apply_filter >> cost 1.95 sec.

apply_filter >> cost 964.93 msec.

ps: resample cost 5s+ (2次和)
```

- Bilateral - sigma-10.
```
效果还不错。耗时1.53hour
```

- VTK-smooth - rep1000-spacing4-poly_spacing(0.5)
```
load_nifti: thy.nii
save_imageState: 3
resamples >> type = 3
ImageState >> size = 204, 147, 148
spacing = 4, 4, 4
apply_resample >> cost 58.41 msec
smooth_thy >>
apply_filter >> start -> VTK_Smooth
pre_do_vtk_smooth >> dims = 204, 147, 148
img_to_vtkimg >> cost 0.57 msec.
img_to_poly >> cost 98.49 msec.
img_to_poly bounds: (xmin,xmax, ymin,ymax, zmin,zmax) = (98.00,650.00,50.00,334.00,0.00,588.00)
vtk_smooth >> cost 2.62 sec.
poly bounds: (xmin,xmax, ymin,ymax, zmin,zmax) = (99.05,650.00,53.13,326.26,0.00,588.00)
poly_to_img >> cost 11.03 sec.
poly_to_img >> dims = 1102, 547, 1176
vtkimg_to_itkimg >> cost 0.27 msec.
cast_ret >> dims = 1102, 547, 1176
cast_ret >> spacing = 0.500, 0.500, 0.500
apply_filter >> cost 14.96 sec.

restore_imageState: 3
ImageState >> size = 816, 588, 592
spacing = 1, 1, 1
apply_resample >> cost 2.40 sec
handle_fmt >> _<repeat_count> -> _1000
save_thy >> /home/heaven7/heaven7/study/github/mine/build-study_ITK-Desktop_Qt_5_14_2_GCC_64bit-Debug/test/test7/thy_1000_4.nii

```

- VTK-smooth - rep1000-spacing4-poly_spacing(1)
```
load_nifti: thy.nii
save_imageState: 3
resamples >> type = 3
ImageState >> size = 204, 147, 148
spacing = 4, 4, 4
apply_resample >> cost 60.69 msec
smooth_thy >>
apply_filter >> start -> VTK_Smooth
pre_do_vtk_smooth >> dims = 204, 147, 148
img_to_vtkimg >> cost 0.51 msec.
img_to_poly >> cost 104.39 msec.
vtk_smooth >> cost 2.64 sec.
poly_to_img >> cost 3.71 sec.
poly_to_img >> dims = 551, 274, 588
vtkimg_to_itkimg >> cost 0.17 msec.
apply_filter >> cost 6.61 sec.

restore_imageState: 3
ImageState >> size = 816, 588, 592
spacing = 1, 1, 1
apply_resample >> cost 2.40 sec
handle_fmt >> _<repeat_count> -> _1000
save_thy >> /home/heaven7/heaven7/study/github/mine/build-study_ITK-Desktop_Qt_5_14_2_GCC_64bit-Debug/test/test7/thy_1000_4.nii

```







