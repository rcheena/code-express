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


#include <cassert>
#include <cstdio>
#include <cstring>
#include "rapidxml/rapidxml.hpp"
#include "snippet_table.hpp"

using namespace std;

extern TCHAR g_pluginPath[MAX_PATH];

void ErrorMessage(const TCHAR* msg);


TableManager::TableManager()
{
    for (int i = 0; i < TABLE_COUNT; ++i) {
        tables_[i] = 0;
    }
}


TableManager::~TableManager()
{
    Clean();
}

// Load snippet file for lang.
// Note that even if reading file or parsing failed, an empty table
// will still be created.
void TableManager::Load(int lang)
{
    assert(lang >= 0);
    assert(lang < TABLE_COUNT);

    Clean(lang);
    tables_[lang] = new SnippetTable;

    TCHAR filePath[MAX_PATH];
    _stprintf(
            filePath,
            _T("%s\\Config\\CodeExpress\\%s.%s"),
            g_pluginPath, LANG_NAMES[lang], _T("xml"));
   
    HANDLE fin;
    fin = CreateFile(
            filePath, 
            GENERIC_READ, 
            FILE_SHARE_READ, 
            NULL,
            OPEN_EXISTING,
            0,
            NULL);
    
    if (fin != INVALID_HANDLE_VALUE) {
        // Can't handle a snippet file up to 4GB(2^32) bytes, which is unlikely to happen.
        DWORD readSize;
        DWORD fileSize = GetFileSize(fin, NULL);
        char* content = new char[fileSize + 1];

        ::ReadFile(fin, content, fileSize, &readSize, NULL);
        content[readSize] = '\0';
        Parse(content, *tables_[lang]);
        delete[] content;
    }
    CloseHandle(fin);
}


void TableManager::Clean(int lang)
{
    assert(lang >= 0);
    assert(lang < TABLE_COUNT);

    if (tables_[lang] != 0) {
        delete tables_[lang];
        tables_[lang] = 0;
    }
}


void TableManager::Clean()
{
    for (int i = 0; i < TABLE_COUNT; ++i) {
        Clean(i);
    }
}


SnippetTable& TableManager::GetTable(int lang)
{
    assert(lang >= 0);
    assert(lang < TABLE_COUNT);

    // Load on first use
    if (tables_[lang] == 0) {
        Load(lang);
    }
    return *tables_[lang];
}


bool TableManager::Parse(char* src, SnippetTable& table)
{
    using namespace rapidxml;
    try {
        xml_document<> doc;
        doc.parse<0>(src);

        xml_node<>* root = doc.first_node();

        if (strcmp("CodeExpress", root->name()) != 0) {
            return false;
        }

        for (xml_node<>* snippet = root->first_node("snippet");
                snippet; snippet = snippet->next_sibling("snippet")) {
            // A snippet must have a tag. If not, it will be skipped.
            xml_attribute<>* tagAttr = snippet->first_attribute("tag");
            if (tagAttr) {
                SnippetData& entry = table[string(tagAttr->value())];
                ParseSnippet(snippet, entry);
            } 
        }
    } catch (parse_error& er) {
        // ErrorMessage(_T("XML format error!"));
        return false;
    }
    return true;
}


void TableManager::ParseSnippet(rapidxml::xml_node<>* snippet, SnippetData& entry)
{
    using namespace rapidxml;

    // Read "var" nodes.
    int i = 0;
    for (xml_node<>* var = snippet->first_node("var");
            var && i < SnippetData::VAR_MAX;
            var = var->next_sibling("var")) {
        // Get "name" attribute.
        xml_attribute<>* nameAttr = var->first_attribute("name");
        if (!nameAttr)
            break;
        entry.vars[i].name = nameAttr->value();

        // Get "optional" attribute.
        xml_attribute<>* optionalAttr = var->first_attribute("optional");
        if (optionalAttr) {
            if (optionalAttr->value()[0] == '0' ||
                optionalAttr->value()[0] == '\0') {
                entry.vars[i].optional = false;
            } else {
                entry.vars[i].optional = true;
            }
        } else {
            entry.vars[i].optional = false;
        }

        // See if there is a format string between <var><![CDATA[ ... ]]></var>
        xml_node<>* fmt = var->first_node();
        if (fmt && fmt->type() == node_cdata) {
            entry.vars[i].formatter = fmt->value();
        }
        ++i;
    }
    entry.varCount = i;

    // Read "code" node.
    xml_node<>* code = snippet->first_node("code"); //->first_node();
    if (code && code->first_node() && code->first_node()->type() == node_cdata)  {
        entry.codeTemplate = code->first_node()->value();
    }
}

