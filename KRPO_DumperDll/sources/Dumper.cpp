#include "Dumper.hpp"
#include <Windows.h>

Dumper::Dumper() : out(), fileName("") {
}

void Dumper::AddViewpoint(std::string &_name, ViewpointType _vpt, double *_val) {
  Viewpoint vp;
  vp.name = _name;
  vp.type = _vpt;
  vp.valuePtr = _val;
  viewpoints.push_back(vp);
}

void Dumper_PSF::BeginDump(std::string &_fileName) {
    // Найти позицию последней точки
    size_t dotPos = _fileName.find_last_of('.');

    // Проверить, что расширение .sp
    if (_fileName.substr(dotPos) == ".sp") {
        // Заменить .sp на .psf
        fileName = _fileName.substr(0, dotPos) + ".psf";
    }
 
    out.open(fileName);
    if (!out.is_open()) {
        std::cout << "FATAL! Could not begin dump!";
        return;
    }   
}

void Dumper_PSF::WriteHeader() {
    out << "HEADER" << std::endl
        << "\"PSFversion\" \"1.00\"" << std::endl
        << "\"simulator\" \"HSPICE\"" << std::endl
        << "\"runtype\" \"Transient Analysis\"" << std::endl
        << "TYPE" << std::endl
        << "\"node\" FLOAT DOUBLE PROP (" << std::endl
        << "\"key\" \"node\"" << std::endl
        << ")" << std::endl
        << "\"branch\" FLOAT DOUBLE PROP (" << std::endl
        << "\"key\" \"branch\"" << std::endl
        << ")" << std::endl
        << "\"sweep\" \"FLOAT DOUBLE\"" << std::endl
        << "SWEEP" << std::endl
        << "\"time\" \"sweep\"" << std::endl
        << "TRACE" << std::endl
        << "\"group\" GROUP " << std::to_string(viewpoints.size()).c_str() << std::endl;
        for (auto& vp : viewpoints) {
            out << "\"" << vp.name << "\" \"node\"" << std::endl;
        }
        out << "VALUE" << std::endl;
}

void Dumper_PSF::WriteValuesAtTime(double _t) {
    if (!out.is_open())
        return;
    out << "\"time\" " << std::scientific << _t << std::endl;
    out << "\"group\"" << std::endl;
    for (auto& vp : viewpoints) {
        out << std::scientific << *vp.valuePtr << std::endl;
    }
}

void Dumper_PSF::EndDump() {
    if (out.is_open())
        out.close();
}

PluginType GetType() {
  return PluginType::dumper;
}

void GetStringID(std::string &_id) {
    _id = "PSF";
}

Dumper *GetDumper() {
  return new Dumper_PSF;
}

void FreeDumper(Dumper *_p_dumper) {
  if (_p_dumper) {
    delete _p_dumper;
    _p_dumper = nullptr;
  }
}
