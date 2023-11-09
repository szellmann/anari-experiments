// ======================================================================== //
// Copyright 2020-2020 The Authors                                          //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include <cstring>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION 1
#define TINYEXR_IMPLEMENTATION 1
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "tinyexr.h"

#include "hdri.h"

void HDRI::load(std::string fileName)
{
  // check the extension
  std::string extension = std::string(strrchr(fileName.c_str(), '.'));
  std::transform(extension.data(), extension.data() + extension.size(), 
      std::addressof(extension[0]), [](unsigned char c){ return std::tolower(c); });

  if (extension.compare(".hdr") != 0 && extension.compare(".exr") != 0) {
    throw std::runtime_error("Error: expected either a .hdr or a .exr file");
  }

  if (extension.compare(".hdr") == 0) {
    int w, h, n;
    float *imgData = stbi_loadf(fileName.c_str(), &w, &h, &n, STBI_rgb);
    width = w;
    height = h;
    numComponents = n;
    pixel.resize(w*h*n);
    memcpy(pixel.data(),imgData,w*h*n*sizeof(float));
    stbi_image_free(imgData);
  } else {
    int w, h, n;
    float* imgData;
    const char* err;
    int ret = LoadEXR(&imgData, &w, &h, fileName.c_str(), &err);
    if (ret != 0)
      throw std::runtime_error(std::string("Error, ") + std::string(err));
    n = 4;

    width = w;
    height = h;
    numComponents = n;
    pixel.resize(w*h*n);
    memcpy(pixel.data(),imgData,w*h*n*sizeof(float));
  }

}
