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


class Expr
{
public:
  virtual CValue eval(CSpreadsheet *spreadsheat) const = 0;
  virtual int getType() const = 0;
  virtual ~Expr() = default; // Ensure proper cleanup
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
    Sum(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}//return could be different
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
        else if(std::holds_alternative<std::string>(rVal) && std::holds_alternative<double>(rVal))
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
      return std::monostate(); // or 0?
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
    else return std::monostate(); // or 0?
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
      return std::monostate(); // or 0?
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
      return std::monostate(); // or 0?
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
      return std::monostate(); // or 0?
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
      return std::monostate(); // or 0?
    return (std::get<double>(lVal) >= std::get<double>(rVal))? CValue(1.) :CValue(0.) ;
  }
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class expBuilder: public CExprBuilder{
  public:
    expBuilder(){};

    void opAdd() override
    {
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      exprStack.push(std::make_shared<Sum>(left, right));
      return;
    };
    void opSub() override
    {
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      exprStack.push(std::make_shared<Subtraction>(left, right));
      return;
    };
    void opMul() override
    {
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      exprStack.push(std::make_shared<Multiplication>(left, right));
      return;
    };
    void opDiv() override
    {
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();  
      exprStack.push(std::make_shared<Division>(left, right));
      return;
    };
    void opPow() override
    {
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
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
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      exprStack.push(std::make_shared<Equal>(left, right));
      return;
    };
    void opNe() override { 
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      exprStack.push(std::make_shared<NotEqual>(left, right));
      return; 
    };
    void opLt() override { 
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      exprStack.push(std::make_shared<LowerThen>(left, right));
      return; 
    };
    void opLe() override { 
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      exprStack.push(std::make_shared<LowerEq>(left, right));
      return; 
    };
    void opGt() override { 
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      exprStack.push(std::make_shared<GreaterThen>(left, right));
      return;
     };
    void opGe() override {
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      exprStack.push(std::make_shared<GreaterEqual>(left, right));
      return; 
    };

    void valNumber(double val) override { 
      exprStack.push(std::make_shared<Numeric>(val));
      return; 
    };
    void valString(std::string val) override { 
      exprStack.push(std::make_shared<Text>(val));
      return;
    };
    void valReference(std::string val) override; // @note is on the bottom of the code, due to my incopetence arrange code differently

    void valRange(std::string val) override { 

      return; 
    };
    void funcCall(std::string fnName, int paramCount) override { return; }; //Not neccessary

    ExprPtr getResult() const {
        if (exprStack.empty()) {
          throw std::runtime_error("Error: Attempted to evaluate an empty expression.");
        }
        return exprStack.top();
    }

    std::stack<ExprPtr> exprStack;    
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class CCell{
  public:
    enum type{ NUMERIC, TEXT, FORMULA};
    CCell();
    CCell(const std::string& value);

    void Set(const std::string& text);
    void Clear();
    CValue getValue(CSpreadsheet * spreadsheet)const;
    CCell clone() const;
    void parseDependencies();
    type get_type() ;
    std::string getContent() const;
    CCell& operator=(const CCell& other);
    expBuilder formula; //->this will be parsed
    void updateFormulaReferences(int deltaCol, int deltaRow);
    std::string content_editor(int deltaColum, int deltaRow);
  private:
    type content_type;
    CValue content;
 //   std::stack<ExprPtr> formulaStack;
    std::string original_content;
};
std::string CCell::getContent()const{
  return original_content;
};
CCell & CCell::operator=(const CCell& other){
  this->content = other.content;
  this->original_content = other.original_content;
  this->content_type = other.content_type;
  parseExpression(original_content,formula);

  return *this;
};
CCell CCell::clone() const{
  CCell newCopy(this->getContent());
  return newCopy;
};

std::string updateFormula(std::string formula, int deltaRow, int deltaCol) {
    std::string result;
    int i = 0;
    int len = formula.length();

    while (i < len) {
        if (std::isalpha(formula[i])) {  // Start of a potential cell reference
            bool absCol = false, absRow = false;
            std::string colPart, rowPart;

            if (i > 0 && formula[i-1] == '$') {
                absCol = true; // There's a $ before the column part
                result.pop_back(); // Remove the '$' from result as it will be added later
            }

            // Collect column part
            while (i < len && std::isalpha(formula[i])) {
                colPart += formula[i++];
            }

            // Check for '$' before the row part
            if (i < len && formula[i] == '$') {
                absRow = true;
                i++; // Skip the '$'
            }

            // Collect row part
            while (i < len && std::isdigit(formula[i])) {
                rowPart += formula[i++];
            }

            // Process the collected cell reference
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
    return CValue(); // Return an empty CValue
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
    friend std::pair<int,int> CPos_parser(std::string_view str);
    bool operator<( const CPos & other) const;
    CPos offset(int dx, int dy) const ;


    void print() const;
    std::string getCode() const;
    std::pair<unsigned int, unsigned int> getRaC() const;
    std::pair<int, int> CPos_parser(std::string_view str) ;
    CPos copy();
  //private:
    mutable unsigned int row;
    mutable unsigned int column;
    bool relative_column, relative_row;
    std::string code;

};

std::pair<unsigned int, unsigned int> CPos::getRaC() const {
    return {row, column};
};

void CPos::print() const{
  std::cout<<code<<"-ROW: "<<row<<" COLUMN: "<<column<<std::endl;
};

std::string CPos::getCode() const{
  return code;
}

std::pair<int, int> CPos::CPos_parser(std::string_view str) {
    std::pair<int, int> position;
    int column = 0;
    int row = 0;
    size_t index = 0;
    bool absolute_column = false;
    bool absolute_row = false;

    bool column_exists = false;
    bool row_exists = false;


    
    // Check for absolute column reference
    if (!str.empty() && str[index] == '$') {
        absolute_column = true;
        index++;
    }

    // Process column letters
    while (index < str.size() && std::isalpha(str[index])) {
        column = column * 26 + (std::tolower(str[index]) - 'a' + 1);
        ++index;
        column_exists = true;
    }

    // Check for absolute row reference
    if (index < str.size() && str[index] == '$' && column_exists) {
        absolute_row = true;
        index++;
    }

    // Process row number
  if (index < str.size() && column_exists) {
        size_t processed_length = 0;
        std::string row_part = std::string(str.substr(index));
        row = std::stoi(row_part, &processed_length);
        // Ensure all characters in the row part were consumed by std::stoi
        if (processed_length != row_part.length()) {
            throw std::invalid_argument("Invalid row format in input: " + std::string(str));
        }
        row_exists = true;
    }

    if(row_exists && column_exists)
    {
        position.first = row;
        position.second = column;
        relative_column = !absolute_column;
        relative_row = !absolute_row;
        return position;
    }
    else throw std::invalid_argument("");
}

std::string back_to_code(unsigned int row, unsigned int column) {
    std::string columnLabel;

    // Column number to column label conversion (base-26)
    while (column > 0) {
        // Adjust for 1-based index
        int remainder = (column - 1) % 26;
        char letter = 'A' + remainder;
        columnLabel = letter + columnLabel;
        column = (column - 1) / 26;
    }
    // Convert row number to string
    std::string rowLabel = std::to_string(row);

    // Combine column and row labels to form the cell code
    return columnLabel + rowLabel;
}

CPos::CPos(std::string_view str) {
  relative_column = true; 
  relative_row = true;
  std::pair<int,int> position = CPos_parser(str);
  if(!relative_column || !relative_row) std::cout<<"RELATIVE\n";
  code = str;
  row = position.first; // Initialize row to default value
  column = position.second; // Initialize column to default value
}
bool CPos::operator<( const CPos & other ) const {
  if(this->row == other.row )
    return this->column < other.column;
  return this->row < other.row;
};

CPos CPos::offset(int dx, int dy) const {
    // Generate a new code for the new position by adding offsets to the current position
    return CPos(back_to_code(row + dy, column + dx));
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class CSpreadsheet
{
public:
  static unsigned capabilities()
  {
    return 0;
    return SPREADSHEET_CYCLIC_DEPS | SPREADSHEET_FUNCTIONS | SPREADSHEET_FILE_IO | SPREADSHEET_SPEED;
  }
  CSpreadsheet(){};
  bool load(std::istream &is);
  bool save(std::ostream &os) const;
  bool setCell(CPos pos, std::string contents);
  CValue getValue(CPos pos);
  void copyRect(CPos dst, CPos src, int w = 1, int h = 1);
//private:
  std::map<CPos, CCell > page; /// @note Stores our cell position and cells with relation to the given cell
};

CValue CSpreadsheet::getValue(CPos pos)
{
  if(page.find(pos)!= page.end())
    return page[pos].getValue(this) ; 
  return CValue();
};

bool CSpreadsheet::setCell(CPos pos, std::string contents)
{
  CCell tmp;
  
  try{
    tmp = CCell(contents);
  }
  catch(...){
    return false;
  }
  page[pos] = {CCell(contents)};
  return true;
};

std::string escapeString(const std::string& input)  {
        std::string output;
        for (char c : input) {
            switch (c) {
                case '\\': output += "\\\\"; break;
                case '"': output += "\\\""; break;
                case '\n': output += "\\n"; break;
                default: output += c; break;
            }
        }
        return output;
    }

    std::string unescapeString(const std::string& input)  {
        std::string output;
        bool escape = false;
        for (char c : input) {
            if (escape) {
                switch (c) {
                    case '\\': output += '\\'; break;
                    case '"': output += '"'; break;
                    case 'n': output += '\n'; break;
                    default: output += c; break; // Unhandled escape sequence
                }
                escape = false;
            } else if (c == '\\') {
                escape = true;
            } else {
                output += c;
            }
        }
        return output;
    }


bool CSpreadsheet::save(std::ostream &os) const {
    for (const auto &entry : page) {
        const CPos &pos = entry.first;
        const CCell &cell = entry.second;

        std::string encodedContent = cell.getContent();
        // Escape special characters in the content
        std::string escapedContent;
        for (char c : encodedContent) {
            switch (c) {
                case '\n': escapedContent += "\\n"; break;
                case '\r': escapedContent += "\\r"; break;
                case '\\': escapedContent += "\\\\"; break;
                case '\031': escapedContent += "\\x1f"; break; // ASCII Unit Separator
                default: escapedContent += c; break;
            }
        }

        // Format the output to ensure correct parsing when loading
        if (!(os << "BUNK" << pos.getCode() << "CONT" << escapedContent << char(31))) {
            std::cerr << "Error writing to output stream." << std::endl;
            return false;
        }
    }
    return true;
}
bool CSpreadsheet::load(std::istream &is) {
    std::string line;
    while (getline(is, line, char(31))) { // Using ASCII 31 as the delimiter
        if (line.empty())
            continue; // Skip empty lines

        size_t bunkPos = line.find("BUNK");
        size_t contPos = line.find("CONT");
        if (bunkPos == std::string::npos || contPos == std::string::npos || bunkPos > contPos) {
            std::cerr << "Error: Invalid line format. Line: " << line << std::endl;
            return false;
        }

        std::string cellPos = line.substr(bunkPos + 4, contPos - (bunkPos + 4));
        std::string contents = line.substr(contPos + 4);
        std::string decodedContents;
        decodedContents.reserve(contents.size()); // Reserve to avoid multiple reallocations

        // Decode escaped characters
        for (size_t i = 0; i < contents.length(); ++i) {
            if (contents[i] == '\\' && i + 1 < contents.length()) {
                switch (contents[i + 1]) {
                    case 'n': decodedContents += '\n'; ++i; break;
                    case 'r': decodedContents += '\r'; ++i; break;
                    case '\\': decodedContents += '\\'; ++i; break;
                    case 'x':
                        if (i + 3 < contents.length() && contents[i + 2] == '1' && contents[i + 3] == 'f') {
                            decodedContents += char(31);
                            i += 3;
                        }
                        break;
                    default: decodedContents += contents[i + 1]; ++i; break; // Assume it was a mistake to escape
                }
            } else {
                decodedContents += contents[i];
            }
        }

        CPos posObject(cellPos);
        if (!setCell(posObject, decodedContents)) {
            std::cerr << "Error setting cell at position " << cellPos << " with content " << decodedContents << std::endl;
            return false;
        }
    }
    return true;
}



void CSpreadsheet::copyRect(CPos dst, CPos src, int w, int h) {
    int deltaRow = dst.getRaC().first - src.getRaC().first;
    int deltaCol = dst.getRaC().second - src.getRaC().second;

    // Loop through each cell in the rectangular area
    for (int row = 0; row < h; ++row) {
        for (int col = 0; col < w; ++col) {
            CPos currentSrc = src.offset(col, row);
            CPos currentDst = dst.offset(col, row);

            // Find the source cell in the map
              if(page.find(currentSrc)!=page.end()){
                // Clone the source cell
                if(page[currentSrc].get_type()==CCell::type::FORMULA){
                      CCell srcCell(updateFormula(page[currentSrc].getContent(),deltaRow,deltaCol));
                     page[currentDst] = srcCell;
                }else if((page[currentSrc].get_type()==CCell::type::TEXT )
                || (page[currentSrc].get_type()==CCell::type::NUMERIC)){
                     CCell srcCell(updateFormula(page[currentSrc].getContent(),deltaRow,deltaCol));
                     page[currentDst] = srcCell;
                }
              else if(page.find(currentDst)!=page.end()  ){
                page.erase(currentDst);
              }
                // If it's a formula, update its references
                // Set the cloned and possibly modified cell to the new position
            }
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class Reference : public Expr {
/* private:
    CPos pos; */

public:
    CPos pos;

    Reference(CPos m_pos)
        : pos(m_pos){}
    int getType()const override{ return 1; }

    //CPos & getPos(){};

/*     ExprPtr clone(CSpreadsheet* spreadsheet ) const override {
        return std::make_shared<Reference>(pos);
} */

    CValue eval(CSpreadsheet* spreadsheet) const override {
        if (spreadsheet->page.find(pos) == spreadsheet->page.end()) {
            return std::monostate();
        }
        return spreadsheet->getValue(pos);
    }
};

void expBuilder::valReference(std::string val) {
    CPos pos(val);
    exprStack.push(std::make_shared<Reference>(pos));
    std::cout << "Pushed Reference to stack with initial position: [" << pos.row << ", " << pos.column << "]" << std::endl;
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

void setCellRange(const std::vector<std::string>& cells, const std::vector<std::string>& values, CSpreadsheet& spreadsheet){
    for(int j = 0; j < cells.size(); j++){
        spreadsheet.setCell(CPos(cells[j]), values[j]);
    }
}

void copyRectRange(const std::vector<std::pair<std::string, std::string >>& fromTo, int h, int w, CSpreadsheet& spreadsheet){
    for(auto p: fromTo){
        spreadsheet.copyRect( CPos(p.first), CPos(p.second), w, h);
    }
}

void saveLoad(CSpreadsheet& spreadsheet){
    std::ostringstream oss;
    std::istringstream iss;
    std::string data;

    oss.clear();
    oss.str("");
    assert(spreadsheet.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert(spreadsheet.load(iss));
}

// Function to add an edge to the graph
void addEdge(std::vector<std::vector<int>>& adj, int u, int v) {
    adj[u].push_back(v);
}


bool dfs(std::vector<std::vector<int>>& adj, std::vector<int>& color, int v, int parent) {
    if(color[v] == 1){
        color[v] = 2;
        return true;
    }

    if(color[v]== 2) return false;

    color[v] = 1;
    for (int u : adj[v]) {
        if (dfs(adj, color, u, v)) return true;
        color[u]=2;
    }
    return false;
}

bool hasCycle(std::vector<std::vector<int>>& adj, int from=0, int to=-1) {
    int V= adj.size();
    if(to==-1)to=adj.size();

    std::vector<int>color(V, 0);
    for (int v = from; v < to; ++v) {
        if (color[v] == 0) {
            if (dfs(adj, color, v,-1))
                return true;
        }
    }
    return false;
}

bool generateRandomGraph(std::vector<std::vector<int>>& adj, int V, int E) {
    std::unordered_set<int> edgeSet;
    while (E > 0) {
        int u = rand() % V;
        int v = rand() % V;
        if (u != v && edgeSet.find(u * V + v) == edgeSet.end()) {
            addEdge(adj, u, v);
            edgeSet.insert(u * V + v);
            if(rand()%10==0){
                addEdge(adj, v, u);
            }
            --E;
        }
    }
    return hasCycle(adj);
}

void generateTableWithCycles() {
    CSpreadsheet spreadsheet;
    int V = rand() %100+10;
    int E = V+rand()%10;
    std::vector<std::vector<int>> adj(V);
    bool hc = generateRandomGraph(adj, V,E);

    // make a labels to all nodes in a graph.
    std::unordered_map<int, std::string> nodeLabels;

    for(int j =0; j< V; j++){
        nodeLabels.insert({j, "A"+std::to_string(j)});
    }

    // Build a table.
    for(int j =0; j < V; j++){
        // generate expression.
        std::string expr= "=";
        for(auto node: adj[j]){
            expr+= "-"+nodeLabels[node];
        }
        expr+="-"+std::to_string(420);
        // Add to a table.
        spreadsheet.setCell(CPos(nodeLabels[j]),expr);
    }

    for(int j =0; j< V; j++){
        if(spreadsheet.getValue(CPos(nodeLabels[j])) == CValue()){
            assert(hasCycle(adj, j,j+1) && adj[j].size()!=0);
        }
        else {
            auto value = spreadsheet.getValue(CPos(nodeLabels[j]));
            if(std::holds_alternative<double>(value)){
                int v = std::stoi(std::to_string(std::abs(std::get<double>(value)))) %420;
                assert(v==0);
            }
        }
    }
}

#define SIMPLE_TESTS // Simple tests - getVal, save & load - no file corruption.
//#define CYCLIC_DEPS_TESTS // Cycle generation, if time > 2s -> exception
//#define FILE_IO_TESTS // file corruption tests.
#include <future>
#include <chrono>

void runTests(){
    srand(time(nullptr));

#ifdef SIMPLE_TESTS
    CSpreadsheet preTests;
    preTests.setCell(CPos("d1"), "=12+10 + $E$1");
    assert(valueMatch(preTests.getValue(CPos("d1")), CValue()));
    preTests.setCell(CPos("e1"), "=-12-10");
    preTests.setCell(CPos("f1"), "=$d1 + E$1");
    assert(valueMatch(preTests.getValue(CPos("d1")), CValue(0.)));
    assert(valueMatch(preTests.getValue(CPos("f1")), CValue(-22.)));
    setCellRange({"d2", "d3", "d4", "d5", "g1", "h1", "h1"}, {"1","2", "3", "4", "1", "=2", "=3", "=4"}, preTests);
    copyRectRange({{"g2", "f1"}, {"g3", "g2"}, {"g4", "f1"}, {"g5", "f1"}}, 1, 1, preTests);

    assert(valueMatch(preTests.getValue(CPos("g2")), CValue(-21.)));
    assert(valueMatch(preTests.getValue(CPos("g3")), CValue(-20.)));
    assert(valueMatch(preTests.getValue(CPos("g4")), CValue(-19.)));
    assert(valueMatch(preTests.getValue(CPos("g5")), CValue(-18.)));

    copyRectRange({{"h2", "g2"}, {"i2", "h2"}, {"h2", "h2"}}, 4,1, preTests);

    assert(valueMatch(preTests.getValue(CPos("h2")), CValue(2.)));
    assert(valueMatch(preTests.getValue(CPos("h3")), CValue(3.)));
    assert(valueMatch(preTests.getValue(CPos("h4")), CValue(4.)));
    assert(valueMatch(preTests.getValue(CPos("h5")), CValue(5.)));

    assert(valueMatch(preTests.getValue(CPos("i2")), CValue(4.)));
    assert(valueMatch(preTests.getValue(CPos("i3")), CValue(5.)));
    assert(valueMatch(preTests.getValue(CPos("i4")), CValue(6.)));
    assert(valueMatch(preTests.getValue(CPos("i5")), CValue(7.)));

    saveLoad(preTests);

    assert(valueMatch(preTests.getValue(CPos("h2")), CValue(2.)));
    assert(valueMatch(preTests.getValue(CPos("h3")), CValue(3.)));
    assert(valueMatch(preTests.getValue(CPos("h4")), CValue(4.)));
    assert(valueMatch(preTests.getValue(CPos("h5")), CValue(5.)));

    assert(valueMatch(preTests.getValue(CPos("i2")), CValue(4.)));
    assert(valueMatch(preTests.getValue(CPos("i3")), CValue(5.)));
    assert(valueMatch(preTests.getValue(CPos("i4")), CValue(6.)));
    assert(valueMatch(preTests.getValue(CPos("i5")), CValue(7.)));

    assert(valueMatch(preTests.getValue(CPos("f1")), CValue(-22.)));

    preTests.copyRect(CPos("h3"), CPos("g2"), 4, 4);

    assert(valueMatch(preTests.getValue(CPos("h6")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("i6")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("h6")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k6")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k3")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k4")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k5")), CValue()));

    saveLoad(preTests);

    assert(valueMatch(preTests.getValue(CPos("h2")), CValue(2.)));
    assert(valueMatch(preTests.getValue(CPos("h3")), CValue(3.)));
    assert(valueMatch(preTests.getValue(CPos("h4")), CValue(4.)));
    assert(valueMatch(preTests.getValue(CPos("h5")), CValue(5.)));

    assert(valueMatch(preTests.getValue(CPos("i2")), CValue(4.)));
    assert(valueMatch(preTests.getValue(CPos("i3")), CValue(5.)));
    assert(valueMatch(preTests.getValue(CPos("i4")), CValue(6.)));
    assert(valueMatch(preTests.getValue(CPos("i5")), CValue(7.)));

    assert(valueMatch(preTests.getValue(CPos("f1")), CValue(-22.)));

    assert(valueMatch(preTests.getValue(CPos("h6")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("i6")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("h6")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k6")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k3")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k4")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k5")), CValue()));

    preTests.setCell(CPos("d6"), "0");

    assert(valueMatch(preTests.getValue(CPos("h6")), CValue(1.)));
    assert(valueMatch(preTests.getValue(CPos("i6")), CValue(3.)));
    assert(valueMatch(preTests.getValue(CPos("h6")), CValue(1.)));
    assert(valueMatch(preTests.getValue(CPos("k6")), CValue()));

    preTests.setCell(CPos("d10"), "=$d11+$E$10");
    assert(valueMatch(preTests.getValue(CPos("d10")), CValue()));

    preTests.setCell(CPos("d11"), "=1");
    preTests.setCell(CPos("e10"), "=10");
    assert(valueMatch(preTests.getValue(CPos("d10")), CValue(11.)));

    copyRectRange({{"i2", "d10"}, {"i3", "i2"}, {"i4", "d10"}, {"i5", "d10"}}, 1,1, preTests);

    assert(valueMatch(preTests.getValue(CPos("i2")), CValue(12.)));
    assert(valueMatch(preTests.getValue(CPos("i3")), CValue(13.)));
    assert(valueMatch(preTests.getValue(CPos("i4")), CValue(14.)));
    assert(valueMatch(preTests.getValue(CPos("i5")), CValue(10.)));

    saveLoad(preTests);

    assert(valueMatch(preTests.getValue(CPos("d10")), CValue(11.)));

    assert(valueMatch(preTests.getValue(CPos("i2")), CValue(12.)));
    assert(valueMatch(preTests.getValue(CPos("i3")), CValue(13.)));
    assert(valueMatch(preTests.getValue(CPos("i4")), CValue(14.)));
    assert(valueMatch(preTests.getValue(CPos("i5")), CValue(10.)));

    assert(valueMatch(preTests.getValue(CPos("h2")), CValue(2.)));
    assert(valueMatch(preTests.getValue(CPos("h3")), CValue(3.)));
    assert(valueMatch(preTests.getValue(CPos("h4")), CValue(4.)));
    assert(valueMatch(preTests.getValue(CPos("h5")), CValue(5.)));

    assert(valueMatch(preTests.getValue(CPos("f1")), CValue(-22.)));

    assert(valueMatch(preTests.getValue(CPos("h6")), CValue(1.)));
    assert(valueMatch(preTests.getValue(CPos("i6")), CValue(3.)));
    assert(valueMatch(preTests.getValue(CPos("h6")), CValue(1.)));
    assert(valueMatch(preTests.getValue(CPos("k6")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k3")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k4")), CValue()));
    assert(valueMatch(preTests.getValue(CPos("k5")), CValue()));
    std::cout<<"SIMPLE_TESTS PASSED\n";
#endif

#ifdef CYCLIC_DEPS_TESTS
    int TC = 150;
    while(TC--){
        std::future<void> future = std::async(std::launch::async, generateTableWithCycles);
        if (future.wait_for(std::chrono::seconds(2)) == std::future_status::timeout) {
            throw std::runtime_error("Cycle detection went wrong!");
        }
    }

    std::cout<<"CYCLIC_DEPS_TESTS PASSED\n";
#endif

#ifdef FILE_IO_TESTS

    CSpreadsheet fileIo;
    setCellRange({"A1", "B1","B3", "A8", "ZZZ8", "AAA69", "ZZZ1", "AHOJ1", "Never1", "Gonna1", "Give1", "You1", "Up1"}, {"10","=10+50", "A1+B1", "B3+A8", "5", "10", "Lez", "1000", "Never", "Gonna", "Let", "U", "Down"}, fileIo);

    std::ostringstream oss;
    std::istringstream iss;
    std::string data;

    saveLoad(fileIo);

    oss.clear();
    oss.str("");
    assert(fileIo.save(oss));
    data = oss.str();
    // swap 2 characters.
    for(int j=0; j< data.length(); j++){
        for(int i=j+1; i< data.length(); i++){
            std::string copy= data;
            std::swap(copy[j], copy[i]);
            iss.clear();
            iss.str(copy);
            assert(fileIo.load(iss) == (copy==data));
        }
    }

    // erase 1 char randomly
    for(int j=0; j< 50; j++){
        oss.clear();
        oss.str("");
        assert(fileIo.save(oss));
        data = oss.str();
        data.erase(rand()%data.length(),1);
        iss.clear();ą
        iss.str(data);
        assert(!fileIo.load(iss));
    }


    CSpreadsheet copied = fileIo;
    setCellRange({"A1", "B1","B3", "A8", "ZZZ8", "AAA69", "ZZZ1", "AHOJ1", "Never1", "Gonna1", "Give1", "You1", "Up1"}, {"0","0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"}, fileIo);

    CSpreadsheet toLoad;
    oss.clear();
    oss.str("");
    assert(copied.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert(toLoad.load(iss));

    assert(valueMatch(toLoad.getValue(CPos("Never1")), CValue("Never")));
    assert(valueMatch(fileIo.getValue(CPos("A1")), CValue(0.)));

    std::cout << "FILE_IO_TESTS PASSED\n";
#endif
}




//==============================================================================================================================


void my_tests(){
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

  std::cout<<"========================== PASS 1 ==========================\n";

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

  //assert ( valueMatch ( x0 . getValue ( CPos ( "B1" ) ), CValue ( -40.0 ) ) );





/*   assert ( x0 . setCell ( CPos ( "A11" ), "=A9+A10" ) );
  assert ( x0 . setCell ( CPos ( "A12" ), "= A9 - A10" ) );
  assert ( x0 . setCell ( CPos ( "A13" ), "= A9 * A10" ) );
  assert ( x0 . setCell ( CPos ( "A14" ), "= A9 / A10" ) );
  assert ( x0 . setCell ( CPos ( "A15" ), "= A10 ^ B9" ) );

  assert ( valueMatch ( x0 . getValue ( CPos ( "A11" ) ), CValue ( 45.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A12" ) ), CValue ( 35.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A13" ) ), CValue ( 200.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A14" ) ), CValue ( 8.0 ) ) );
  assert ( valueMatch ( x0 . getValue ( CPos ( "A15" ) ), CValue ( 25.0 ) ) );
 */

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


/*   assert ( valueMatch ( x2 . getValue ( CPos ( "F12" ) ), CValue ( 1. ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "F13" ) ), CValue ( 1. ) ) );
  assert ( valueMatch ( x2 . getValue ( CPos ( "F14" ) ), CValue ( 1. ) ) ); */

  assert ( x1 . setCell ( CPos ( "A0" ), "\n\n\n\n\\\\" ) );
  assert ( x1 . setCell ( CPos ( "A1" ), "\n \n guten tag \\n" ) );


  
  oss . clear ();
  oss . str ( "" );
  assert ( x1 . save ( oss ) );
  data = oss . str ();
  iss . clear ();
  iss . str ( data );
  assert ( x3 . load ( iss ) );
  std::cout<< x3.page[CPos("A0")].getContent()<<std::endl;
  std::cout<< x3.page[CPos("A1")].getContent()<<std::endl;

  //assert ( x1 . setCell ( CPos ( "A2" ), "Hello \n Hi \n \\n" ) );



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


  std::cout<<"==========================PASS 2==========================\n";
  assert ( x1 . setCell ( CPos ( "A0" ), "22" ) );
  assert ( x1 . setCell ( CPos ( "A1" ), "99" ) );
  x1 . copyRect ( CPos ( "A0" ), CPos ( "A1" ), 1, 1 );
  assert ( valueMatch ( x1 . getValue ( CPos ( "A0" ) ), x1 . getValue ( CPos ( "A1" ) ) ) );

  std::cout<<"\n BASIC TESTS YEPEEE :3\n"<<std::endl;
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
  std::cout<<x1.page[CPos("B1")].getContent();
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
  x0.setCell(CPos("C2"), "=\"Another\\ \n \t \' \" hello \\\\ \a \b test\"");

  assert(x0.save(oss));
  data = oss.str();
  iss.str(data);
  assert(x1.load(iss));
  assert(valueMatch(x1.getValue(CPos("A2")), CValue(789.1011)));
  assert(valueMatch(x1.getValue(CPos("B2")), CValue("\"Another\\ \n \t \' \" \\\\ \a \b test\"")));
  std::cout<<"C2 "<<x1.page[CPos("C2")].getContent()<<std::endl;

  assert(valueMatch(x1.getValue(CPos("C2")), CValue("\"Another\\ \n \t \' \" \\\\ \a \b test\"")));


  // Display results or log them
  std::cout << "All tests passed!" << std::endl;
};

int main ()
{
  runTests();
  save_load_tests();
  my_tests();
  
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
  std::cout<<"HI\n"<<std::endl<<std::endl;

  x0.save(std::cout);
  data = oss . str ();
  iss . clear ();
  iss . str ( data );
  std::cout<<data<<std::endl<<std::endl;
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
  back_to_code(1,1002022302);



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
       std::cout<<x0.page[CPos ( "H11" )].getContent()<<std::endl;
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
  std::cout<<"YEPEEE :3"<<std::endl;
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
