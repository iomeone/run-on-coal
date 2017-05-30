#pragma once
#include "Elements/Element.h"

namespace ROC
{

class File : public Element
{
    enum FileMode { NoneMode = -1, ReadMode, WriteMode };
    FileMode m_type = FileMode::NoneMode;
    std::fstream *m_file;
    std::string m_path;
public:
    size_t Read(std::string &f_data, size_t f_lenght);
    size_t Write(const std::string &f_data);

    size_t GetSize();
    bool SetPosition(size_t f_pos);
    size_t GetPosition();
    inline bool IsEOF() { return (m_file ? m_file->eof() : false); }
    void Flush();

    inline void GetPath(std::string &f_string) { f_string.assign(m_path); }

    static bool Delete(Core *f_core, std::string &f_path);
    static bool Rename(Core *f_core, std::string &f_old, std::string &f_new);
protected:
    File();
    ~File();
    bool Create(const std::string &f_path, const std::string &f_rPath);
    bool Open(const std::string &f_path, const std::string &f_rPath, bool f_ro);

    friend class ElementManager;
};

}
