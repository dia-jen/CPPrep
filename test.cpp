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
constexpr unsigned                     SPREADSHEET_PARSER                      = 0x10;
#endif /* __PROGTEST__ */
class CPos;
std::pair<int,int> CPos_parser(std::string_view str);
class CSpreadsheet;


class Expr
{
public:
  virtual CValue eval(CSpreadsheet *spreadsheat) const = 0;
  virtual void adjust(int deltaCol, int deltaRow) const = 0;
  virtual std::shared_ptr<Expr> clone(CSpreadsheet *spreadsheat) const = 0;
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
  ExprPtr clone(CSpreadsheet *spreadsheat) const override {
        return std::make_shared<Numeric>(value);
  }
  void adjust(int deltaCol, int deltaRow) const override{return ;}
  CValue eval(CSpreadsheet *spreadsheat) const override
  {
    std::cout<<"Evaluating numeric\n";
    return value;
  }
};

class Text : public Expr
{
public:
  explicit Text(std::string val) : value(std::move(val)) {}
  int getType()const override{ return 0; }
  void adjust(int deltaCol, int deltaRow) const override{return ;}
  ExprPtr clone(CSpreadsheet *spreadsheat) const override {
        return std::make_shared<Text>(value);
    }
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
    void adjust(int deltaCol, int deltaRow) const override{
      left->adjust(deltaCol,deltaRow);
      right->adjust(deltaCol,deltaRow);
      return ;
      }
    ExprPtr clone(CSpreadsheet *spreadsheat) const override {
        return std::make_shared<Sum>(left->eval(spreadsheat),right->eval(spreadsheat));
    }
    CValue eval(CSpreadsheet *spreadsheet) const override {
        auto lVal = left->eval(spreadsheet);
        auto rVal = right->eval(spreadsheet);
        if (std::holds_alternative<std::monostate>(lVal) || std::holds_alternative<std::monostate>(rVal))
            return std::monostate();
        if (std::holds_alternative<double>(lVal) && std::holds_alternative<double>(rVal))
            return std::get<double>(lVal) + std::get<double>(rVal);
        return std::to_string(std::get<double>(lVal)) + std::get<std::string>(rVal);
    }
};

class Subtraction : public Expr {
private:
    ExprPtr left, right;

public:
    Subtraction(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}
    int getType()const override{ return 0; }
    void adjust(int deltaCol, int deltaRow) const override{
      left->adjust(deltaCol,deltaRow);
      right->adjust(deltaCol,deltaRow);
      return ;
      }    
    ExprPtr clone(CSpreadsheet *spreadsheat) const override {
        return std::make_shared<Subtraction>(left->eval(spreadsheat),right->eval(spreadsheat));
    }
    CValue eval(CSpreadsheet *spreadsheet) const override {
        auto lVal = left->eval(spreadsheet);
        auto rVal = right->eval(spreadsheet);
        if (std::holds_alternative<std::monostate>(lVal) && std::holds_alternative<std::monostate>(rVal))
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
  void adjust(int deltaCol, int deltaRow) const override
  {
    left->adjust(deltaCol, deltaRow);
    return;
  }
  ExprPtr clone(CSpreadsheet *spreadsheat) const override
  {
    return std::make_shared<Negative>(left->eval(spreadsheat));
  }
  CValue eval(CSpreadsheet *spreadsheat) const override
  {
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
  void adjust(int deltaCol, int deltaRow) const override
  {
    left->adjust(deltaCol, deltaRow);
    right->adjust(deltaCol, deltaRow);
    return;
  }
  ExprPtr clone(CSpreadsheet *spreadsheat) const override
  {
    return std::make_shared<Multiplication>(left->eval(spreadsheat),right->eval(spreadsheat));
  }

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
  void adjust(int deltaCol, int deltaRow) const override
  {
    left->adjust(deltaCol, deltaRow);
    right->adjust(deltaCol, deltaRow);
    return;
  }
  ExprPtr clone(CSpreadsheet *spreadsheat) const override
  {
    return std::make_shared<Division>(left->eval(spreadsheat),right->eval(spreadsheat));
  }
  CValue eval(CSpreadsheet *spreadsheat) const override
  {
    auto lVal = left->eval(spreadsheat);
    auto rVal = right->eval(spreadsheat);
    if (std::get<double>(right->eval(spreadsheat)) == 0 || !std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
    {
      throw std::runtime_error("Math error: Attempted to divide by Zero\n");
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
  void adjust(int deltaCol, int deltaRow) const override
  {
    left->adjust(deltaCol, deltaRow);
    right->adjust(deltaCol, deltaRow);
    return;
  }
  ExprPtr clone(CSpreadsheet *spreadsheat) const override
  {
    return std::make_shared<Power>(left->eval(spreadsheat),right->eval(spreadsheat));
  }
  CValue eval(CSpreadsheet *spreadsheat) const override
  {

    auto lVal = left->eval(spreadsheat);
    auto rVal = right->eval(spreadsheat);

    if (!std::holds_alternative<double>(lVal) || !std::holds_alternative<double>(rVal))
      return std::monostate(); // or 0?
    return pow(std::get<double>(lVal), std::get<double>(rVal));
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
      if (std::dynamic_pointer_cast<Text>(right) || std::dynamic_pointer_cast<Text>(left)) {
        throw std::runtime_error("Invalid operation: Addition involving text"); // You could throw an error or handle text concatenation
      }//vie to scitavat text, nehadya exception ale prazdnu hodnotu
      exprStack.push(std::make_shared<Sum>(left, right));
      return;
    };
    void opSub() override
    {
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      if (std::dynamic_pointer_cast<Text>(right) || std::dynamic_pointer_cast<Text>(left)) {
        throw std::runtime_error("Invalid operation: Operation involving text");
      }
      exprStack.push(std::make_shared<Subtraction>(left, right));
      return;
    };
    void opMul() override
    {
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      if (std::dynamic_pointer_cast<Text>(right) || std::dynamic_pointer_cast<Text>(left)) {
        throw std::runtime_error("Invalid operation: Operation involving text");
      }
      exprStack.push(std::make_shared<Multiplication>(left, right));
      return;
    };
    void opDiv() override
    {
      std::cout << " \\ " ;
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      if (std::dynamic_pointer_cast<Text>(right) || std::dynamic_pointer_cast<Text>(left)) {
        throw std::runtime_error("Invalid operation: Operation involving text");
      }
      exprStack.push(std::make_shared<Division>(left, right));
      return;
    };
    void opPow() override
    {
      std::cout << " ^ " ;
      auto right = exprStack.top(); exprStack.pop();
      auto left = exprStack.top(); exprStack.pop();
      if (std::dynamic_pointer_cast<Text>(right) || std::dynamic_pointer_cast<Text>(left)) {
        throw std::runtime_error("Invalid operation: Operation involving text");
      }
      exprStack.push(std::make_shared<Power>(left, right));
      return;
    };

    void opNeg() override {
      std::cout << " neg \n" ;
      auto left = exprStack.top(); exprStack.pop();
      if (std::dynamic_pointer_cast<Text>(left)) {
        // You could throw an error or handle text concatenation
        throw std::runtime_error("Invalid operation: Operation involving text");
      }
      exprStack.push(std::make_shared<Negative>(left));
      return;
    };
    void opEq() override { return; };
    void opNe() override { return; };
    void opLt() override { return; };
    void opLe() override { return; };
    void opGt() override { return; };
    void opGe() override { return; };

    void valNumber(double val) override { 
      std::cout << val ;
      exprStack.push(std::make_shared<Numeric>(val));
      return; 
    };
    void valString(std::string val) override { 
      exprStack.push(std::make_shared<Text>(val));
      std::cout<< val<<std::endl; 
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
    expBuilder formula; //->this will be parsed
    void updateFormulaReferences(int deltaCol, int deltaRow);
    std::string content_editor(int deltaColum, int deltaRow);
  private:
    type content_type;
    CValue content;
    std::stack<ExprPtr> formulaStack;
    std::string original_content;
};
CCell CCell::clone() const{
  CCell newCopy(this->original_content);
  return newCopy;
};

std::string CCell::getContent()const{
  return original_content;
}
std::string content_editor(){

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
    CCell cell;
    std::string getCode() const;
    std::pair<unsigned int, unsigned int> getRaC() const;
    std::pair<int, int> CPos_parser(std::string_view str) ;
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
    int column = 10;
    int row = 0;
    size_t index = 0;

    // Process column letters
    if(str[index]=='$') {
      index++; 
      relative_column = false;
    }
    while (index < str.size() && std::isalpha(str[index])) {
        column = column * 100 + (std::tolower(str[index]) - 'a' + 1);
        ++index;
    }
     if (index < str.size() && str[index] == '$') {
      relative_row = false;
      index++;
    }
    // Convert the remainder of the string to row number
    if (index < str.size()) {
        try {
            row = std::stoi(std::string(str.substr(index)));
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid row number in input: " << str << std::endl;
            throw;
        }
    }

    position.first = row;
    position.second = column;
    return position;
}

std::string back_to_code(unsigned int row, unsigned int column) {
    std::string columnLabel;

    // Convert column number to string and skip the first two digits ("10")
    std::string columnStr = std::to_string(column).substr(2);

    // Process each pair of digits
    for (size_t i = 0; i < columnStr.length(); i += 2) {
        if (i + 1 < columnStr.length()) {
            int letterNumber = std::stoi(columnStr.substr(i, 2)) - 1; // Convert back to 0-based
            char letter = 'A' + letterNumber;
            columnLabel += letter;
        }
    }

    // Convert row number to string
    std::string rowLabel = std::to_string(row);
    std::cout<< columnLabel + rowLabel << std::endl;
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
    return SPREADSHEET_CYCLIC_DEPS | SPREADSHEET_FUNCTIONS | SPREADSHEET_FILE_IO | SPREADSHEET_SPEED;
  }
  CSpreadsheet(){};
  bool load(std::istream &is);
  bool save(std::ostream &os) const;
  bool setCell(CPos pos, std::string contents);
  CValue getValue(CPos pos);
  void copyRect(CPos dst, CPos src, int w = 1, int h = 1);
  void print(); /// @note my implemented functions
//private:
  std::set<CPos> page; /// @note Stores our cell position and cells with relation to the given cell
};

void CSpreadsheet::print(){
  std::cout<<"Size of page:\t"<< page.size() << std::endl;
  std::cout<<"Printing set of cells: \n";
  for(const auto  & part : page){
    part.print();
  }
};

CValue CSpreadsheet::getValue(CPos pos)
{
  auto it = page.find(pos);
  it->cell.formula;
  if( it == page.end()) 
    return CValue();
  return it->cell.getValue(this) ; 
};

bool CSpreadsheet::setCell(CPos pos, std::string contents)
{
  CCell tmp(contents);
  page.erase(pos); // Every time id runs it tries to erase cpos, afther that it updates the value
  pos.cell = tmp;
  page.insert(pos);
  //std::cout<<"CELL ADD SUCCESSFULL\n";
  this->print();
  return true;
};

bool CSpreadsheet::save(std::ostream &os) const
{
  for (const auto &it : this->page)
  {
    std::string encodedContent = it.cell.getContent();
    // Replace newlines with \\n to ensure they are saved in a single line
    size_t pos = 0;
    while ((pos = encodedContent.find('\n', pos)) != std::string::npos)
    {
      encodedContent.replace(pos, 1, "\\n");
      pos += 2; // Skip past the inserted sequence
    }
    os << "BUNK" << it.getCode() << "CONT" << encodedContent << std::endl;
  }
  return true;
}

bool CSpreadsheet::load(std::istream &is)
{
  std::string line;
  while (getline(is, line))
  {
    if (line.empty())
      continue; // Skip empty lines
    size_t bunkPos = line.find("BUNK");
    size_t contPos = line.find("CONT");
    if (bunkPos == std::string::npos || contPos == std::string::npos || bunkPos > contPos)
    {
      std::cerr << "Error: Invalid line format. Line: " << line << std::endl;
      return false;
    }

    std::string cellPos = line.substr(bunkPos + 4, contPos - (bunkPos + 4));
    std::string contents = line.substr(contPos + 4);

    // Decode escaped newlines
    size_t pos = 0;
    while ((pos = contents.find("\\n", pos)) != std::string::npos)
    {
      contents.replace(pos, 2, "\n");
      pos += 1; // Move past the newline
    }

    CPos posObject(cellPos);
    if (!setCell(posObject, contents))
    {
      std::cerr << "Error setting cell at position " << cellPos << " with content " << contents << std::endl;
      return false;
    }
  }
  return true;
}

void CSpreadsheet::copyRect(CPos dst, CPos src, int w, int h) {
    int deltaRow = dst.getRaC().first - src.getRaC().first;
    int deltaCol = dst.getRaC().second - src.getRaC().second;

    for (int row = 0; row < h; ++row) {
        for (int col = 0; col < w; ++col) {
            CPos currentSrc = src.offset(col, row);
            CPos currentDst = dst.offset(col, row);
            auto srcIt = page.find(currentSrc);
            if (srcIt != page.end()) {
                // Clone the source cell to avoid modifying the original
                if(srcIt->cell.getContent()!=""){
                  CCell srcCell(srcIt->cell.getContent());  // Assuming clone() is properly implemented
                
                std::cout<<srcIt->cell.getContent()<<"UPDATINg FORMULA\n";
                // Update references in the cloned cell if it's a formula

                // Set the cloned and potentially modified cell at the new position
              if(srcCell.get_type()==CCell::type::FORMULA ) {
                  std::cout<<"UPDATINg FORMULA\n";
                  currentDst.cell = srcCell;
                  std::cout<<srcIt->cell.getContent()<<std::endl;
                  currentDst.cell.updateFormulaReferences(deltaCol, deltaRow);
                  page.erase(currentDst); // Every time id runs it tries to erase cpos, afther that it updates the value
                  page.insert(currentDst);

                  }
                else if((srcCell.get_type() == CCell::type::NUMERIC) || (srcCell.get_type() == CCell::type::TEXT))setCell(currentDst,srcCell.getContent());
            }
            else continue;
            } 
        }
    }
}

//   x0 . copyRect ( CPos ( "G11" ), CPos ( "F10" ), 1, 4 );
// dst = 11 1007 src =10 1006 vec(1, 1)
// srcF = = D0+5 : 10 + 5 dstF = E1+5 70 + 5

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
    void adjust(int deltaCol, int deltaRow)const override {
      std::cout << "Before Adjustment - Row: " << pos.row << ", Column: " << pos.column << std::endl;
      if (pos.relative_column) {
          pos.column += deltaCol;
          std::cout << "Adjusted Column by " << deltaCol << std::endl;
      }
      if (pos.relative_row) {
          pos.row += deltaRow;
          std::cout << "Adjusted Row by " << deltaRow << std::endl;
      }
      std::cout << "After Adjustment - Row: " << pos.row << ", Column: " << pos.column << std::endl;
    }
    ExprPtr clone(CSpreadsheet) const override {
        return std::make_shared<Reference>(pos);
  }

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


void CCell::updateFormulaReferences(int deltaCol, int deltaRow) {
    std::stack<ExprPtr> tempStack;

    // Debugging output
    std::cout << "Updating formula references with deltas: " << deltaCol << ", " << deltaRow << std::endl;

    // Reverse the stack to access in order
    while (!formula.exprStack.empty()) {
        tempStack.push(formula.exprStack.top());
        formula.exprStack.pop();
    }
    if (tempStack.empty()) {
        std::cout << "Warning: exprStack is empty when trying to update references." << std::endl;
    }

    while (!tempStack.empty()) {
            std::cout << "!!!!!!!!!!!!!!!!!\n " << std::endl;

        auto expr = tempStack.top();
        tempStack.pop();
        expr->adjust(deltaCol,deltaRow);
        formula.exprStack.push(expr);
    }
}

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
 
void my_tests(){
  CSpreadsheet x0, x1, x2;
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




  std::cout<<"==========================PASS 2==========================\n";

  std::cout<<"\n BASIC TESTS YEPEEE :3\n"<<std::endl;
}


int main ()
{
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
  x1.print();
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
