#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <array>
#include <format>

using namespace std::literals;

struct CharPredicate {
  char c = 0;
  CharPredicate() = default;
  CharPredicate(char C) : c(C) {}

  bool operator()(char C) const { return c == C; }
};


struct NotCharPredicate {
  char c = 0;
  NotCharPredicate() = default;
  NotCharPredicate(char C) : c(C) {}

  bool operator()(char C) const { return c != C; }
};


std::string& ConvertStringName(std::string& string, std::vector<std::pair<std::string, std::string>>& names) {

  std::string nString;
  std::copy_if(string.begin(), string.end(), std::back_inserter(nString), NotCharPredicate(' '));

  string = nString;
  std::for_each(nString.begin(), nString.end(), [](char& c) {c = std::toupper(c); });
  nString = "FP" + std::move(nString);
  names.push_back({nString, string});

  string = " (APIENTRYP " + std::move(nString);
  string.push_back(')');

  return string;
}


int main(int argv, char** argc) {
  if (argv == 2 && argc[1][0] == 'h') {
    std::cout << "FTFP {input} {output}\ninput -- File with functions\noutput -- File to place function pointers\n";
    return 0;
  }

#if !(_DEBUG)
  if (argv != 4) {
    std::cout << "Invalid argument count";
    return 0;
  }


  std::string input = argc[1];
  std::string headerOutput = argc[2];
  std::string sourceOutput = argc[3];

  if (!std::filesystem::exists(input)) {
    std::cout << "Input file does not exist";
    return 0;
  }
#endif

  std::array<char, 6> ignoreChars = {'/', '}', '{', 'e', '#'};

#if _DEBUG
  std::stringstream is = std::stringstream();
  is << "#ifndef PUMPKIN_ROLL_SRC_PRIVATE_FUNCTIONS_H\n#define PUMPKIN_ROLL_SRC_PRIVATE_FUNCTIONS_H\n\n#include \"pumpkin/types.h\"\n\nnamespace pumpkin {\n\n\nStartReturn Init(StartSettings const& start, int argv, char** argc, void (*devLoad)());\nvoid Update();\nvoid End();\n\nRuntimeSettings* GetRuntime();\n\nstd::string ExecutableLocation();\n\nvoid PrintError(PrintLevel level, char const* file, char const* msg);\n\ndouble DeltaTime();\n\n\n// Object\n// --------------------------------------------------\n// --------------------------------------------------\n\nObject* RegisterObject(std::string const& name);\nObject* GetObject(std::string const& name);\nbool DeleteObject(std::string const& name);\n\nchar const* Object_GetName(Object const* object);\nbool Object_SetModel(Object* object, Model* model);\nbool Object_AddScript(Object* object, std::string const& name);\nScript* Object_GetScript(Object* object, std::string const& name);\nbool Object_RemoveScript(Object* object, std::string const& name);\n\nvoid Object_AddDeleteCallback(Object* object, ObjectDeleteCallback ptrFunc, int id);\nvoid Object_RemoveDeleteCallback(Object* object, ObjectDeleteCallback ptrFunc, int id);\n\n// --------------------------------------------------\n// --------------------------------------------------\n// Object\n\n\n\n// Transform\n// --------------------------------------------------\n// --------------------------------------------------\n\nvoid Transform_GenerateModel(Transform transform, MatrixWrapper& store);\n\n// --------------------------------------------------\n// --------------------------------------------------\n// Transform\n\n\n\n// Camera\n// --------------------------------------------------\n// --------------------------------------------------\n\nCamera* RegisterCamera(std::string const& name);\nCamera* GetCamera(std::string const& name);\n\nbool SetPrimaryCamera(Camera* camera);\nCamera* GetPrimaryCamera();\n\nvoid Camera_GenerateView(Camera* camera);\nvoid Camera_GenerateProjection(Camera* camera);\n\n::pPack::Vector3* Camera_Forward(Camera* camera);\n::pPack::Vector3* Camera_Right(Camera* camera);\n\nbool Camera_GetAngleBased(Camera* camera);\nvoid Camera_AngleBased(Camera* camera, bool b);\n\nvoid Camera_LookAtTarget(Camera* camera, ::pPack::Vector3* target);\n::pPack::Vector3* Camera_GetLookAtTarget(Camera* camera);\n\n// --------------------------------------------------\n// --------------------------------------------------\n// Camera\n\n\n\n\n// Mesh\n// --------------------------------------------------\n// --------------------------------------------------\n\nMesh* RegisterMesh(std::string const& name, void* vertices, size_t size, size_t count, bool dynamic, GLuint format);\nMesh* GetMesh(std::string const& name);\n\nGLuint RegisterFormat(std::string const& name, FormatStartInfo const* const formatStartInfo, GLuint count, bool autoOffset);\nGLuint GetFormat(std::string const& name);\n\nvoid ApplyStaticBuffer();\n\n// --------------------------------------------------\n// --------------------------------------------------\n// Mesh\n\n\n\n// Model\n// --------------------------------------------------\n// --------------------------------------------------\n\nModel* RegisterModel(std::string const& name);\nModel* GetModel(std::string const& name);\n\nbool Model_SetShader(Model* model, Shader* shader);\nbool Model_SetMesh(Model* model, Mesh* mesh);\n\nPropertyHolder* Model_GetProperties(Model* shader);\n\n// --------------------------------------------------\n// --------------------------------------------------\n// Model\n\n\n\n// Shader\n// --------------------------------------------------\n// --------------------------------------------------\n\nShader* RegisterShader(std::string const& name, ShaderInfo* startInfos, int count);\nShader* GetShader(std::string const& name);\n\nPropertyHolder* Shader_GetProperties(Shader* shader);\n\n// --------------------------------------------------\n// --------------------------------------------------\n// Shader\n\n\n\n// PropertyHolder\n// --------------------------------------------------\n// --------------------------------------------------\n\nbool PropertyHolder_AddProperty(PropertyHolder* holder, std::string const& name, void* value, VariableType type);\nbool PropertyHolder_SetProperty(PropertyHolder* holder, std::string const& name, void* value);\nbool PropertyHolder_SetOrAddProperty(PropertyHolder* holder, std::string const& name, void* value, VariableType type);\nvoid PropertyHolder_DeleteProperty(PropertyHolder* holder, std::string const& name);\n\nvoid* PropertyHolder_GetProperty(PropertyHolder* holder, std::string const& name);\n\n// --------------------------------------------------\n// --------------------------------------------------\n// PropertyHolder\n\n\n\n// Script\n// --------------------------------------------------\n// --------------------------------------------------\n\nbool RegisterScriptRaw(ScriptAllocateFunction scriptAllocate, std::string const& name, size_t size);\nScript* CreateScript(std::string const& name);\nchar const* GetScriptName(Script* script);\n\n// --------------------------------------------------\n// --------------------------------------------------\n// Script\n\n}; // namespace pumpkin\n\n\n#endif";
  std::stringstream hOS = std::stringstream();
  std::stringstream sOS = std::stringstream();
  std::string headerOutput = "temp.h";
#else
  std::ifstream is(input.c_str(), std::ios::in);
  std::ofstream hOS(headerOutput.c_str(), std::ios::out | std::ios::trunc);
  std::ofstream sOS(sourceOutput.c_str(), std::ios::out | std::ios::trunc);
#endif

  std::string line;
  std::string read;
  std::string rework;
  std::stringstream lineStream;
  size_t readPos;

  std::string nameName = "";
  bool useNamespace = false;

  auto now = std::chrono::system_clock::now();
  auto local = std::chrono::zoned_time{std::chrono::current_zone(), now};

  hOS << std::format("/*\n*\n* Function Declarations Header\n* Built {:%F %I:%M %p}\n*\n*/\n\n#ifndef __FTFP__BUILD__H\n\n", local);
  hOS << "// Windows specific for the moment\n#define APIENTRY __stdcall\n#define APIENTRYP APIENTRY *\n#define APIGET extern\n";
  hOS << "typedef void* (*PROCADDRESSFUNC)(char const* addr);\n\n";


  std::vector<std::pair<std::string, std::string>> listedFunctions = std::vector<std::pair<std::string, std::string>>();

  while (true) {
    std::getline(is, line);
    if (!is.good()) break;

    if (!line.empty()) {
      if (line[0] == 'n') {
        hOS << line << "\n\n";
        useNamespace = true;
        nameName = line;
        continue;
      } else if ((line.size() >= 8 && memcmp(line.c_str(), "#include", 8) == 0)) {
        hOS << line << "\n";
        continue;
      }
    } else {
      continue;
    }

    if (std::any_of(ignoreChars.begin(), ignoreChars.end(), CharPredicate(line[0]))) {
      continue;
    }

    lineStream << line;
    lineStream >> read; // Ignore first API definition or starter, not optional
    lineStream >> read;
    rework.clear();
    rework.append("typedef ");
    rework.append(read);
    if (read.size() >= 4 && (memcmp(read.c_str(), "char", 4) == 0 || (memcmp(read.c_str(), "const", 4) == 0))) {
      size_t readPos = static_cast<size_t>(lineStream.tellg()) + 1;
      rework.push_back(' ');
      lineStream >> read;
      if (read.size() >= 4 && (memcmp(read.c_str(), "char", 4) == 0 || (memcmp(read.c_str(), "const", 4) == 0))) {
        rework.append(read);
      } else {
        lineStream.seekg(readPos);
      }
    }

    std::getline(lineStream, read, '(');
    rework.append(ConvertStringName(read, listedFunctions));
    readPos = static_cast<size_t>(lineStream.tellg()) - 1;
    read = std::move(lineStream).str();
    for (int i = readPos; i < read.size(); i++) {
      rework.push_back(read[i]);
    }

    hOS << rework << "\n";
  }

  hOS << "\n\n// **************************************************\n// Declarations\n\n";

  for (auto const& [projName, name] : listedFunctions) {
    hOS << "APIGET " << projName << ' ' << name << ";\n";
  }

  hOS << "\nbool LoadFunctions(PROCADDRESSFUNC proc);\n";

  if (useNamespace) {
    hOS << "\n}\n";
  }

  hOS << "#endif";

  hOS.flush();
#if !(_DEBUG)
  hOS.close();
  is.close();
#endif



  sOS << std::format("/*\n*\n* Function Declarations Source\n* Built {:%F %I:%M %p}\n*\n*/\n\n", local);
  auto s = std::filesystem::path(headerOutput).filename();
  sOS << "#include " << std::filesystem::path(headerOutput).filename() << "\n\n";

  if (useNamespace) {
    sOS << nameName << "\n\n";
  }

  for (auto const& [projName, name] : listedFunctions) {
    sOS << projName << ' ' << name << " = NULL;\n";
  }

  sOS << "\n\nbool LoadFunctions(PROCADDRESSFUNC proc) {\n";
  sOS << "  if (!proc) return false;\n\n";
  for (auto const& [projName, name] : listedFunctions) {
    sOS << "  " << name << " = (" << projName << ")proc(\"" << name << "\"); if (!" << name << ") return false;\n";
  }

  sOS << "\n  return true;\n}\n\n";

  if (useNamespace) {
    sOS << '}';
  }

  sOS.flush();

#if _DEBUG
  std::cout << std::move(hOS).str() << std::string(50, '\n');
  std::cout << std::move(sOS).str() << '\n';
#else
  sOS.close();
#endif
}