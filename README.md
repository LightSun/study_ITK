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







