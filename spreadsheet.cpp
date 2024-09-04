#ifndef __PROGTEST__
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <variant>
#include <optional>
#include <compare>
#include <charconv>
#include <span>
#include <utility>
#include "expression.h"
using namespace std::literals;
using CValue = std::variant<std::monostate, double, std::string>;

constexpr unsigned                     SPREADSHEET_CYCLIC_DEPS                 = 0x01;
constexpr unsigned                     SPREADSHEET_FUNCTIONS                   = 0x02;
constexpr unsigned                     SPREADSHEET_FILE_IO                     = 0x04;
constexpr unsigned                     SPREADSHEET_SPEED                       = 0x08;
//constexpr unsigned                     SPREADSHEET_PARSER                      = 0x10;
#endif /* __PROGTEST__ */
#include <regex>

class CPos;
std::pair<int,int> CPos_parser(std::string_view str);
class CSpreadsheet;


class Expr // Parent class used for polymorphic implementation & evaluation of formulas
{
public:
  virtual CValue eval(CSpreadsheet *spreadsheat) const = 0;
  virtual int getType() const = 0;
  virtual ~Expr() = default; 
};

using ExprPtr = std::shared_ptr<Expr>;

class Numeric : public Expr
{
private:
  double value;
public:
  explicit Numeric(double val) : value(val) {};
  int getType()const override{ return 0; }
  CValue eval(CSpreadsheet *spreadsheat) const override
  {
    return value;
  }
};

class Text : public Expr
{
public:
  explicit Text(std::string &val) : value(std::move(val)) {}
  int getType()const override{ return 0; }
  CValue eval(CSpreadsheet *spreadsheat) const override
  {
    return value;
  }
private:
  std::string value;
};

class Sum : public Expr {
private:
    ExprPtr left, right;
public:
    Sum(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
    int getType()const override{ return 0; }
    CValue eval(CSpreadsheet *spreadsheet) const override {
        auto lVal = left->eval(spreadsheet);
        auto rVal = right->eval(spreadsheet);
        if (std::holds_alternative<std::monostate>(lVal) || std::holds_alternative<std::monostate>(rVal))
            return std::monostate();
        if (std::holds_alternative<double>(lVal) && std::holds_alternative<double>(rVal))
          return std::get<double>(lVal) + std::get<double>(rVal);
        else if(std::holds_alternative<double>(lVal) && std::holds_alternative<std::string>(rVal))
          return std::to_string(std::get<double>(lVal)) + std::get<std::string>(rVal);
        else if(std::holds_alternative<std::string>(lVal) && std::holds_alternative<double>(rVal))
          return  std::get<std::string>(lVal )+ std::to_string(std::get<double>(rVal));
        else
          return  std::get<std::string>(lVal ) + std::get<std::string>(rVal);

    }
};

class Subtraction : public Expr {
private:
    ExprPtr left, right;

public:
    Subtraction(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
    int getType()const override{ return 0; }

        
    CValue eval(CSpreadsheet *spreadsheet) const override {
        auto lVal = left->eval(spreadsheet);
        auto rVal = right->eval(spreadsheet);
        if (!std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
            return std::monostate();
        else if(std::holds_alternative<std::monostate>(lVal))
            return std::get<double>(rVal);
        else if(std::holds_alternative<std::monostate>(rVal))
            return std::get<double>(lVal);

        return std::get<double>(lVal) - std::get<double>(rVal);
    }
};

class Negative : public Expr
{
private:
  ExprPtr left;
public:
  Negative(ExprPtr l) : left(std::move(l)) {}
  int getType() const override { return 0; }
  CValue eval(CSpreadsheet *spreadsheat) const override
  {
    if (!std::holds_alternative<double>(left->eval(spreadsheat)))
      return std::monostate();
    return -(std::get<double>(left->eval(spreadsheat)));
  }
};

class Multiplication : public Expr
{
private:
  ExprPtr left, right;
public:
  Multiplication(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
  int getType() const override { return 0; }
  CValue eval(CSpreadsheet *spreadsheat) const override
  {
    auto lVal = left->eval(spreadsheat);
    auto rVal = right->eval(spreadsheat);

    if (!std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
      return std::monostate(); // or 0?

    return std::get<double>(lVal) * std::get<double>(rVal);
  }
};

class Division : public Expr
{
private:
  ExprPtr left, right;
public:
  Division(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
  int getType() const override { return 0; }
  CValue eval(CSpreadsheet *spreadsheat) const override
  {
    auto lVal = left->eval(spreadsheat);
    auto rVal = right->eval(spreadsheat);
    if ( !std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal) || (std::get<double>(right->eval(spreadsheat)) == 0) )
    {
      return std::monostate();
    }
    return std::get<double>(lVal) / std::get<double>(rVal);
  }
};

class Power : public Expr
{
private:
  ExprPtr left, right;

public:
  Power(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
  int getType() const override { return 0; }
  CValue eval(CSpreadsheet *spreadsheat) const override
  {

    auto lVal = left->eval(spreadsheat);
    auto rVal = right->eval(spreadsheat);

    if (!std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
      return std::monostate();
    return pow(std::get<double>(lVal), std::get<double>(rVal));
  }
};

class Equal:public Expr{
private:
  ExprPtr left, right;
public:
  Equal(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
  int getType() const override { return 0; }
  CValue eval(CSpreadsheet* spreadsheet) const override {
    auto lVal = left->eval(spreadsheet);
    auto rVal = right->eval(spreadsheet);

    if (std::holds_alternative<std::string>(lVal) && std::holds_alternative<std::string>(rVal))
      return (std::get<std::string>(lVal) == std::get<std::string>(rVal))? CValue(1.) :CValue(0.) ;
    if(std::holds_alternative<double>(lVal) && std::holds_alternative<double>(rVal))
      return (std::get<double>(lVal) == std::get<double>(rVal))? CValue(1.) :CValue(0.) ;
    else return std::monostate(); 
  }
};

class NotEqual:public Expr{
private:
  ExprPtr left, right;
public:
  NotEqual(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
  int getType() const override { return 0; }
  CValue eval(CSpreadsheet* spreadsheet) const override {
    auto lVal = left->eval(spreadsheet);
    auto rVal = right->eval(spreadsheet);
    if (std::holds_alternative<std::string>(lVal) && std::holds_alternative<std::string>(rVal))
      return (std::get<std::string>(lVal) != std::get<std::string>(rVal))? CValue(1.) :CValue(0.) ;
    if (!std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
      return std::monostate(); 
    return (std::get<double>(lVal) != std::get<double>(rVal))? CValue(1.) :CValue(0.) ;
  }
};

class LowerThen:public Expr{
private:
  ExprPtr left, right;
public:
  LowerThen(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
  int getType() const override { return 0; }
  CValue eval(CSpreadsheet* spreadsheet) const override {
    auto lVal = left->eval(spreadsheet);
    auto rVal = right->eval(spreadsheet);
    if (std::holds_alternative<std::string>(lVal) && std::holds_alternative<std::string>(rVal))
      return (std::get<std::string>(lVal) < std::get<std::string>(rVal))? CValue(1.) :CValue(0.) ;
    if (!std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
      return std::monostate(); // or 0?
    return (std::get<double>(lVal) < std::get<double>(rVal))? CValue(1.) :CValue(0.) ;
  }
};

class LowerEq:public Expr{
private:
  ExprPtr left, right;
public:
  LowerEq(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
  int getType() const override { return 0; }
  CValue eval(CSpreadsheet* spreadsheet) const override {
    auto lVal = left->eval(spreadsheet);
    auto rVal = right->eval(spreadsheet);
    if (std::holds_alternative<std::string>(lVal) && std::holds_alternative<std::string>(rVal))
      return (std::get<std::string>(lVal) <= std::get<std::string>(rVal))? CValue(1.) :CValue(0.) ;
    if (!std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
      return std::monostate(); 
    return (std::get<double>(lVal) <= std::get<double>(rVal))? CValue(1.) :CValue(0.) ;
  }
};

class GreaterThen:public Expr{
private:
  ExprPtr left, right;
public:
  GreaterThen(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
  int getType() const override { return 0; }
  CValue eval(CSpreadsheet* spreadsheet) const override {
    auto lVal = left->eval(spreadsheet);
    auto rVal = right->eval(spreadsheet);
    if (std::holds_alternative<std::string>(lVal) && std::holds_alternative<std::string>(rVal))
      return (std::get<std::string>(lVal) > std::get<std::string>(rVal))? CValue(1.) :CValue(0.) ;
    if (!std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
      return std::monostate(); 
    return (std::get<double>(lVal) > std::get<double>(rVal))? CValue(1.) :CValue(0.) ;
  }
};

class GreaterEqual:public Expr{
private:
  ExprPtr left, right;
public:
  GreaterEqual(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
  int getType() const override { return 0; }


  CValue eval(CSpreadsheet* spreadsheet) const override {
    auto lVal = left->eval(spreadsheet);
    auto rVal = right->eval(spreadsheet);
    if (std::holds_alternative<std::string>(lVal) && std::holds_alternative<std::string>(rVal))
      return (std::get<std::string>(lVal) >= std::get<std::string>(rVal))? CValue(1.) :CValue(0.) ;
    if (!std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
      return std::monostate();
    return (std::get<double>(lVal) >= std::get<double>(rVal))? CValue(1.) :CValue(0.) ;
  }
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class expBuilder : public CExprBuilder
{
public:
  expBuilder(){};

  void opAdd() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<Sum>(left, right));
    return;
  };
  void opSub() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<Subtraction>(left, right));
    return;
  };
  void opMul() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<Multiplication>(left, right));
    return;
  };
  void opDiv() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<Division>(left, right));
    return;
  };
  void opPow() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<Power>(left, right));
    return;
  };
  void opNeg() override
  {
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<Negative>(left));
    return;
  };
  void opEq() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<Equal>(left, right));
    return;
  };
  void opNe() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<NotEqual>(left, right));
    return;
  };
  void opLt() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<LowerThen>(left, right));
    return;
  };
  void opLe() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<LowerEq>(left, right));
    return;
  };
  void opGt() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<GreaterThen>(left, right));
    return;
  };
  void opGe() override
  {
    auto right = exprStack.top();
    exprStack.pop();
    auto left = exprStack.top();
    exprStack.pop();
    exprStack.push(std::make_shared<GreaterEqual>(left, right));
    return;
  };
  void valNumber(double val) override
  {
    exprStack.push(std::make_shared<Numeric>(val));
    return;
  };
  void valString(std::string val) override
  {
    exprStack.push(std::make_shared<Text>(val));
    return;
  };
  void valReference(std::string val) override; // @note is on the bottom of the code, due to incopetence arrange code differently

  void valRange(std::string val) override
  {
    return;
  };
  void funcCall(std::string fnName, int paramCount) override { return; }; 

  ExprPtr getResult() const
  {
    if (exprStack.empty())
    {
      throw std::runtime_error("Error: Attempted to evaluate an empty expression.");
    }
    return exprStack.top();
  }

  std::stack<ExprPtr> exprStack;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class CCell
{
public:
  enum type
  {
    NUMERIC,
    TEXT,
    FORMULA
  };
  CCell();
  CCell(const std::string &value);
  void extractCellReferences(const std::string &formula);
  void Set(const std::string &text);
  void Clear();
  CValue getValue(CSpreadsheet *spreadsheet) const;
  type get_type();
  std::string getContent() const;
  expBuilder formula; //->this will be parsed
  std::string content_editor(int deltaColum, int deltaRow);
  std::set<std::string> references;

private:
  bool is_cyclic = false;
  type content_type;
  CValue content;
  std::string original_content;
};

void CCell::extractCellReferences(const std::string &formula)  // Extracts references from string declaring formula/expression that has references to another cells
{
  int i = 0, len = formula.length();

  while (i < len)
  {
    if (std::isalpha(formula[i]))
    {
      std::string colPart, rowPart;
      bool columnComplete = false;
      if (i > 0 && formula[i - 1] == '$')
      {
        i++;
      }
      while (i < len && std::isalpha(formula[i]))
      {
        colPart += std::toupper(formula[i]);
        i++;
        columnComplete = true;
      }

      if (i < len && formula[i] == '$' && columnComplete)
      {
        i++;
      }

      while (i < len && std::isdigit(formula[i]))
      {
        rowPart += formula[i];
        i++;
      }

      if (!colPart.empty() && !rowPart.empty())
      {
        references.insert(colPart + rowPart);
      }
    }
    else
    {
      i++;
    }
  }
}

std::string CCell::getContent()const{
  return original_content;
};

std::string updateFormula(std::string formula, int deltaRow, int deltaCol) { // Updates refernces in a string declaring calculation formula
    std::string result;
    int i = 0;
    int len = formula.length();
    if(formula.find("\"")!=std::string::npos) return formula;
    while (i < len) {
        if (std::isalpha(formula[i])) { 
            bool absCol = false, absRow = false;
            std::string colPart, rowPart;

            if (i > 0 && formula[i-1] == '$') {
                absCol = true; 
                result.pop_back(); 
            }

            while (i < len && std::isalpha(formula[i])) {
                colPart += formula[i++];
            }

            if (i < len && formula[i] == '$') {
                absRow = true;
                i++; 
            }

            while (i < len && std::isdigit(formula[i])) {
                rowPart += formula[i++];
            }

            int colIndex = 0;
            for (char c : colPart) {
                colIndex = colIndex * 26 + (toupper(c) - 'A' + 1);
            }

            if (!absCol) {
                colIndex += deltaCol;
            }

            std::string newColPart;
            while (colIndex > 0) {
                colIndex--;
                newColPart = char('A' + (colIndex % 26)) + newColPart;
                colIndex /= 26;
            }

            int rowIndex = std::stoi(rowPart);
            if (!absRow) {
                rowIndex += deltaRow;
            }

            result += (absCol ? "$" : "") + newColPart + (absRow ? "$" : "") + std::to_string(rowIndex);
        } else {
            result += formula[i++];
        }
    }

    return result;
}
CCell::type CCell::get_type(){
  return this->content_type;
} ;
CCell::CCell(){};
CCell::CCell(const std::string& value)  {
    if (value[0] == '=') {
        parseExpression(value, formula);
        extractCellReferences(value);
        content_type = type::FORMULA;
    }
    else {
        try {
            double numericValue = std::stod(value);
            content = numericValue;
            content_type = type::NUMERIC;
        }
        catch (const std::invalid_argument&) {
            content = std::string(value);
            content_type = type::TEXT;
        }
    }
    original_content = value;
}
CValue CCell::getValue(CSpreadsheet *spreadsheet) const
{
  if (content_type == FORMULA)
  {
    auto result = formula.getResult()->eval(spreadsheet);
    CValue A = CValue(result);
    return A;
  }
  else if (content_type == NUMERIC)
  {

    double numericValue = std::get<double>(content);
    return numericValue;
  }
  else if (content_type == TEXT)
  {
    std::string textValue = std::get<std::string>(content);
    return textValue;
  }
  else if (content.index() == 0)
  {
    return CValue(); 
  }
  else
  {
    throw std::runtime_error("Unknown type stored in CCell");
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class CPos
{
public:
  CPos(std::string_view str);
  friend std::pair<int, int> CPos_parser(std::string_view str);
  bool operator<(const CPos &other) const;
  CPos offset(int dx, int dy) const;
  void print() const;
  std::string getCode() const;
  std::pair<unsigned int, unsigned int> getRaC() const;
  std::pair<int, int> CPos_parser(std::string_view str);
  CPos copy();

  mutable unsigned int row;
  mutable unsigned int column;
  bool relative_column, relative_row;
  std::string code;
};
CPos::CPos(std::string_view str)
{
  relative_column = true;
  relative_row = true;
  std::pair<int, int> position = CPos_parser(str);
  code = str;
  row = position.first;
  column = position.second;
}
std::pair<unsigned int, unsigned int> CPos::getRaC() const {
    return {row, column};
};

void CPos::print() const{
  std::cout<<code<<"-ROW: "<<row<<" COLUMN: "<<column<<std::endl;
};

std::string CPos::getCode() const{
  return code;
}

std::pair<int, int> CPos::CPos_parser(std::string_view str) //Parses string declaration of cell to numeric representation
{
  std::pair<int, int> position;
  int column = 0;
  int row = 0;
  size_t index = 0;
  bool absolute_column = false;
  bool absolute_row = false;

  bool column_exists = false;
  bool row_exists = false;

  if (!str.empty() && str[index] == '$')
  {
    absolute_column = true;
    index++;
  }

  while (index < str.size() && std::isalpha(str[index]))
  {
    column = column * 26 + (std::tolower(str[index]) - 'a' + 1);
    ++index;
    column_exists = true;
  }

  if (index < str.size() && str[index] == '$' && column_exists)
  {
    absolute_row = true;
    index++;
  }

  if (index < str.size() && column_exists)
  {
    size_t processed_length = 0;
    std::string row_part = std::string(str.substr(index));
    row = std::stoi(row_part, &processed_length);
    if (processed_length != row_part.length())
    {
      throw std::invalid_argument("Invalid row format in input: " + std::string(str));
    }
    row_exists = true;
  }

  if (row_exists && column_exists)
  {
    position.first = row;
    position.second = column;
    relative_column = !absolute_column;
    relative_row = !absolute_row;
    return position;
  }
  else
    throw std::invalid_argument("");
}

std::string back_to_code(unsigned int row, unsigned int column) // Parses numeric represntation of position into original string representation
{ 
  std::string columnLabel;

  while (column > 0)
  {
    int remainder = (column - 1) % 26;
    char letter = 'A' + remainder;
    columnLabel = letter + columnLabel;
    column = (column - 1) / 26;
  }
  std::string rowLabel = std::to_string(row);
  return columnLabel + rowLabel;
}

bool CPos::operator<( const CPos & other ) const {
  if(this->row == other.row )
    return this->column < other.column;
  return this->row < other.row;
};

CPos CPos::offset(int dx, int dy) const {    // Generate a new code for the new position by adding offsets to the current position
  return CPos(back_to_code(row + dy, column + dx));
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class CSpreadsheet
{
public:
  static unsigned capabilities()
  {
    return SPREADSHEET_CYCLIC_DEPS;
  }
  CSpreadsheet(){};
  bool load(std::istream &is);
  bool save(std::ostream &os) const;
  bool setCell(CPos pos, std::string contents);
  bool dfsCycleCheck(const CPos &pos, std::map<CPos, int> &state);
  CValue getValue(CPos pos);
  void copyRect(CPos dst, CPos src, int w = 1, int h = 1);
  std::map<CPos, CCell> page; 
};

bool CSpreadsheet::dfsCycleCheck(const CPos &pos, std::map<CPos, int> &state)
{
  if (state[pos] == 1)
    return true; // Cycle found, this cell is already being visited
  if (state[pos] == 2)
    return false; // Already fully processed this cell

  state[pos] = 1;
  CCell &cell = page[pos];
  if (cell.get_type() == CCell::type::FORMULA)
  {
    for (const auto &refStr : cell.references)
    {
      CPos refPos(refStr);
      if (page.find(refPos) != page.end() && dfsCycleCheck(refPos, state))
      {
        return true;
      }
    }
  }
  state[pos] = 2;
  return false;
}

CValue CSpreadsheet::getValue(CPos pos)
{
  std::map<CPos, int> state;
  if (page.find(pos) != page.end() && !dfsCycleCheck(pos, state))
    return page[pos].getValue(this);
  return CValue();
};

bool CSpreadsheet::setCell(CPos pos, std::string contents)
{
  CCell tmp;

  try
  {
    tmp = CCell(contents);
  }
  catch (...)
  {
    return false; // Return false if the cell contents are invalid
  }
  page[pos] = tmp;
  return true; 
}

bool CSpreadsheet::save(std::ostream &os) const
{
  for (const auto &entry : page)
  {
    const auto &pos = entry.first;   // CPos object
    const auto &cell = entry.second; // CCell object

    std::string encodedContent = cell.getContent();

    os << "BUNK" << pos.getCode() << "CONT" << encodedContent << char(31); 
    if (!os)
    {
      std::cerr << "Error writing to output stream." << std::endl;
      return false;
    }
  }
  return true;
}

bool CSpreadsheet::load(std::istream &is)
{
  std::string line;
  while (getline(is, line, char(31)))
  {
    if (line.empty())
      continue; 
    size_t bunkPos = line.find("BUNK");
    size_t contPos = line.find("CONT");
    if (bunkPos == std::string::npos || contPos == std::string::npos || bunkPos > contPos)
    {
      std::cerr << "Error: Invalid line format. Line: " << line << std::endl;
      return false;
    }

    std::string cellPos = line.substr(bunkPos + 4, contPos - (bunkPos + 4));
    std::string contents = line.substr(contPos + 4);

    CPos posObject(cellPos);
    if (!setCell(posObject, contents))
    {
      std::cerr << "Error setting cell at position " << cellPos << " with content " << contents << std::endl;
      return false;
    }
  }
  return true;
}

void CSpreadsheet::copyRect(CPos dst, CPos src, int w, int h)
{
  int deltaRow = dst.getRaC().first - src.getRaC().first;
  int deltaCol = dst.getRaC().second - src.getRaC().second;

  std::map<CPos, CCell> tmpCells;
  for (int row = 0; row < h; ++row)
  {
    for (int col = 0; col < w; ++col)
    {
      CPos currentSrc = src.offset(col, row);
      CPos currentDst = dst.offset(col, row);

      if (page.find(currentSrc) != page.end())
      {
        if (page[currentSrc].get_type() == CCell::type::FORMULA)
        {
          CCell srcCell(updateFormula(page[currentSrc].getContent(), deltaRow, deltaCol));
          tmpCells[currentDst] = srcCell;
        }
        else if ((page[currentSrc].get_type() == CCell::type::TEXT) || (page[currentSrc].get_type() == CCell::type::NUMERIC))
        {
          CCell srcCell(page[currentSrc].getContent());
          tmpCells[currentDst] = srcCell;
        }
        else
        {
          CCell srcCell(page[currentSrc].getContent());
          tmpCells[currentDst] = srcCell;
        }
      }
      else if (page.find(currentDst) != page.end() && page.find(currentSrc) == page.end())
      {
        tmpCells[currentDst]=CCell();
      }
  
    }
  }

  for(const auto & [pos,cell]:tmpCells){
    page[pos] = cell;
  }

}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class Reference : public Expr
{
public:
  CPos pos;

  Reference(CPos m_pos)
      : pos(m_pos) {}
  int getType() const override { return 1; }
  CValue eval(CSpreadsheet *spreadsheet) const override
  {
    if (spreadsheet->page.find(pos) == spreadsheet->page.end())
    {
      return std::monostate();
    }
    return spreadsheet->getValue(pos);
  }
};

void expBuilder::valReference(std::string val)
{
  CPos pos(val);
  exprStack.push(std::make_shared<Reference>(pos));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef __PROGTEST__

bool                                   valueMatch                              ( const CValue                        & r,
                                                                                 const CValue                        & s )

{
  if ( r . index () != s . index () )
    return false;
  if ( r . index () == 0 )
    return true;
  if ( r . index () == 2 )
    return std::get<std::string> ( r ) == std::get<std::string> ( s );
  if ( std::isnan ( std::get<double> ( r ) ) && std::isnan ( std::get<double> ( s ) ) )
    return true;
  if ( std::isinf ( std::get<double> ( r ) ) && std::isinf ( std::get<double> ( s ) ) )
    return ( std::get<double> ( r ) < 0 && std::get<double> ( s ) < 0 )
           || ( std::get<double> ( r ) > 0 && std::get<double> ( s ) > 0 );
  return fabs ( std::get<double> ( r ) - std::get<double> ( s ) ) <= 1e8 * DBL_EPSILON * fabs ( std::get<double> ( r ) );
}

//==============================================================================================================================



//==============================================================================================================================


void basic_tests(){ 
  CSpreadsheet x0, x1, x2, x3;
  std::ostringstream oss;
  std::istringstream iss;
  std::string data;
  assert ( x0 . setCell ( CPos ( "A1" ), "10" ) );
  assert ( x0 . setCell ( CPos ( "A2" ), "20.5" ) );
  assert ( x0 . setCell ( CPos ( "A3" ), "3e1" ) );
  assert ( x0 . setCell ( CPos ( "A4" ), "=40" ) );
  assert ( x0 . setCell ( CPos ( "A5" ), "=5e+1" ) );
  assert ( x0 . setCell ( CPos ( "A6" ), "raw text with any characters, including a quote \" or a newline\n" ) );
  assert ( x0 . setCell ( CPos ( "A7" ), "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\"" ) );

  assert ( x0 . setCell ( CPos ( "A8" ), "=3 + 4 * 20" ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A8" ) ), CValue ( 83.0 ) ) );

  assert ( valueMatch ( x0 . getValue ( CPos ( "A1" ) ), CValue ( 10.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A2" ) ), CValue ( 20.5 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A3" ) ), CValue ( 30.0 ) ) );

  assert ( x0 . setCell ( CPos ( "A9" ), "40" ) );
  assert ( x0 . setCell ( CPos ( "B9" ), "2" ) );

  assert ( x0 . setCell ( CPos ( "A10" ), "5" ) );

  assert ( x0 . setCell ( CPos ( "B1" ), "=A1+A2*A3" ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B1" ) ), CValue ( 625.0 ) ) );

  assert ( x0 . setCell ( CPos ( "B2" ), "=A2-A1" ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B2" ) ), CValue ( 10.5 ) ) );

  assert ( x0 . setCell ( CPos ( "BBWB1" ), "=10" ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "BBWB1" ) ), CValue ( 10.0 ) ) );

  x0 . getValue ( CPos ( "A4" ) );
  x0 . getValue ( CPos ( "A5" ) );
  x0 . getValue ( CPos ( "A6" ) );
  x0 . getValue ( CPos ( "A7" ) );

  assert ( x2 . setCell ( CPos ( "A1" ), "= A4 + A7" ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "A1" ) ), CValue () ) );

  assert ( x2 . setCell ( CPos ( "A3" )," 10."));
  assert ( x2 . setCell ( CPos ( "A4" )," 30."));
  assert ( x2 . setCell ( CPos ( "A5" )," 40."));

  assert ( x2 . setCell ( CPos ( "D0" ), "10" ) );
  assert ( x2 . setCell ( CPos ( "D1" ), "20" ) );
  assert ( x2 . setCell ( CPos ( "D2" ), "30" ) );
  assert ( x2 . setCell ( CPos ( "D3" ), "40" ) );
  assert ( x2 . setCell ( CPos ( "D4" ), "50" ) );
  assert ( x2 . setCell ( CPos ( "E0" ), "60" ) );
  assert ( x2 . setCell ( CPos ( "E1" ), "70" ) );
  assert ( x2 . setCell ( CPos ( "E2" ), "80" ) );
  assert ( x2 . setCell ( CPos ( "E3" ), "90" ) );
  assert ( x2 . setCell ( CPos ( "E4" ), "100" ) );
  assert ( x2 . setCell ( CPos ( "F10" ), "110" ) );
  assert ( x2 . setCell ( CPos ( "F11" ), "120" ) );
  assert ( x2 . setCell ( CPos ( "F12" ), "=130" ) );
  assert ( x2 . setCell ( CPos ( "F13" ), "=140" ) );
  x2 . copyRect ( CPos ( "G11" ), CPos ( "F10" ), 1, 4 );

  assert ( valueMatch ( x2 . getValue ( CPos ( "F10" ) ), CValue ( 110.0 ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "F11" ) ), CValue ( 120.0 ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "F12" ) ), CValue ( 130.0 ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "F13" ) ), CValue ( 140.0 ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "F14" ) ), CValue() ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "G10" ) ), CValue() ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "G11" ) ), CValue ( 110.0 ) ) ); 
  assert ( valueMatch ( x2 . getValue ( CPos ( "G12" ) ), CValue ( 120.0 ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "G13" ) ), CValue ( 130.0 ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "G14" ) ), CValue ( 140.0 ) ) );

  x2. copyRect ( CPos ( "B3" ), CPos ( "A3" ), 1, 3 );
  assert ( valueMatch ( x2 . getValue ( CPos ( "B3" ) ), CValue ( 10. ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "B4" ) ), CValue ( 30. ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "B5" ) ), CValue ( 40. ) ) );

  assert ( x1 . setCell ( CPos ( "A0" ), "\n\n\n\n\\\\" ) );
  assert ( x1 . setCell ( CPos ( "A1" ), "\n \n guten tag \\n" ) );

  oss . clear ();
  oss . str ( "" );
  assert ( x1 . save ( oss ) );
  data = oss . str ();
  iss . clear ();
  iss . str ( data );
  assert ( x3 . load ( iss ) );

  assert ( valueMatch ( x3 . getValue ( CPos ( "A0" ) ), CValue ( "\n\n\n\n\\\\" ) ) );
  assert ( valueMatch ( x3 . getValue ( CPos ( "A1" ) ), CValue ( "\n \n guten tag \\n" ) ) );

  assert ( x0 . setCell ( CPos ( "A1" ), "10" ) );
  assert ( x0 . setCell ( CPos ( "A2" ), "0" ) );
  assert ( x0 . setCell ( CPos ( "A3" ), "=A1/A2" ) );

    assert ( valueMatch ( x0 . getValue ( CPos ( "A3" ) ), CValue () ) );
  assert ( x0 . setCell ( CPos ( "A2" ), "Hello\n" ) );
  assert ( x0 . setCell ( CPos ( "A3" ), "its me" ) );
  assert ( x0 . setCell ( CPos ( "A1" ), "=A2+A3" ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A1" ) ), CValue ("Hello\nits me") ) );
  assert ( x0 . setCell ( CPos ( "A3" ), "3" ) );
  assert ( x0 . setCell ( CPos ( "A1" ), "=A2+A3" ) );


  assert ( x1 . setCell ( CPos ( "A0" ), "22" ) );
  assert ( x1 . setCell ( CPos ( "A1" ), "99" ) );
  x1 . copyRect ( CPos ( "A0" ), CPos ( "A1" ), 1, 1 );
  assert ( valueMatch ( x1 . getValue ( CPos ( "A0" ) ), x1 . getValue ( CPos ( "A1" ) ) ) );

};

void save_load_tests(){
  CSpreadsheet x0, x1;
  std::ostringstream oss;
  std::istringstream iss;
  std::string data;

  // Test 1: Save and Load a simple numeric value
  x0.setCell(CPos("A1"), "123.456");
  assert(x0.save(oss));
  data = oss.str();
  iss.str(data);
  assert(x1.load(iss));
  assert(valueMatch(x1.getValue(CPos("A1")), CValue(123.456)));

  // Clear stream states and contents for the next test
  oss.str("");
  oss.clear();
  iss.clear();

  // Test 2: Save and Load a string value
  x0.setCell(CPos("B1"), "\"Hello,\t world!\"");
  assert(x0.save(oss));
  data = oss.str();
  iss.str(data);
  assert(x1.load(iss));
  assert(valueMatch(x1.getValue(CPos("B1")), CValue("\"Hello,\t world!\"")));

  oss.str("");
  oss.clear();
  iss.clear();

  // Test 3: Save and Load a formula (as a string for simplicity)
  x0.setCell(CPos("C1"), "=\"SUM(A1,B1)\"");
  assert(x0.save(oss));
  data = oss.str();
  iss.str(data);
  assert(x1.load(iss));
  assert(valueMatch(x1.getValue(CPos("C1")), CValue("SUM(A1,B1)")));

  oss.str("");
  oss.clear();
  iss.clear();

  // Test 4: Save and Load with special characters
  x0.setCell(CPos("D1"), "\"New\\nLine and \\\"escaped\\\" quotes\"");
  assert(x0.save(oss));
  data = oss.str();
  iss.str(data);
  assert(x1.load(iss));
  assert(valueMatch(x1.getValue(CPos("D1")), CValue("\"New\\nLine and \\\"escaped\\\" quotes\"")));

  oss.str("");
  oss.clear();
  iss.clear();

  // Test 5: Save and Load multiple cells
  x0.setCell(CPos("A2"), "789.1011");
  x0.setCell(CPos("B2"), "\"Another\\ \n \t \' \" \\\\ \a \b test\"");
  x0.setCell(CPos("C2"), "=\"Another\\ \n \t \'  hello \\\\ \a \b test\"");

  assert(x0.save(oss));
  data = oss.str();
  iss.str(data);
  assert(x1.load(iss));
  assert(valueMatch(x1.getValue(CPos("A2")), CValue(789.1011)));
  assert(valueMatch(x1.getValue(CPos("B2")), CValue("\"Another\\ \n \t \' \" \\\\ \a \b test\"")));

  assert(valueMatch(x1.getValue(CPos("C2")), CValue("Another\\ \n \t \'  hello \\\\ \a \b test")));


  // Display results or log them
  std::cout << "Save & load tests passed!" << std::endl;
};

void copyRect_tests() {
    CSpreadsheet ss,x1;
    std::ostringstream oss;
    std::istringstream iss;
    std::string data;

    // Set up initial cells
    assert(ss.setCell(CPos("A1"), "10"));
    assert(ss.setCell(CPos("A2"), "20"));
    assert(ss.setCell(CPos("A3"), "=A1+A2"));  // Formula should update when copied
    assert(ss.setCell(CPos("B1"), "30"));
    assert(ss.setCell(CPos("B2"), "=A2*2"));

    // Test 1: Copy single cell with value
    ss.copyRect(CPos("C1"), CPos("A1"), 1, 1);
    assert(valueMatch(ss.getValue(CPos("C1")), CValue(10.0)));

    // Test 2: Copy single cell with formula and check if the reference updates correctly
    ss.copyRect(CPos("C3"), CPos("A3"), 1, 1);
    assert(valueMatch(ss.getValue(CPos("C3")), CValue()));  // C3 should now be =C1+C2

    // Test 3: Copy a range of cells including formulas
    ss.copyRect(CPos("D1"), CPos("A1"), 2, 2);
    assert(valueMatch(ss.getValue(CPos("D1")), CValue(10.0)));  // Copy of A1
    assert(valueMatch(ss.getValue(CPos("D2")), CValue(20.0)));  // Copy of A2
    assert(valueMatch(ss.getValue(CPos("E1")), CValue(30.0)));  // Copy of B1
    assert(valueMatch(ss.getValue(CPos("E2")), CValue(40.0)));  // Should evaluate to 40 because E2 = D2*2

    // Test 4: Overlapping regions (source and destination overlap)
    ss.setCell(CPos("A4"), "100");
    ss.copyRect(CPos("A5"), CPos("A4"), 1, 1);
    assert(valueMatch(ss.getValue(CPos("A5")), CValue(100.0)));

    // Test 5: Copy to a larger range than the source covers
    ss.copyRect(CPos("C5"), CPos("A1"), 3, 3);  // Source is only 2x2
    assert(valueMatch(ss.getValue(CPos("C5")), CValue(10.0)));  // A1
    assert(valueMatch(ss.getValue(CPos("C6")), CValue(20.0)));  // A2
    assert(valueMatch(ss.getValue(CPos("D5")), CValue(30.0)));  // B1
    assert(valueMatch(ss.getValue(CPos("D6")), CValue(40.0)));  // B2
    // Cells without explicit source should be empty or default
    assert(valueMatch(ss.getValue(CPos("C7")), CValue(30.)));       // Beyond A3, should be empty
    assert(valueMatch(ss.getValue(CPos("D7")), CValue()));       // Beyond B2, should be empty

    // Test 6: Edge case - Copying an empty cell to a range
    ss.copyRect(CPos("F1"), CPos("Z99"), 2, 2);  // Assuming Z99 is empty
    assert(valueMatch(ss.getValue(CPos("F1")), CValue()));
    assert(valueMatch(ss.getValue(CPos("F2")), CValue()));
    assert(valueMatch(ss.getValue(CPos("G1")), CValue()));
    assert(valueMatch(ss.getValue(CPos("G2")), CValue()));

    assert(ss.setCell(CPos("A2"), "=\"N8+22\""));
    assert(valueMatch(ss.getValue(CPos("A2")), CValue("N8+22")));
    ss.copyRect(CPos("Z99"), CPos("A2"), 2, 2);  // Assuming Z99 is empty
    assert(valueMatch(ss.getValue(CPos("Z99")), CValue("N8+22")));

    assert(ss.setCell(CPos("A2"), "=\"N8+22\""));
    assert(ss.setCell(CPos("A3"), "=A2"));

    assert(valueMatch(ss.getValue(CPos("A2")), CValue("N8+22")));
    ss.copyRect(CPos("Z99"), CPos("A3"), 2, 2);  // Assuming Z99 is empty
    assert(valueMatch(ss.getValue(CPos("Z99")), CValue()));

    assert(valueMatch(ss.getValue(CPos("A2")), CValue("N8+22")));
    ss.copyRect(CPos("Z99"), CPos("A2"), 2, 2);  // Assuming Z99 is empty
    assert(valueMatch(ss.getValue(CPos("Z99")), CValue("N8+22")));


    assert(ss.setCell(CPos("A3"), "30"));
    assert(ss.setCell(CPos("F12"), "=A3"));
    assert(ss.setCell(CPos("C5"), "=10"));

  //  ss.copyRect(CPos("F12"), CPos("A3"), 1, 1);  // Assuming Z99 is empty
    assert(valueMatch(ss.getValue(CPos("F12")), CValue(30.)));
    ss.copyRect(CPos("G13"), CPos("F12"), 1, 1);  // Assuming Z99 is empty
    assert(valueMatch(ss.getValue(CPos("G13")), CValue()));
    ss.copyRect(CPos("H14"), CPos("G13"), 1, 1);  // Assuming Z99 is empty
    assert(valueMatch(ss.getValue(CPos("H14")), CValue(10.)));

      oss.str("");
    oss.clear();
    iss.clear();

  // Test 4: Save and Load with special characters
    CSpreadsheet s1;
    assert(s1.setCell(CPos("A1"), "1"));
    assert(s1.setCell(CPos("B1"), "2"));
    assert(s1.setCell(CPos("C1"), "3"));
    assert(s1.setCell(CPos("A2"), "4"));
    assert(s1.setCell(CPos("B2"), "5"));
    assert(s1.setCell(CPos("C2"), "6"));
    assert(s1.setCell(CPos("A3"), "7"));
    assert(s1.setCell(CPos("B3"), "8"));
    assert(s1.setCell(CPos("C3"), "=A1+B1+C1+A2+B2+C2+A3+B3"));
    assert(valueMatch(s1.getValue(CPos("C3")), 36.0));
    s1.copyRect(CPos("B2"), CPos("A1"), 3, 3);

    assert(s1.setCell(CPos("A1"), "= A2 + A3"));
    assert(s1.setCell(CPos("A2"), "= A4"));
    assert(s1.setCell(CPos("A3"), "= A4"));
    assert(s1.setCell(CPos("A4"), "100"));

    assert(valueMatch(s1.getValue(CPos("A1")), 200.));
    assert(valueMatch(s1.getValue(CPos("A2")), 100.));
    assert(valueMatch(s1.getValue(CPos("A3")), 100.));
    assert(valueMatch(s1.getValue(CPos("A4")), 100.));



    std::cout << "CopyRect tests passed." << std::endl;
}


int main ()
{
  //runTests();
  save_load_tests();
  basic_tests();
  copyRect_tests();
  CSpreadsheet x0, x1;
  std::ostringstream oss;
  std::istringstream iss;
  std::string data;
  assert ( x0 . setCell ( CPos ( "A1" ), "10" ) );
  assert ( x0 . setCell ( CPos ( "A2" ), "20.5" ) );
  assert ( x0 . setCell ( CPos ( "A3" ), "3e1" ) );
  assert ( x0 . setCell ( CPos ( "A4" ), "=40" ) );
  assert ( x0 . setCell ( CPos ( "A5" ), "=5e+1" ) );
  assert ( x0 . setCell ( CPos ( "A6" ), "raw text with any characters, including a quote \" or a newline\n" ) );
  assert ( x0 . setCell ( CPos ( "A7" ), "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\"" ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A1" ) ), CValue ( 10.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A2" ) ), CValue ( 20.5 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A3" ) ), CValue ( 30.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A4" ) ), CValue ( 40.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A5" ) ), CValue ( 50.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A6" ) ), CValue ( "raw text with any characters, including a quote \" or a newline\n" ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A7" ) ), CValue ( "quoted string, quotes must be doubled: \". Moreover, backslashes are needed for C++." ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A8" ) ), CValue() ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "AAAA9999" ) ), CValue() ) );
  assert ( x0 . setCell ( CPos ( "B1" ), "=A1+A2*A3" ) );
  assert ( x0 . setCell ( CPos ( "B2" ), "= -A1 ^ 2 - A2 / 2   " ) );
  assert ( x0 . setCell ( CPos ( "B3" ), "= 2 ^ $A$1" ) );
  assert ( x0 . setCell ( CPos ( "B4" ), "=($A1+A$2)^2" ) );
  assert ( x0 . setCell ( CPos ( "B5" ), "=B1+B2+B3+B4" ) );
  assert ( x0 . setCell ( CPos ( "B6" ), "=B1+B2+B3+B4+B5" ) );  
  assert ( valueMatch ( x0 . getValue ( CPos ( "B1" ) ), CValue ( 625.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B2" ) ), CValue ( -110.25 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B3" ) ), CValue ( 1024.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B4" ) ), CValue ( 930.25 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B5" ) ), CValue ( 2469.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B6" ) ), CValue ( 4938.0 ) ) );
  assert ( x0 . setCell ( CPos ( "A1" ), "12" ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B1" ) ), CValue ( 627.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B2" ) ), CValue ( -154.25 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B4" ) ), CValue ( 1056.25 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B5" ) ), CValue ( 5625.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B6" ) ), CValue ( 11250.0 ) ) );
  x1 = x0;
  assert ( x0 . setCell ( CPos ( "A2" ), "100" ) );
  assert ( x1 . setCell ( CPos ( "A2" ), "=A3+A5+A4" ) );//120

  assert ( valueMatch ( x1 . getValue ( CPos ( "A2" ) ), CValue ( 120.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "A1" ) ), CValue ( 12.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "A3" ) ), CValue ( 30.0 ) ) );

  
  assert ( valueMatch ( x0 . getValue ( CPos ( "B1" ) ), CValue ( 3012.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B2" ) ), CValue ( -194.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B4" ) ), CValue ( 12544.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B5" ) ), CValue ( 19458.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "B6" ) ), CValue ( 38916.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B1" ) ), CValue ( 3612.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B2" ) ), CValue ( -204.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B4" ) ), CValue ( 17424.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B5" ) ), CValue ( 24928.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B6" ) ), CValue ( 49856.0 ) ) );
  
  oss . clear ();
  oss . str ( "" );
  assert ( x0 . save ( oss ) );

  data = oss . str ();
  iss . clear ();
  iss . str ( data );
  assert ( x1 . load ( iss ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B1" ) ), CValue ( 3012.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B2" ) ), CValue ( -194.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B4" ) ), CValue ( 12544.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B5" ) ), CValue ( 19458.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B6" ) ), CValue ( 38916.0 ) ) );
  assert ( x0 . setCell ( CPos ( "A3" ), "4e1" ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B1" ) ), CValue ( 3012.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B2" ) ), CValue ( -194.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B4" ) ), CValue ( 12544.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B5" ) ), CValue ( 19458.0 ) ) );
  assert ( valueMatch ( x1 . getValue ( CPos ( "B6" ) ), CValue ( 38916.0 ) ) );
  oss . clear ();
  oss . str ( "" );

  
  assert ( x0 . save ( oss ) );
  data = oss . str ();
  for ( size_t i = 0; i < std::min<size_t> ( data . length (), 10 ); i ++ )
    data[i] ^=0x5a;
  iss . clear ();
  iss . str ( data );
  assert ( ! x1 . load ( iss ) );
  assert ( x0 . setCell ( CPos ( "D0" ), "10" ) );
  assert ( x0 . setCell ( CPos ( "D1" ), "20" ) );
  assert ( x0 . setCell ( CPos ( "D2" ), "30" ) );
  assert ( x0 . setCell ( CPos ( "D3" ), "40" ) );
  assert ( x0 . setCell ( CPos ( "D4" ), "50" ) );
  assert ( x0 . setCell ( CPos ( "E0" ), "60" ) );
  assert ( x0 . setCell ( CPos ( "E1" ), "70" ) );
  assert ( x0 . setCell ( CPos ( "E2" ), "80" ) );
  assert ( x0 . setCell ( CPos ( "E3" ), "90" ) );
  assert ( x0 . setCell ( CPos ( "E4" ), "100" ) );
  assert ( x0 . setCell ( CPos ( "F10" ), "=D0+5" ) );
  assert ( x0 . setCell ( CPos ( "F11" ), "=$D0+5" ) );
  assert ( x0 . setCell ( CPos ( "F12" ), "=D$0+5" ) );
  assert ( x0 . setCell ( CPos ( "F13" ), "=$D$0+5" ) );
  x0 . copyRect ( CPos ( "G11" ), CPos ( "F10" ), 1, 4 );

  assert ( valueMatch ( x0 . getValue ( CPos ( "F10" ) ), CValue ( 15.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "F11" ) ), CValue ( 15.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "F12" ) ), CValue ( 15.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "F13" ) ), CValue ( 15.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "F14" ) ), CValue() ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "G10" ) ), CValue() ) );

  assert ( valueMatch ( x0 . getValue ( CPos ( "G11" ) ), CValue ( 75.0 ) ) ); //copy rectanlge implemtaciu dokončiť
  assert ( valueMatch ( x0 . getValue ( CPos ( "G12" ) ), CValue ( 25.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "G13" ) ), CValue ( 65.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "G14" ) ), CValue ( 15.0 ) ) );
  x0 . copyRect ( CPos ( "G11" ), CPos ( "F10" ), 2, 4 );
  assert ( valueMatch ( x0 . getValue ( CPos ( "F10" ) ), CValue ( 15.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "F11" ) ), CValue ( 15.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "F12" ) ), CValue ( 15.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "F13" ) ), CValue ( 15.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "F14" ) ), CValue() ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "G10" ) ), CValue() ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "G11" ) ), CValue ( 75.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "G12" ) ), CValue ( 25.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "G13" ) ), CValue ( 65.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "G14" ) ), CValue ( 15.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "H10" ) ), CValue() ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "H11" ) ), CValue() ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "H12" ) ), CValue() ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "H13" ) ), CValue ( 35.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "H14" ) ), CValue() ) );
  assert ( x0 . setCell ( CPos ( "F0" ), "-27" ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "H14" ) ), CValue ( -22.0 ) ) );
  x0 . copyRect ( CPos ( "H12" ), CPos ( "H13" ), 1, 2 );
  assert ( valueMatch ( x0 . getValue ( CPos ( "H12" ) ), CValue ( 25.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "H13" ) ), CValue ( -22.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "H14" ) ), CValue ( -22.0 ) ) ); 

  std::cout << "All tests passed!" << std::endl;

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
