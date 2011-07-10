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


#include <tchar.h>
#include <string>
#include "base/PluginInterface.h"
#include "snippet_table.hpp"
#include "resource.h"

using namespace std;

const TCHAR NPP_PLUGIN_NAME[] = TEXT("Code Express"); 

const int FUNCTION_COUNT = 6;

const char* const ALLOWED_CHARS = 
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890*&_=;";
        
const char* const MARK_PATTERN = "@{[A-Za-z0-9]*}";
const int MARK_PATTERN_LEN = strlen(MARK_PATTERN);

FuncItem g_funcItems[FUNCTION_COUNT];
ShortcutKey g_shortKeys[FUNCTION_COUNT];
NppData g_nppData;
HMODULE g_hModule;

TCHAR g_pluginPath[MAX_PATH];

//----------------------------------------------------------------------------//

void InlineReplace();
void ShowInputBox();
void GotoNextMark();
void GotoPrevMark();
void AboutCodeExpress();


bool SetCommand(size_t index, const TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool checkOnInit);

inline LangType GetLangType();

//----------------------------------------------------------------------------//

extern "C" __declspec(dllexport) void setInfo(NppData nppData)
{
    g_nppData = nppData;
}


extern "C" __declspec(dllexport) const TCHAR * getName()
{
    return NPP_PLUGIN_NAME;
}


extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
    *nbF = FUNCTION_COUNT;
    return g_funcItems;
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
    switch (notifyCode->nmhdr.code) 
    {
        case NPPN_SHUTDOWN:

            break;        
        case NPPN_LANGCHANGED:
            TableManager::GetInstance().Load(GetLangType());
            break;
        default:
            return;
    }
}


extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{

    return TRUE;
}


#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE

//----------------------------------------------------------------------------//

extern "C" BOOL APIENTRY DllMain(HANDLE hModule, DWORD reseon, LPVOID lpReserved) 
{
    switch (reseon)    
    {
        case DLL_PROCESS_ATTACH:
        {
            // Get the path of the plugin 
            g_hModule = static_cast<HMODULE>(hModule);
            
            ::GetModuleFileName(g_hModule, g_pluginPath, MAX_PATH);
            TCHAR* backslash = _tcsrchr(g_pluginPath, _T('\\'));
            if (backslash != NULL)
                *backslash = _T('\0');
           
            // Plugin's menu
            g_shortKeys[0]._isAlt = false;
            g_shortKeys[0]._isCtrl = false;
            g_shortKeys[0]._isShift = false;
            g_shortKeys[0]._key = VK_TAB;
            SetCommand(0, TEXT("Inline replace"), InlineReplace, &g_shortKeys[0], false);
            
            g_shortKeys[1]._isAlt = true;
            g_shortKeys[1]._isCtrl = false;
            g_shortKeys[1]._isShift = false;
            g_shortKeys[1]._key = VK_RETURN;
            SetCommand(1, TEXT("Show input box"), ShowInputBox, &g_shortKeys[1], false);
            
            g_shortKeys[2]._isAlt = true;
            g_shortKeys[2]._isCtrl = false;
            g_shortKeys[2]._isShift = false;
            g_shortKeys[2]._key = 0xBC;
            SetCommand(2, TEXT("Goto prev mark"), GotoPrevMark, &g_shortKeys[2], false);            
            
            g_shortKeys[3]._isAlt = true;
            g_shortKeys[3]._isCtrl = false;
            g_shortKeys[3]._isShift = false;
            g_shortKeys[3]._key = 0xBE;
            SetCommand(3, TEXT("Goto next mark"), GotoNextMark, &g_shortKeys[3], false);

			SetCommand(4, TEXT("---"), NULL, NULL, false);
    		SetCommand(5, TEXT("About..."), AboutCodeExpress, NULL, false);
            
            
            // Pre-load some languages.
            // Others will be loaded on first use.
            TableManager::GetInstance().Load(TableManager::GLOBAL);
            TableManager::GetInstance().Load(L_TXT);
            
            break;
        }
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

//----------------------------------------------------------------------------//

//
// index:       zero based number to indicate the order of command
// cmdName:     the command name that you want to see in plugin menu
// pFunc:       the symbol of function (function pointer) associated with this command.
// sk:          optional. Define a shortcut to trigger this command
// checkOnInit: optional. Make this menu item be checked visually
// 
bool SetCommand(size_t index, const TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool checkOnInit)
{
    if (index >= FUNCTION_COUNT)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(g_funcItems[index]._itemName, cmdName);
    g_funcItems[index]._pFunc = pFunc;
    g_funcItems[index]._init2Check = checkOnInit;
    g_funcItems[index]._pShKey = sk;

    return true;
}


void ErrorMessage(const TCHAR* msg)
{
    ::MessageBox(g_nppData._nppHandle, msg, NPP_PLUGIN_NAME, MB_OK | MB_ICONERROR);
}

//----------------------------------------------------------------------------//

inline LangType GetLangType()
{
    LangType langtype;
	::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&langtype);
    return langtype;
}


inline HWND SciCurHandle() 
{
	int currentEdit;
	::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	return (currentEdit == 0)? 
    		g_nppData._scintillaMainHandle : 
            g_nppData._scintillaSecondHandle;
}


inline void SciSelectRange(HWND scintilla, int startPos, int endPos)
{
    ::SendMessage(scintilla, SCI_SETSELECTIONSTART, startPos, 0);
    ::SendMessage(scintilla, SCI_SETSELECTIONEND, endPos, 0);
}


inline void SciGetSelectedText(HWND scintilla, string& text)
{
    int len = static_cast<int>(::SendMessage(scintilla, SCI_GETSELTEXT, 0, 0));
    char* buf = new char[len];
    ::SendMessage(scintilla, SCI_GETSELTEXT, 0, (LPARAM) buf);
    text.assign(buf, buf + len - 1);    // Don't copy the last '\0' charactor
    delete[] buf;
}


inline bool SciFindInTarget(HWND scintilla, const char* text, int textLen, int start, int end, int flag)
{
    ::SendMessage(scintilla, SCI_SETSEARCHFLAGS, flag, 0);
    ::SendMessage(scintilla, SCI_SETTARGETSTART, start, 0);
    ::SendMessage(scintilla, SCI_SETTARGETEND, end, 0);
    
    int f = static_cast<int>(::SendMessage(scintilla, SCI_SEARCHINTARGET, textLen, (LPARAM) text));
    
    if (f != -1) {
        start = static_cast<int>(::SendMessage(scintilla, SCI_GETTARGETSTART, 0, 0));
        end = static_cast<int>(::SendMessage(scintilla, SCI_GETTARGETEND, 0, 0));
        SciSelectRange(scintilla, start, end);
        return true;
    }
    return false;
}


// Determin indent and EOL
void SciGetIndent(HWND scintilla, int pos, string& indent)
{
    // Ignore indent over 60 chars.
    const int kIndentMax = 60;
    
    int lineNumber = static_cast<int>(::SendMessage(scintilla, SCI_LINEFROMPOSITION, pos, 0));
    int lineStart = static_cast<int>(::SendMessage(scintilla, SCI_POSITIONFROMLINE, lineNumber, 0));
    int eolMode = static_cast<int>(::SendMessage(scintilla, SCI_GETEOLMODE, 0, 0));
    
    char buf[kIndentMax + 1];
    
    switch (eolMode) {
        case SC_EOL_CRLF:
            indent = "\r\n";
            break;
        case SC_EOL_CR:
            indent = "\r";
            break;
        case SC_EOL_LF:
            indent = "\n";
            break;
	}
    
    Sci_TextRange range;
    range.chrg.cpMin = lineStart;
    range.chrg.cpMax = (pos > lineStart + kIndentMax )? lineStart + kIndentMax : pos;
    range.lpstrText = buf;

    int len = static_cast<int>(::SendMessage(scintilla, SCI_GETTEXTRANGE, 0, (LPARAM) &range)); 
    
    indent.append(buf, len);
        
    size_t i = indent.find_first_not_of(" \t\r\n");
    if(i != string::npos) {
        indent.erase(i);
    }
}


// If current selection is "@{}", erase it.
void EraseMark(HWND scintilla)
{
    int len = static_cast<int>(::SendMessage(scintilla, SCI_GETSELTEXT, 0, 0));

    if (len == 4) {
        char buf[4];
        static_cast<int>(::SendMessage(scintilla, SCI_GETSELTEXT, 0, (LPARAM) buf));
        if (strcmp("@{}", buf) == 0) {
            ::SendMessage(scintilla, SCI_REPLACESEL, 0, (LPARAM) "");
        }
    }
}


void ParseTag(const string& input, string& tag)
{
    size_t tagEnd = input.find_first_of(" =");
    tag = input.substr(0, tagEnd);
}


int ParseVals(const string& input, string* vals)
{
    if (input.empty())
        return 0;

    size_t searchStart = 0;
    int i = 0;

    while (i < SnippetData::VAR_MAX && searchStart < input.size()) {
        size_t valStart = input.find_first_not_of("\t ", searchStart);
        if (valStart == string::npos)
            break;
        
        int sepPos = input.find_first_of(';', valStart);
        
        if (sepPos != string::npos) {
            vals[i] = input.substr(valStart, sepPos - valStart);
            searchStart = sepPos + 1;
            i++;
        } else {
            vals[i] = input.substr(valStart, string::npos);
            i++;
            break;
        }       
    }
    return i;
}


// Insert indent and replace all EOL chars in same pass
void ReindentText(string& text, const string& indent)
{
    size_t lineStart = 0;
    size_t lineEnd = text.find_first_of("\n\r");
    
    // If there is only single line, nothing change
    if (lineEnd == string::npos)
        return;

    string result(text.begin(), text.begin() + lineEnd);
    result.append(indent);
    
    while (true) {
        // Simply skip all EOL chars
        if (text[lineEnd] == '\r' && text[lineEnd + 1] == '\n') {
            lineStart = lineEnd + 2;
        } else {
            lineStart = lineEnd + 1;
        }
        
        if (lineStart >= text.size())
            break;
            
        lineEnd = text.find_first_of("\n\r", lineStart);
        if (lineEnd != string::npos) {
            result.append(text, lineStart, lineEnd - lineStart);
            result.append(indent);
        } else {
            result.append(text, lineStart, text.size() - lineStart);
            break;
        }
    }

    text.swap(result);
}


void FindReplace(string& text, const string& find, const string& replacement)
{
    size_t repLen = replacement.size();
    size_t findLen = find.size();
    size_t pos = text.find(find, 0);
    
    while (pos != string::npos) {
        text.replace(pos, findLen, replacement);
        if (pos + repLen >= text.size())
            break;
        pos = text.find(find, pos + repLen);
    }
}


void SubstituteVars(const SnippetData& data, const string* values, string& result)
{
    if (values == 0)
        return;

    result = data.codeTemplate;
    // replace all @{var} mark
    for (int i = 0; i < data.varCount; ++i) {
        string mark("@{");
        mark += data.vars[i].name;
        mark += "}";

        if (!values[i].empty()) {
            if (data.vars[i].formatter.empty()) {
                FindReplace(result, mark, values[i]);
            } else {
                string fmt = data.vars[i].formatter;
                FindReplace(fmt, "@{_}", values[i]);
                FindReplace(result, mark, fmt);
            }        
        } else if (data.vars[i].optional) {
            FindReplace(result, mark, "");
        }
    }
}


SnippetData* LookupTag(const string& tag)
{
    SnippetTable& langTable = 
            TableManager::GetInstance().GetTable(GetLangType());
    
    SnippetTable::iterator it = langTable.find(tag);
    
    if (it == langTable.end()) {
        SnippetTable& globalTable = 
                TableManager::GetInstance().GetTable(TableManager::GLOBAL);
                
        it = globalTable.find(tag);
        
        if (it == globalTable.end()) {
            return 0;
        }
    }
    return &(it->second);
}


bool InterpretInput(HWND scintilla, const string& input, const string& selected, int curPos)
{
    string tag, result;
    ParseTag(input, tag);
    SnippetData* data = LookupTag(tag);
    
    if (!data) 
        return false;
        
    string vals[SnippetData::VAR_MAX];
    if (tag.size()+1 < input.size()) {
        ParseVals(input.substr(tag.size() + 1), vals);
    }
    SubstituteVars(*data, vals, result);
    
    // Find @{SEL} marks and replace with selected text
    if (!selected.empty()) {
        FindReplace(result, "@{SEL}", selected);
    } else {
        FindReplace(result, "@{SEL}", "@{}");
    }
    
    string indent;
    SciGetIndent(scintilla, curPos, indent);
    ReindentText(result, indent);
    
    ::SendMessage(scintilla, SCI_REPLACESEL, 0, (LPARAM) result.c_str());
    
    SciFindInTarget(
            scintilla, 
            MARK_PATTERN, 
            MARK_PATTERN_LEN,
            curPos, 
            curPos + result.size(),
            SCFIND_REGEXP);
    
    EraseMark(scintilla);
    return true;
}

//----------------------------------------------------------------------------//

void ProcDialogInput(const string& input)
{
    HWND scintilla = SciCurHandle();
    
    int selStart = static_cast<int>(::SendMessage(scintilla, SCI_GETSELECTIONSTART, 0, 0));
    int selEnd = static_cast<int>(::SendMessage(scintilla, SCI_GETSELECTIONEND, 0, 0));    
    int curPos = static_cast<int>(::SendMessage(scintilla, SCI_GETCURRENTPOS, 0, 0));
    
    // Fetch selection
    string selected;
    SciGetSelectedText(scintilla, selected);
    
    // Try to replace current word.
    bool succ = InterpretInput(scintilla, input, selected, selStart);
    
    // If replacing fail, restore scintilla state;
    if (!succ) {
        ::SendMessage(scintilla, SCI_SETCURRENTPOS, curPos, 0);
        SciSelectRange(scintilla, selStart, selEnd);
    }
}


INT_PTR CALLBACK InputBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const int kInputMax = 128;

    switch (uMsg) {
        case WM_COMMAND:
        {
            int id = LOWORD(wParam);
            
            switch (id) {
                case IDOK: 
                {
                    char buf[kInputMax];
                    HWND hInput = ::GetDlgItem(hWnd, IDC_INPUT);
                    int len = ::GetWindowTextA(hInput, buf,  kInputMax);
                    
                    string input(buf, buf + len);
                    ProcDialogInput(input);
                }
                case IDCANCEL: 
                    ::EndDialog(hWnd, id);
                    break;
                case IDC_INPUT:
                {
                    // Take a peek at what user has input, 
                    // and see if it's in the snippet table.
                    char buf[kInputMax];
                    int len = ::GetWindowTextA((HWND)lParam, buf,  kInputMax);
                    
                    string input(buf, buf + len), tag;
                    ParseTag(input, tag);
                    SnippetData* data = LookupTag(tag);
                    HWND hHint = ::GetDlgItem(hWnd, IDC_HINT);
                    
                    // If the tag is in the snippet table, 
                    // then give the user more informations.
                    if (data != 0) {
                        tag.append("  ");
                        for (int i = 0; 
                            i < SnippetData::VAR_MAX && !data->vars[i].name.empty(); 
                            ++i
                        ) {
                            tag.append(data->vars[i].name);
                            tag.append("; ");
                        }
                        SetWindowTextA(hHint, tag.c_str());                     
                    } else {
                        SetWindowTextA(hHint, " "); 
                    }
                    break;
                }
            }
            break;
        }
        case WM_INITDIALOG:
        {
            HWND hInput = ::GetDlgItem(hWnd, IDC_INPUT);
            ::SendMessageA(hInput, EM_SETLIMITTEXT, kInputMax, 0); 
            // text = reinterpret_cast<string*>(lParam);
            break;
        }
        default:
            return FALSE; 
    }
    return TRUE;
}


//----------------------------------------------------------------------------//

void InlineReplace()
{
    HWND scintilla = SciCurHandle();
    
    // Perform tabbing if some text is already selected.
    int selStart = static_cast<int>(::SendMessage(scintilla, SCI_GETSELECTIONSTART, 0, 0));
    int selEnd = static_cast<int>(::SendMessage(scintilla, SCI_GETSELECTIONEND, 0, 0));
    
    if(selEnd != selStart) {
        ::SendMessage(scintilla, SCI_TAB, 0, 0);
        return;
    }
    
    // re-define word for scintilla
    ::SendMessage(scintilla, SCI_SETWORDCHARS, 0, (LPARAM) ALLOWED_CHARS); 
    
    // Select current word 
    int curPos = static_cast<int>(::SendMessage(scintilla, SCI_GETCURRENTPOS, 0, 0));
    int startPos = static_cast<int>(::SendMessage(scintilla, SCI_WORDSTARTPOSITION, curPos, (LPARAM)true));
    int endPos = static_cast<int>(::SendMessage(scintilla, SCI_WORDENDPOSITION, curPos, (LPARAM)true));
    
    SciSelectRange(scintilla, startPos, endPos);

    string input;
    SciGetSelectedText(scintilla, input);
    
    // Try to replace current word.
    bool succ = InterpretInput(scintilla, input, string(""), startPos);
    
    // If replacing fail, restore scintilla state.
    if (!succ) {
        ::SendMessage(scintilla, SCI_SETCURRENTPOS, curPos, 0);
        SciSelectRange(scintilla, selStart, selEnd);
        ::SendMessage(scintilla, SCI_TAB, 0, 0);
    }

    ::SendMessage(scintilla, SCI_SETCHARSDEFAULT, 0, 0);
}


void ShowInputBox()
{
    INT_PTR ret = DialogBoxParam(
            (HINSTANCE) g_hModule, 
            MAKEINTRESOURCE(IDD_INPUT_DLG),
            g_nppData._nppHandle,
            InputBoxProc,
            (LPARAM) 0);
}


void GotoPrevMark()
{
    HWND scintilla = SciCurHandle();
    ::SendMessage(scintilla, SCI_SEARCHANCHOR, 0, 0);    
    ::SendMessage(scintilla, SCI_SEARCHPREV, SCFIND_REGEXP, (LPARAM) MARK_PATTERN); 
    
    EraseMark(scintilla);
}



void GotoNextMark()
{
    HWND scintilla = SciCurHandle();
    int selEnd = static_cast<int>(::SendMessage(scintilla, SCI_GETSELECTIONEND, 0, 0));
    int docLen = static_cast<int>(::SendMessage(scintilla, SCI_GETLENGTH, 0, 0));
    SciFindInTarget(
            scintilla, 
            MARK_PATTERN, 
            MARK_PATTERN_LEN,
            selEnd, 
            docLen,
            SCFIND_REGEXP);
            
    EraseMark(scintilla);
}


void AboutCodeExpress()
{
    const TCHAR* aboutMsg = 
            TEXT("Code Express 0.2 (Proof of concept version)\r\n")
            TEXT("Copyright (C) 2011 Novus. \r\n");
    const TCHAR* title = TEXT("About Code Express...");
    ::MessageBox(g_nppData._nppHandle, aboutMsg, title, MB_OK);
}


