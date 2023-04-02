// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "sourcehighlighter.h"

#include "xmlwriter.h"

#include <csnap/database/snapshot.h>

#include <csnap/model/filelist.h>

#include <array>
#include <algorithm>
#include <filesystem>
#include <set>

namespace csnap
{

static std::string get_symbol_symref(const Symbol& sym)
{
  return std::to_string(sym.id.value()) + std::string(".") + sym.name.c_str();
}

static std::array<char, 256> build_replacements()
{
  std::array<char, 256> r;

  for (size_t i(0); i < r.size(); ++i)
  {
    r[i] = (char)i;
  }

  for (char c : {'<', '>', '*', '"', '|'})
  {
    r[c] = '_';
  }

  return r;
}

static std::string get_symbol_symrefs_filename(const Symbol& sym)
{
  static const std::array<char, 256> replacements = build_replacements();

  std::string r = get_symbol_symref(sym);

  std::for_each(r.begin(), r.end(), [](char& c) {
    c = replacements[(unsigned char)c];
    });

  return r;
}

PathResolver::~PathResolver()
{

}

std::string PathResolver::relpath(const std::string& source, const std::string& target)
{
  if (target == source)
  {
    size_t n = target.rfind('/');

    if (n == std::string::npos)
      return target;
    else
      return target.substr(n + 1);
  }

  size_t minlen = std::min(source.size(), target.size());
  size_t i = 0;

  while (i < minlen && target.at(i) == source.at(i)) ++i;

  size_t nbdir = std::count(source.begin() + i, source.end(), '/');

  std::string r;
  r.reserve(target.size() - i + nbdir * 3);

  while (nbdir--)
  {
    r.append("../");
  }

  r.append(target.begin() + i, target.end());

  return r;
}


std::filesystem::path PathResolver::filePath(const File& f) const
{
  return f.path;
}

SourceHighlighter::SourceHighlighter(XmlWriter& xmlstream, const FileContent& fc, FileSema fm, const FileList& fs, const SymbolMap& ss, const DefinitionTable& defs) :
  m_default_path_resolver(),
  m_path_resolver(&m_default_path_resolver),
  xml(xmlstream),
  content(fc),
  sema(std::move(fm)),
  files(fs),
  symbols(ss),
  definitions(defs),
  m_file_path(sema.file->path)
{

}

std::string SourceHighlighter::rootPath() const
{
  std::string path = filePath().generic_string();
  size_t nbdir = std::count(path.begin(), path.end(), '/');

  std::string r;
  r.reserve(nbdir * 3);

  while (nbdir--)
  {
    r.append("../");
  }

  return r;
}

const std::filesystem::path& SourceHighlighter::filePath() const
{
  return m_file_path;
}

void SourceHighlighter::setFilePath(const std::filesystem::path& fp)
{
  m_file_path = fp;
}

PathResolver& SourceHighlighter::pathResolver() const
{
  return *m_path_resolver;
}

void SourceHighlighter::setPathResolver(PathResolver& resolver)
{
  m_path_resolver = &resolver;
}

void SourceHighlighter::writeLineSource(int l)
{
  if (l <= 0)
    return;

  if (m_current_line != l - 1)
    lexer.reset();
  else
    lexer.output.clear();

  const std::vector<std::string_view>& lines = content.lines;

  size_t i = size_t(l) - 1;

  if (i >= lines.size())
    return;

  lexer.tokenize(lines.at(i).data(), lines.at(i).size());

  // $TODO: the following if/else block could be improved
  if (m_current_line != l - 1 || !current_line_sema)
  {
    auto incitbegin = std::find_if(sema.includes.begin(), sema.includes.end(), [l](const Include& incl) {
      return incl.line >= l;
      });
    IncludeIterator incit{ incitbegin, incitbegin ==  sema.includes.end() ? incitbegin : std::next(incitbegin) };

    auto refitbegin = std::find_if(sema.references.begin(), sema.references.end(), [l](const SymbolReference& ref) {
      return ref.line >= l;
      });

    auto refitend = std::find_if(refitbegin, sema.references.end(), [l](const SymbolReference& ref) {
      return ref.line > l;
      });

    ReferenceIterator refit{refitbegin, refitend };

    SemaIterators semaits{ incit, refit };
    current_line_sema = std::make_unique<SemaIterators>(semaits);
  }
  else
  {
    auto incitbegin = std::find_if(current_line_sema->include.iterator(), sema.includes.cend(), [l](const Include& incl) {
      return incl.line >= l;
      });

    auto incitend = std::find_if(incitbegin, sema.includes.cend(), [l](const Include& incl) {
      return incl.line > l;
      });

    IncludeIterator incit{ incitbegin, incitend };

    auto refitbegin = std::find_if(current_line_sema->reference.iterator(), sema.references.cend(), [l](const SymbolReference& ref) {
      return ref.line >= l;
      });

    auto refitend = std::find_if(refitbegin, sema.references.cend(), [l](const SymbolReference& ref) {
      return ref.line > l;
      });

    ReferenceIterator refit{ refitbegin, refitend };

    current_line_sema->include = incit;
    current_line_sema->reference = refit;
  }

  m_current_line = l;

  writeLineSource(lexer.output, *current_line_sema);
}

int SourceHighlighter::currentLine() const
{
  return m_current_line;
}

const std::string_view& SourceHighlighter::currentText() const
{
  return content.lines[currentLine() - 1];
}

std::string SourceHighlighter::pathHref(const std::filesystem::path& p) const
{
  std::filesystem::path current_dir = filePath().parent_path();
  std::filesystem::path relpath = std::filesystem::relative(p, current_dir);
  return relpath.generic_u8string();
}

std::string SourceHighlighter::tagFromKeyword(const std::string& kw)
{
  static const std::set<std::string> kws = {
    "const",
    "bool",
    "void",
    "char",
    "int",
    "float",
    "double",
  };

  if (kws.find(kw) == kws.end())
    return "b";
  else
    return "em";
}

inline static bool is_class(csnap::Whatsit w)
{
  return w == csnap::Whatsit::CXXClass;
}

const std::string& SourceHighlighter::cssClass(const Symbol& sym)
{
  static const std::string cxxclass = "type cxxclass";
  static const std::string css_enum = "type enum";
  static const std::string css_struct = "type struct";
  static const std::string css_union = "type union";
  static const std::string css_typedef = "type typedef";
  static const std::string css_cxxtypealias = "type cxxtypealias";
  static const std::string ns = "namespace";
  static const std::string enmconstant = "enumconstant";
  static const std::string fn = "fn";
  static const std::string css_instancemethod = "member memfn";
  static const std::string css_staticvariable = "member staticvar";
  static const std::string css_staticmethod = "member staticfn";
  static const std::string css_constructor = "member constructor";
  static const std::string css_destructor = "member destructor";
  static const std::string css_conversionfunction = "member convfn";
  static const std::string memfield = "member field";
  static const std::string variable = "variable";
  static const std::string css_cxxinterface = "cxxinterface";
  static const std::string empty = "";

  switch (sym.kind)
  {
  case csnap::Whatsit::Typedef:
    return css_typedef;
  case csnap::Whatsit::Function:
    return fn;
  case csnap::Whatsit::Variable:
    return variable;
  case csnap::Whatsit::Field:
    return memfield;
  case csnap::Whatsit::EnumConstant:
    return enmconstant;
  case csnap::Whatsit::Enum:
    return css_enum;
  case csnap::Whatsit::Struct:
    return css_struct;
  case csnap::Whatsit::Union:
    return css_union;
  case csnap::Whatsit::CXXClass:
    return cxxclass;
  case csnap::Whatsit::CXXNamespace:
    return ns; 
  case csnap::Whatsit::CXXNamespaceAlias:
      return ns;
  case csnap::Whatsit::CXXStaticVariable:
    return css_staticvariable;
  case csnap::Whatsit::CXXStaticMethod:
    return css_staticmethod;
  case csnap::Whatsit::CXXInstanceMethod:
    return css_instancemethod;
  case csnap::Whatsit::CXXConstructor:
    return css_constructor;
  case csnap::Whatsit::CXXDestructor:
    return css_destructor;
  case csnap::Whatsit::CXXConversionFunction:
    return css_conversionfunction;
  case csnap::Whatsit::CXXTypeAlias:
    return css_cxxtypealias;
  case csnap::Whatsit::CXXInterface:
    return css_cxxinterface;
  default:
    return empty;
  }
}

int SourceHighlighter::getTokCol(std::string_view text, const cpptok::Token& tok)
{
  return static_cast<int>(tok.text().data() - text.data());
}

void SourceHighlighter::insertPrecedingSpaces(std::string_view txt, size_t& col, const cpptok::Token& tok)
{
  int tokcol = getTokCol(txt, tok);

  while (col < tokcol)
  {
    ++col;
    xml.writeCharacters(" ");
  }
}

void SourceHighlighter::writeToken(size_t& col, const cpptok::Token& tok)
{
  std::string txt = std::string(tok.text().data(), static_cast<int>(tok.text().length()));

  if (tok.isKeyword())
  {
    std::string tag = tagFromKeyword(txt);

    xml.writeStartElement(tag);
    xml.writeCharacters(txt);
    xml.writeEndElement();
  }
  else if (tok.isIdentifier() || tok.isOperator() || tok.isPunctuator())
  {
    xml.writeCharacters(txt);
  }
  else if (tok.isComment())
  {
    xml.writeStartElement("i");
    xml.writeCharacters(txt);
    xml.writeEndElement();
  }
  else if (tok.type() == cpptok::TokenType::Preproc)
  {
    xml.writeStartElement("u");
    xml.writeCharacters(txt);
    xml.writeEndElement();
  }
  else if (tok.type() == cpptok::TokenType::Include)
  {
    xml.writeStartElement("span");

    xml.writeAttribute("class", "include");
    xml.writeCharacters(txt);

    xml.writeEndElement();
  }
  else if (tok.isLiteral())
  {
    xml.writeStartElement("var");
    xml.writeCharacters(txt);
    xml.writeEndElement();
  }
  else
  {
    xml.writeCharacters(txt);
  }

  col += static_cast<int>(tok.text().length());
}

void SourceHighlighter::writeToken(size_t& col, TokenIterator& tokit)
{
  writeToken(col, *tokit);
  ++tokit;
}

bool SourceHighlighter::match(int line, const Include& incl)
{
  return line == incl.line;
}

bool SourceHighlighter::match(size_t col, const SymbolReference& symref)
{
  return symref.col == col + 1;
}

void SourceHighlighter::writeTokenAnnotated(size_t& col, TokenIterator& tokit, IncludeIterator& inclit)
{
  const File* incfile = files.get((*inclit).included_file_id);

  if (!incfile)
  {
    xml.writeStartElement("span");

    xml.writeAttribute("class", "include");

    writeToken(col, tokit);

    xml.writeEndElement();
  }
  else
  {
    std::filesystem::path target_path = pathResolver().filePath(*incfile);

    xml.writeStartElement("a");

    xml.writeAttribute("class", "include");

    std::string href = pathHref(target_path);

    xml.writeAttribute("href", href);

    writeToken(col, tokit);

    xml.writeEndElement();
  }
}

void SourceHighlighter::writeTokenAnnotated(size_t& col, TokenIterator& tokit, ReferenceIterator& refit)
{
  const SymbolReference& symref = *(refit);

  int symbol_id = symref.symbol_id;
  SymbolPtr symbol = symbols.at(SymbolId(symbol_id));
  std::string sym_ref = get_symbol_symref(*symbol);
  const std::string& css_class = cssClass(*symbol);

  auto do_write_token = [this, &col, &tokit, symbol]() {
    writeToken(col, tokit);
    
    if (symbol && symbol->kind == Whatsit::CXXDestructor)
    {
      writeToken(col, tokit);
    }
  };

  if (symref.flags & SymbolReference::Definition)
  {
    xml.writeStartElement("dfn");

    if (!css_class.empty())
      xml.writeAttribute("class", css_class);
    xml.writeAttribute("sym-ref", sym_ref);

    do_write_token();

    xml.writeEndElement();
  }
  else
  {
    SymbolReference symdef;

    if (definitions.hasUniqueDefinition(SymbolId(symbol_id), &symdef))
    {
      std::filesystem::path filepath = pathResolver().filePath(*files.get(FileId(symdef.file_id)));

      std::string href = pathHref(filepath);
      href += "#L" + std::to_string(symdef.line);

      {
        xml.writeStartElement("a");

        xml.writeAttribute("href", href);
        if (!css_class.empty())
          xml.writeAttribute("class", css_class);
        xml.writeAttribute("sym-ref", sym_ref);

        do_write_token();

        xml.writeEndElement();
      }

    }
    else
    {
      xml.writeStartElement("span");

      if (!css_class.empty())
        xml.writeAttribute("class", css_class);
      xml.writeAttribute("sym-ref", sym_ref);

      do_write_token();

      xml.writeEndElement();
    }
  }
}

void SourceHighlighter::writeTokens(size_t& col, TokenIterator& tokit, SemaIterators& sema)
{
  while (!tokit.atend())
  {
    insertPrecedingSpaces(currentText(), col, *tokit);

    if ((*tokit).type() == cpptok::TokenType::Include && !sema.include.atend() && match(m_current_line, *sema.include))
    {
      writeTokenAnnotated(col, tokit, sema.include);
      ++sema.include;
    }
    else if (!sema.reference.atend() && match(col, *sema.reference))
    {
      writeTokenAnnotated(col, tokit, sema.reference);
      ++sema.reference;
    }
    else
    {
      writeToken(col, *tokit);
      ++tokit;
    }
  }
}

void SourceHighlighter::writeLineSource(const std::vector<cpptok::Token>& tokens, SemaIterators& sema)
{
  TokenIterator tokit{ tokens.begin(), tokens.end() };
  size_t col = 0;

  writeTokens(col, tokit, sema);
}

} // namespace csnap
