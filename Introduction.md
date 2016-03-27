# Introduction #

You can find some demo at:

http://youtu.be/7xNeUbsScUc

# Install #

Download and extract this plug-in, copy "Plugin" folder under your Notepad++ install position.

# How to use #

  * TAB-trigger: simply key-in the "tag" of a snippet, then hit TAB key.
  * Visit placeholder marks: "ALT+," and "ALT+."
  * Show the input dialog: "ALT+Enter".

# How to edit snippet files #

The snippet files are stored in XML format, one XML file for one language. Global.xml is always active. To create a snippet file for a language, you need to create a XML file with name _"language-name.xml"_ under _"Notepad++/plugin/config/CodeExpress"_. Supported language names are

| text     | php      | c        | cpp      | cs       | objc       |
|:---------|:---------|:---------|:---------|:---------|:-----------|
| java     | rc       | html     | xml      | makefile | pascal     |
| batch    | ini      | ascii    | user     | asp      | sql        |
| vb       | js       | css      | perl     | python   | lua        |
| tex      | fortran  | bash     | flash    | nsis     | tcl        |
| lisp     | scheme   | asm      | diff     | props    | ps         |
| ruby     | smalltalk| vhdl     | kix      | au3      | caml       |
| ada      | verilog  | matlab   | haskell  | inno     | search     |
| cmake    | yaml     | cobol    | gui4cli  | d        | powershell |
| r        | jsp      | external | global   |          |            |

There is no special editor for snippet files by now. Notepad++ itself is a good XML editor, using collapse level 2 (shortcut ALT+2) will give you a clean view. You can open the snippet file for the language your are using through menu _"Plugins/Code Express/Edit Current Snippet"_.

## File format ##

Basic layout of a snippet file is like:

```xml
<?xml version="1.0"?>
<CodeExpress>

<snippet tag="gpl">...

Unknown end tag for &lt;/snippet&gt;



<snippet tag="lorem">...

Unknown end tag for &lt;/snippet&gt;



...



Unknown end tag for &lt;/CodeExpress&gt;

```


Individual snippet looks like:

```xml
<snippet tag="demo">

<var name="foo"> <![CDATA[  foo=@{_} ]]> 

Unknown end tag for &lt;/var&gt;



<var name="bar" optional="1">

Unknown end tag for &lt;/var&gt;



<code>

<![CDATA[This is just a demo.
@{foo}
@{bar}
]]>

```



Unknown end tag for &lt;/snippet&gt;



Unknown end tag for &lt;/code&gt;



  * A snippet must have a "tag" attribute.
  * A snippet can have none or some variables, currently **6** at most.
  * A vaiable must have a "name" attribute, and can have an optional "optional" attribute.
    * optional="1" indicate that if no value is given, this variable will not apear in the final code.
  * Code template should be put inside `<code>`...`</code>` tag with CDATA section markup.
  * You can add hot spots inside the code template:
    * Any pattern matches "`@{[A-Za-z0-9]*}`" will be treated as a hot spot, which can be visited using "ALT+," and "ALT+."
    * @{} will be erased automatically when it is selected by "ALT+," or "ALT+."
    * If the text inside @{...} matches a variable name, it also serves as a placeholder of the variable.
    * @{SEL} is reserved, means insert selected text here. If no text is selected before triggering this snippet, @{SEL} will be replaced by @{}.

# Known Problems #

