/* #include <iostream>
#include <string>
#include <cctype> // For std::isalpha and std::isdigit
#include <set>

// Function to extract and return a set of cell references from a formula
std::set<std::string> extractCellReferences(const std::string& formula) {
    std::set<std::string> references;
    int i = 0, len = formula.length();

    while (i < len) {
        if (std::isalpha(formula[i])) {  // Potential start of a cell reference
            std::string colPart, rowPart;
            bool columnComplete = false;

            // Skip '$' for absolute references
            if (i > 0 && formula[i-1] == '$') {
                i++;
            }

            // Collect column part
            while (i < len && std::isalpha(formula[i])) {
                colPart += std::toupper(formula[i]); // Convert to uppercase for uniformity
                i++;
                columnComplete = true;
            }

            // Check for '$' before the row part
            if (i < len && formula[i] == '$' && columnComplete) {
                i++;
            }

            // Collect row part
            while (i < len && std::isdigit(formula[i])) {
                rowPart += formula[i];
                i++;
            }

            if (!colPart.empty() && !rowPart.empty()) {
                references.insert(colPart + rowPart);  // Add the cleaned-up reference to the set
            }
        } else {
            i++;  // Move to the next character
        }
    }

    return references;
}

int main() {
    std::string formula = "sadad sad 34 sdf s3";
    std::set<std::string> references = extractCellReferences(formula);

    std::cout << "Extracted References: ";
    for (const auto& ref : references) {
        std::cout << ref << " ";
    }
    std::cout << std::endl;

    return 0;
}
 */

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>

class CSpreadsheet {
public:
    std::map<std::string, CCell> cells;

    bool detectCycles() {
        std::set<std::string> visited;
        std::set<std::string> recStack;

        for (auto& [key, cell] : cells) {
            if (cell.get_type() == CCell::FORMULA) {
                if (isCyclic(key, visited, recStack)) {
                    return true; // Cycle found
                }
            }
        }
        return false; // No cycles found
    }

private:
    bool isCyclic(const std::string& cellId, std::set<std::string>& visited, std::set<std::string>& recStack) {
        if (recStack.find(cellId) != recStack.end()) {
            return true; // Current cell is already in recursion stack -> cycle
        }
        if (visited.find(cellId) != visited.end()) {
            return false; // Already visited
        }

        visited.insert(cellId);
        recStack.insert(cellId);

        // Get the cell object from the map
        auto& cell = cells[cellId];
        for (const auto& ref : cell.references) {
            if (isCyclic(ref, visited, recStack)) {
                return true;
            }
        }

        recStack.erase(cellId);
        return false;
    }
};

class CCell {
public:
    enum Type { NUMERIC, TEXT, FORMULA };
    Type type;
    std::set<std::string> references; // This should be filled by parsing the formula

    Type get_type() const {
        return type;
    }
};

int main() {
    CSpreadsheet ss;
    ss.cells["A1"] = CCell{CCell::FORMULA};
    ss.cells["A1"].references.insert("B1");
    ss.cells["B1"] = CCell{CCell::FORMULA};
    ss.cells["B1"].references.insert("C1");
    ss.cells["C1"] = CCell{CCell::FORMULA};
    ss.cells["C1"].references.insert("A1"); // Introducing a cycle A1 -> B1 -> C1 -> A1

    if (ss.detectCycles()) {
        std::cout << "Cycle detected!" << std::endl;
    } else {
        std::cout << "No cycles." << std::endl;
    }
    return 0;
}
