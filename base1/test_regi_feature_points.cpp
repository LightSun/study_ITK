#include "itkBlockMatchingImageFilter.h"
#include "itkImage.h"
#include "itkPoint.h"
#include "itkPointSet.h"

using ImageType = itk::Image<unsigned char, 2>;

static void
CreateImage(ImageType::Pointer image, const unsigned int x);

//TODO fix crash?
int
test_registration_match_feature_points()
{
  // Create input images
  auto fixedImage = ImageType::New();
  CreateImage(fixedImage, 40);

  auto movingImage = ImageType::New();
  CreateImage(movingImage, 50);

  //  using BlockMatchingImageFilterType = itk::BlockMatchingImageFilter<ImageType, ImageType, PointSetType>;
  using BlockMatchingImageFilterType = itk::BlockMatchingImageFilter<ImageType>;
  auto blockMatchingImageFilter = BlockMatchingImageFilterType::New();

  // Generate feature points
  //  using PointSetType = itk::PointSet< float, 2>;
  using PointSetType = BlockMatchingImageFilterType::FeaturePointsType;
  using PointType = PointSetType::PointType;
  using PointsContainerPointer = PointSetType::PointsContainerPointer;

  auto                   pointSet = PointSetType::New();
  PointsContainerPointer points = pointSet->GetPoints();

  PointType p0, p1, p2, p3;

  p0[0] = 40.0;
  p0[1] = 40.0;
  p1[0] = 40.0;
  p1[1] = 60.0;
  p2[0] = 60.0;
  p2[1] = 40.0;
  p3[0] = 60.0;
  p2[1] = 60.0;

  points->InsertElement(0, p0);
  points->InsertElement(1, p1);
  points->InsertElement(2, p2);
  points->InsertElement(3, p3);

  blockMatchingImageFilter->SetFixedImage(fixedImage);
  blockMatchingImageFilter->SetMovingImage(movingImage);
  blockMatchingImageFilter->SetFeaturePoints(pointSet);
  //CRASH after this
  blockMatchingImageFilter->UpdateLargestPossibleRegion();

  typename BlockMatchingImageFilterType::DisplacementsType * displacements =
    blockMatchingImageFilter->GetDisplacements();

  std::cout << "There are " << displacements->GetNumberOfPoints() << " displacements." << std::endl;

  return EXIT_SUCCESS;
}

void
CreateImage(ImageType::Pointer image, const unsigned int x)
{
  // Allocate empty image
  itk::Index<2> start;
  start.Fill(0);
  itk::Size<2> size;
  size.Fill(100);
  ImageType::RegionType region(start, size);
  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(0);

  // Make a white square
  for (unsigned int r = x; r < x + 20; ++r)
  {
    for (unsigned int c = 40; c < 60; ++c)
    {
      ImageType::IndexType pixelIndex;
      pixelIndex[0] = r;
      pixelIndex[1] = c;
      image->SetPixel(pixelIndex, 255);
    }
  }
}
