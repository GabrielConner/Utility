#include "pPack/modelLoading.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <format>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


using namespace ::pPack;

inline constexpr size_t _PROP_TYPE_SIZE[4] = {3, 2, 3, 4};
enum struct PropertyType { POSITION=0, UV, NORMAL, COLOR };

int main(int argc, char** argv) {
  if (argc < 3 || strcmp(argv[1],"-h") == 0) {
    std::cout << "GMV {ModelLocation} {Output} [s] [f]\nModelLocation -- The file location of the model\nOutput -- The data from the vertices to copy\n\tp -- Position\n\tu -- UV\n\tn -- Normal\n\n\tc -- Color\n\ns -- Add backslashes along each line (for macro creation)\nf -- Add an 'f' to the end of each number";
    return 0;
  }


  auto optMesh = LoadMesh(argv[1]);
  if (!optMesh) {
    std::cout << "Failed to load mesh\n";
    return 1;
  }

  if (optMesh->vertexCount == 0) {
    std::cout << "Invalid mesh\n";
    return 1;
  }



  std::vector<PropertyType> props = std::vector<PropertyType>();
  for (char c : std::string(argv[2])) {
    if (c == 'p') props.push_back(PropertyType::POSITION);
    else if (c == 'u') props.push_back(PropertyType::UV);
    else if (c == 'n') props.push_back(PropertyType::NORMAL);
    else if (c == 'c') props.push_back(PropertyType::COLOR);
  }

  if (props.size() == 0) {
    std::cout << "At least one valid variable is required\n";
    return 1;
  }

  std::string add = "";
  std::string formatString = "{:}";
  for (int i = 3; i < argc; i++) {
    char c = *argv[i];
    if (c == 's') {
      add = "\\";
    } else if (c == 'f') {
      formatString = "{:f}";
    }
  }
  

  std::stringstream str;
  for (size_t i = 0; i < optMesh->vertexCount; i++) {
    LoadedMeshVertex const& vert = optMesh->vertices[i];

    for (PropertyType const& prop : props) {
      switch (prop) {
        case PropertyType::POSITION:
          str << std::vformat(formatString, std::make_format_args(vert.position)) << ",";
          break;
        case PropertyType::UV:
          str << std::vformat(formatString, std::make_format_args(vert.uv)) << ",";
          break;
        case PropertyType::NORMAL:
          str << std::vformat(formatString, std::make_format_args(vert.normal)) << ",";
          break;
        case PropertyType::COLOR:
          str << std::vformat(formatString, std::make_format_args(vert.color)) << ",";
          break;
      }
    }
    str << add << '\n';
  }

  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, size_t(str.tellp()) + 1);
  void* mem = GlobalLock(handle);
  if (!mem) {
    GlobalFree(handle);
    optMesh->Delete();
    return 1;
  }

  memcpy((char*)mem, str.str().c_str(), str.tellp());

  GlobalUnlock(handle);
  if (OpenClipboard(NULL) == 0) {
    return 1;
  }
  EmptyClipboard();
  SetClipboardData(CF_TEXT, handle);
  CloseClipboard();

  optMesh->Delete();
  return 0;
}