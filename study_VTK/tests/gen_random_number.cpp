
#include <vtkBoxMuellerRandomSequence.h>
#include <vtkNew.h>

//GaussianRandomNumber.cxx
int test_GaussianRandomNumber(int argc, char* argv[]){

    // The number of random numbers we wish to produce
     unsigned int numRand = 3;

     vtkNew<vtkBoxMuellerRandomSequence> randomSequence;

     //从均值为 0.0 且标准差为 2.0 的高斯分布生成 numRand 随机数
     // Generate numRand random numbers from a Gaussian distribution with mean 0.0
     // and standard deviation 2.0
     auto mean = 0.0;
     auto standardDeviation = 2.0;
     for (unsigned int i = 0; i < numRand; i++)
     {
       auto a = randomSequence->GetScaledValue(mean, standardDeviation);
       randomSequence->Next();
       std::cout << a << std::endl;
     }

    return 0;
}
