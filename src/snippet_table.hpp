// This file is part of Code Express
// Copyright (C) 2011 Novus

// This program is free software; you can redistribute it and/or 
// modify it under the terms of the GNU General Public License as 
// published by the Free Software Foundation; either version 2 of 
// the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
// General Public License for more details. You should have received 
// a copy of the GNU General Public License along with this program; 
// if not, write to the Free Software Foundation, Inc., 59 Temple 
// Place, Suite 330, Boston, MA 02111-1307 USA


#ifndef SNIPPET_TABLE_HPP_
#define SNIPPET_TABLE_HPP_

#include <string>
#include <map>
#include <tchar.h>
#include <windows.h>
#include "base/Notepad_plus_msgs.h"

// Forward declaration of xml_node
namespace rapidxml 
{
    template<class Ch> class xml_node;
}


struct SnippetData
{
    enum {VAR_MAX = 5};
    
    struct VarInfo 
    {
        std::string name;
        std::string formatter; 
        bool optional;
    };
    
    VarInfo vars[VAR_MAX];
    std::string codeTemplate;
    int varCount;
};


typedef std::map<std::string, SnippetData> SnippetTable;



class TableManager
{
public:
    enum {  
        GLOBAL = L_EXTERNAL + 1,
        TABLE_COUNT
    };
    
    ~TableManager();
    
    void Load(int lang);
    void Save(int lang);
    
    void Clean(int lang);
    void Clean();
    
    SnippetTable& GetTable(int lang);
    
    static TableManager& GetInstance()
    {
        static TableManager inst;
        return inst;
    }
    
private:
    TableManager();
    TableManager(const TableManager&);
    TableManager& operator=(const TableManager&);

    bool Parse(char* input, SnippetTable& table);
    void ParseSnippet(rapidxml::xml_node<char>* snippet, SnippetData& entry);
    
    SnippetTable* tables_[TABLE_COUNT];
};


const TCHAR* const LANG_NAMES[TableManager::TABLE_COUNT] = {
    _T("txt"),      _T("php"),      _T("c"),        _T("cpp"),      _T("cs"),
    _T("objc"),     _T("java"),     _T("rc"),       _T("html"),     _T("xml"), 
    _T("makefile"), _T("pascal"),   _T("batch"),    _T("ini"),      _T("nfo"), 
    _T("user"),     _T("asp"),      _T("sql"),      _T("vb"),       _T("js"), 
    _T("css"),      _T("perl"),     _T("python"),   _T("lua"),      _T("tex"), 
    _T("fortran"),  _T("bash"),     _T("flash"),    _T("nsis"),     _T("tcl"), 
    _T("lisp"),     _T("scheme"),   _T("asm"),      _T("diff"),     _T("props"),
    _T("ps"),       _T("ruby"),     _T("smalltalk"),_T("vhdl"),     _T("kix"),
    _T("au3"),      _T("caml"),     _T("ada"),      _T("verilog"),  _T("matlab"),
    _T("haskell"),  _T("inno"),     _T("search"),   _T("cmake"),    _T("yaml"), 
    _T("external"), _T("global")
};

#endif
