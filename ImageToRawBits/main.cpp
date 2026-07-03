#include "stb_image.h"
#include <iostream>
#include <filesystem>
#include <array>
#include <fstream>


struct UI16Vector4 {
  uint16_t
    r = 0,
    g = 0,
    b = 0,
    a = 0;

  UI16Vector4() = default;
};

struct Swizzle {
  int size = 0;
  size_t offset = 0;

  Swizzle() = default;
};



int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cout << "IRB {Input} {Output} {Color}[r|g|b|a]\n\nInput  -- \tThe image file to open\n\nOutput -- \tThe file to output the binary data\n\nColor  -- \tThe pseudo swizzle mask and bit field\n\t\tColors in a row represent that color with that many bits; eg. [rrrrrggggggbbbbb] is 65K color bits\n\t\tCan be repeated with various bit fields\n\t\t[rrggrrrg] is valid; [rrrrr] will always be 5 bit red field, not 3-2 or other combinations\n\t\tMax 16 bits per grouping";
    return 0;
  }

  if (argc != 4) {
    std::cout << "Invalid number of arguements {" << argc - 1 << "}\nExpected : " << 3 << "\n";
    return 1;
  }

  std::string input = argv[1];
  std::string output = argv[2];
  std::string swizzles = std::string(argv[3]) + ']'; // simple end

  if (!std::filesystem::exists(input)) {
    std::cout << "Input file does not exist\n";
    return 1;
  }

  std::vector<Swizzle> swizzleList = std::vector<Swizzle>();
  {
    Swizzle inSwizzle = Swizzle();
    inSwizzle.size = 1;
    char type = 0;
    for (char const& c : swizzles) {
      if (type != 0) {
        if (c != type) {
          swizzleList.push_back(inSwizzle);
          inSwizzle.size = 1;
          type = 0;
        } else {
          inSwizzle.size++;
          continue;
        }
      }


      switch (c) {
        case 'r':
          inSwizzle.offset = 0;
          type = 'r';
          break;
        case 'g':
          inSwizzle.offset = 1;
          type = 'g';
          break;
        case 'b':
          inSwizzle.offset = 2;
          type = 'b';
          break;
        case 'a':
          inSwizzle.offset = 3;
          type = 'a';
          break;
      }
    }
  }
  if (swizzleList.size() == 0) {
    std::cout << "Invalid color swizzles\n";
    return 1;
  }

  
  int bitsPerPixel = 0;
  for (Swizzle& swizzle : swizzleList) {
    if (swizzle.size > 16) swizzle.size = 16;
    bitsPerPixel += swizzle.size;
  }



  int x, y;
  UI16Vector4* imageData = (UI16Vector4*)stbi_load_16(input.c_str(), &x, &y, 0, STBI_rgb_alpha);
  if (imageData == nullptr) {
    std::cout << "Failed to load input file\n";
    return 1;
  }
  size_t imageSize = size_t(x) * y;


  size_t totalByteCount = imageSize * bitsPerPixel + 7 >> 3;

  uint8_t* bytes = (uint8_t*)calloc(totalByteCount, 1);
  size_t byte = 0;
  signed char bit = 7; 

  for (int j = 0; j < y; j++) {
    for (int i = 0; i < x; i++) {
      uint16_t* color = (uint16_t*)&imageData[j * x + i];
      for (Swizzle& swizzle : swizzleList) {
        uint16_t val = (*(color + swizzle.offset) >> (16 - swizzle.size));

        for (int b = swizzle.size - 1; b >= 0; b--) {
          bytes[byte] |= (((val >> b) & 0x1) << bit);
          if (--bit < 0) {
            byte++;
            bit = 7;
          }
        }
      }
    }
  }

  stbi_image_free(imageData);


  std::ofstream stream(output, std::ios::binary | std::ios::trunc);
  stream.write((char*)bytes, totalByteCount);
  stream.flush();
  stream.close();
  free(bytes);

  return 0;
}