#include <iostream>
#include <cmath>

#include "Texture.h"
#include <stb_image.h>

Texture::Texture(uint32_t width, uint32_t height) :
Texture(width, height, FilterMode::BILINEAR)
{
}

Texture::Texture(uint32_t width, uint32_t height, FilterMode filterMode) :
Texture(width, height, filterMode, BorderMode::REPEAT)
{
}

Texture::Texture(uint32_t width, uint32_t height, FilterMode filterMode,
                 BorderMode borderMode) :
width(width),
height(height),
filterMode(filterMode),
borderModeU(borderMode),
borderModeV(borderMode)
{
  borderColor = Vec3{ 0.0f, 0.0f, 0.0f };
  data = std::make_unique<Image>(width, height, 3);
}

Texture::Texture(const std::string& filename) :
Texture(filename, FilterMode::BILINEAR)
{
}

Texture::Texture(const std::string& filename, FilterMode filterMode) :
Texture(filename, filterMode, BorderMode::REPEAT)
{
}

Texture::Texture(const std::string& filename, FilterMode filterMode,
                 BorderMode borderMode) :
filterMode(filterMode),
borderModeU(borderMode),
borderModeV(borderMode),
borderColor(Vec3{0,0,0})
{
  stbi_set_flip_vertically_on_load(false);

  int width, height, nrComponents;
  stbi_uc* image_data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if(image_data) {
    this->width = uint32_t(width);
    this->height = uint32_t(height);
    this->borderModeU = borderMode;
    this->borderModeV = borderMode;
    this->filterMode = filterMode;

    data = std::make_unique<Image>(width, height, nrComponents, std::vector<uint8_t>{image_data, image_data + (width * height * nrComponents) });

    stbi_image_free(image_data);
  } else {
    std::cerr << "Texture failed to load at path: " << filename << std::endl;
    stbi_image_free(image_data);
  }
}

Texture Texture::genCheckerboardTexture(uint32_t width, uint32_t height) {
  Texture checkerboard(width, height, FilterMode::NEAREST, BorderMode::REPEAT);

  // TODO Task02:
  // Implement the generation of a checkerboard texture.
  // data[0] should be of black color
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; ++x) {
      
      bool isBlack = (x+y) % 2 == 0;
      float colorValue = isBlack ? 0.0f : 1.0f;
      checkerboard.data->setNormalizedValue(x, y, 0, colorValue);
      checkerboard.data->setNormalizedValue(x, y, 1, colorValue);
      checkerboard.data->setNormalizedValue(x, y, 2, colorValue);
    }
  }

  return checkerboard;
}

static uint32_t handleBorderCoordinate(int pixelCoord, uint32_t max,
                                       BorderMode bordermode) {
  // TODO Task03: Implement the missing bordermodes CLAMP_TO_BORDER, CLAMP_TO_EDGE, MIRRORED_REPEAT.
  // The texel index range is [0; max - 1]

  // TODO: How to check?

  switch (bordermode) {
    case BorderMode::CLAMP_TO_EDGE:{
      if (pixelCoord < 0) {
        pixelCoord = 0;
      } else if (pixelCoord >= int(max)) {
        pixelCoord = int(max) - 1;
      }
      // else pixelCoord is already in range, do nothing
      break;
    }case BorderMode::CLAMP_TO_BORDER:{
      if (pixelCoord < 0 || pixelCoord >= int(max)) {
        return uint32_t(-1); // Indicate border color
      }
      break;
    }case BorderMode::MIRRORED_REPEAT:{
      // Apply abs because the pattern is mirrored at every multiple of max, so negative coordinates are mirrored to positive ones
      pixelCoord = std::abs(pixelCoord);

      int numberOfRepeats = pixelCoord / int(max);
      bool isEvenRepeat = (numberOfRepeats % 2) == 0; // In even repeats we are in the original orientation, in odd repeats we are mirrored
      if (isEvenRepeat) {
        pixelCoord = pixelCoord % int(max);
      } else {
        // In odd repeats we are mirrored,
        // so we need to mirror the coordinate and then apply modulo
        // To mirror the coordinate we can take the distance to the next multiple of max and subtract it from max
        pixelCoord = int(max) - 1 - (pixelCoord % int(max));
      }
      break;
    }case BorderMode::REPEAT:{
      pixelCoord = pixelCoord % int(max);
      if (pixelCoord < 0)
        pixelCoord += int(max);
      break;
    }default:{
      std::cerr << "Specified bordermode cannot be handled... Applying REPEAT as default." << std::endl;
      return handleBorderCoordinate(pixelCoord, max, BorderMode::REPEAT);
      break;
    }
  }

  return uint32_t(pixelCoord);
}

Vec3 Texture::sample(int pixelCoordX, int pixelCoordY) const {
  // TODO Task03: Implement the missing bordermodes CLAMP_TO_BORDER, CLAMP_TO_EDGE, MIRRORED_REPEAT
  // TODO: Qué hay que hacer? Solo eso?

  const uint32_t sampleCoordX = handleBorderCoordinate(pixelCoordX, width, borderModeU);
  const uint32_t sampleCoordY = handleBorderCoordinate(pixelCoordY, height, borderModeV);
  if (sampleCoordX == uint32_t(-1) || sampleCoordY == uint32_t(-1)) {
    return borderColor;
  }else{
    return {data->getValue(sampleCoordX, sampleCoordY, 0) / 255.0f,
            data->getValue(sampleCoordX, sampleCoordY, 1) / 255.0f,
            data->getValue(sampleCoordX, sampleCoordY, 2) / 255.0f};
  }
}


Vec3 Texture::sample(const TextureCoordinates& texCoords) const {
  const float dx = texCoords.u * width;
  const float dy = texCoords.v * height;

  switch (filterMode) {
    case FilterMode::NEAREST: {
      const int nx = static_cast<int>(std::floor(dx + 0.5f));
      const int ny = static_cast<int>(std::floor(dy + 0.5f));
      return sample(nx, ny);
    }
    case FilterMode::BILINEAR: {
      // TODO Task03: Implement sampling using Bilinear Filtering
      // TODO: Cómo corregir?
      const int x1 = static_cast<int>(std::floor(dx));
      const int y1 = static_cast<int>(std::floor(dy));
      const int x2 = x1 + 1;
      const int y2 = y1 + 1;

      // Points considered: (x1, y1), (x2, y1), (x1, y2), (x2, y2)
      Vec3 c11 = sample(x1, y1);
      Vec3 c21 = sample(x2, y1);
      Vec3 c12 = sample(x1, y2);
      Vec3 c22 = sample(x2, y2);

      // Weights for interpolation
      float wx = (dx - x1);
      float wy = (dy - y1);

      return (1-wx) * (1-wy) * c11 + 
             wx * (1-wy) * c21 + 
             (1-wx) * wy * c12 + 
             wx * wy * c22;
    }
    default:
      std::cout << "Specified bordermode cannot be handled..." << std::endl;
      return {};
  }
}

void Texture::setBorderMode(BorderMode borderMode) {
  borderModeU = borderMode;
  borderModeV = borderMode;
}

void Texture::setBorderModeU(BorderMode borderMode) {
  borderModeU = borderMode;
}

void Texture::setBorderModeV(BorderMode borderMode) {
  borderModeV = borderMode;
}

void Texture::setBorderColor(const Vec3& borderColor) {
  this->borderColor = borderColor;
}
