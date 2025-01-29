#include "PluginManager.hpp"
#include <direct.h>
#include <iostream>

void wchar_t2string(const wchar_t *wchar, std::string &dest) {
  dest.erase();
  size_t index = 0;
  while (0 != wchar[index])
    dest += (char)wchar[++index];
}

void string2wchar_t(const std::string &str, wchar_t dest[256]) {
  size_t index = 0;
  while (index < str.size()) {
    dest[index] = (wchar_t)str[index];
    ++index;
  }
  dest[index] = 0;
}

std::string GetExeDirectory() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH); // Получаем полный путь к .exe

    // Находим последний символ '\' и обрезаем путь до директории
    std::string path(exePath);
    size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        return path.substr(0, lastSlash + 1); // Включая слеш в конце
    }
    return ""; // Если что-то пошло не так
}

bool PluginManager::ListPluginsOnStartup() {
  std::cout << "Поиск доступных для использования модулей в каталоге с модулями..." << std::endl;
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind = nullptr;

  PluginInfo pi;
  do {
    if (hFind == nullptr) {
      /*char cwd[1024];
      getcwd(cwd, sizeof(cwd));
      std::cout << "Директория поиска модулей: " << cwd << std::endl;*/
      std::string exeDir = GetExeDirectory();
      std::string modulesPath = exeDir + "modules\\*.dll"; // Путь к папке с DLL

      hFind = FindFirstFile(modulesPath.c_str(), &FindFileData);

      if (INVALID_HANDLE_VALUE == hFind) {
        std::cout << "\tНе найдено ни одного модуля" << std::endl << std::endl;
        return false;
      }
    }

    pi.fileName = "modules\\" + std::string(FindFileData.cFileName);
    HMODULE hDll = LoadLibrary(pi.fileName.c_str());
    if (!hDll) {
        DWORD error = GetLastError();
        std::cerr << "Error loading DLL: " << error << std::endl;
    }
    if (NULL == hDll) {
      std::cout << "\tПредупреждение. Найден файл " << pi.fileName << ", но он не является плагином." << std::endl;
      continue;
    }
    type_func_ptr funcType = (type_func_ptr)GetProcAddress(hDll, "GetType");
    if (!funcType) {
      std::cout << "\tПредупреждение. Найдена библиотека " << pi.fileName << ", но она не является нашим плагином." << std::endl;
      continue;
    }
    pi.type = funcType();

    idstr_func_ptr funcId = (idstr_func_ptr)GetProcAddress(hDll, "GetStringID");
    if (!funcId) {
      std::cout << "\tПредупреждение. Найдена библиотека " << pi.fileName << ", но она не является нашим плагином." << std::endl;
      continue;
    }
    funcId(pi.idString);
    FreeLibrary(hDll);

    plugins.push_back(pi);
  } while (FindNextFile(hFind, &FindFileData));

  int numSolvers = 0,
      numDumpers = 0;

  for (size_t i = 0; i < plugins.size(); ++i)
    switch (plugins[i].type) {
    case PluginType::solver:
      ++numSolvers;
      break;
    case PluginType::dumper:
      ++numDumpers;
      break;
    }

  if (!numSolvers)
    std::cout << "\tОшибка. Среди модулей нет решателя СЛАУ, нечем решать ММ схемы." << std::endl;
  if (!numDumpers)
    std::cout << "\tОшибка. Среди модулей нет дампера, нечем сохранять результаты моделирования." << std::endl;

  if(numSolvers && numDumpers)
    std::cout << "\tРешателей найдено: " << numSolvers << std::endl << "\tДамперов найдено: " << numDumpers << std::endl;

  std::cout << "Поиск модулей завершён." << std::endl << std::endl;

  return (numSolvers && numDumpers);
}

PluginManager::PluginManager() : ok(false), hSolverDll(nullptr), hDumperDll(nullptr), p_getSolverFunc(nullptr), p_getDumperFunc(nullptr), p_freeSolverFunc(nullptr), p_freeDumperFunc(nullptr) {
  ok = ListPluginsOnStartup();
}

PluginManager::~PluginManager() {
  if (hSolverDll) {
    FreeLibrary(hSolverDll);
    hSolverDll = nullptr;
  }
  if (hDumperDll) {
    FreeLibrary(hDumperDll);
    hDumperDll = nullptr;
  }
}

bool PluginManager::IsOk() {
  return ok;
}

void PluginManager::List() {
  std::cout << "Список модулей:" << std::endl;
  for (size_t i = 0; i < plugins.size(); ++i) {
    switch (plugins[i].type) {
    case PluginType::solver:
      std::cout << i + 1 << ". '" << plugins[i].fileName << "' - solver, ID string='" << plugins[i].idString << "'" << std::endl;
      break;
    case PluginType::dumper:
      std::cout << i + 1 << ". '" << plugins[i].fileName << "' - dumper, ID string='" << plugins[i].idString << "'" << std::endl;
      break;
    default:
      std::cout << "*" << i + 1 << ". '" << plugins[i].fileName << "' - unknown type: " << (int)plugins[i].type << ", ID string='" << plugins[i].idString << "'" << std::endl;
    }
  }
  std::cout << std::endl ;
}

bool PluginManager::SelectSolver(std::string &_solverName) {
  for (size_t i = 0; i < plugins.size(); ++i) {
    if (plugins[i].type != PluginType::solver)
      continue;
    if (plugins[i].idString == _solverName) {
      hSolverDll = LoadLibrary(plugins[i].fileName.c_str());
      p_getSolverFunc = (get_solver_func_ptr)GetProcAddress(hSolverDll, "GetSolver");
      p_freeSolverFunc = (free_solver_func_ptr)GetProcAddress(hSolverDll, "FreeSolver");
    }
  }
  if (!p_getSolverFunc || !p_freeSolverFunc) {
    std::cout << "Ошибка. Не могу найти нужные функции в решателе." << std::endl;
    return false;
  }
  return true;
}

bool PluginManager::SelectDumper(std::string &_dumperName) {
  for (size_t i = 0; i < plugins.size(); ++i) {
    if (plugins[i].type != PluginType::dumper)
      continue;
    if (plugins[i].idString == _dumperName) {
      hDumperDll = LoadLibrary(plugins[i].fileName.c_str());
      p_getDumperFunc = (get_dumper_func_ptr)GetProcAddress(hDumperDll, "GetDumper");
      p_freeDumperFunc = (free_dumper_func_ptr)GetProcAddress(hDumperDll, "FreeDumper");
    }
  }
  if (!p_getDumperFunc || !p_freeDumperFunc) {
    std::cout << "Ошибка. Не могу найти нужные функции в дампре." << std::endl;
    return false;
  }
  return true;
}

std::string PluginManager::GetDefaultSolverName() {
  for (size_t i = 0; i < plugins.size(); ++i)
    if (plugins[i].type == PluginType::solver)
      return plugins[i].idString;
  return std::string("");
}

std::string PluginManager::GetDefaultDumperName() {
  for (size_t i = 0; i < plugins.size(); ++i)
    if (plugins[i].type == PluginType::dumper)
      return plugins[i].idString;
  return std::string("");
}

Solver *PluginManager::GetSolver() {
  Solver *p_solver = p_getSolverFunc();
  return p_solver;
}

Dumper *PluginManager::GetDumper() {
  Dumper *p_dumper = p_getDumperFunc();
  return p_dumper;
}

void PluginManager::FreeSolver(Solver *_p_solver) {
  p_freeSolverFunc(_p_solver);
}

void PluginManager::FreeDumper(Dumper *_p_dumper) {
  p_freeDumperFunc(_p_dumper);
}
