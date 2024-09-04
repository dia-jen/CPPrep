#include <iostream>
#include "CLandRegister.h"  // Include your header file here

int main() {
    CLandRegister x;
    // Add some entries
    x.add("City1", "Address1", "Region1", 12345, "Owner1");
    x.add("City2", "Address2", "Region2", 12346, "Owner2");

    // Test listByOwner
    CIterator it = x.listByOwner("Owner1");
    if (!it.atEnd()) {
        std::cout << "Iterator points to: " << it.city() << " " << it.addr() << " " << it.region() << " " << it.id() << " " << it.owner() << std::endl;
    } else {
        std::cout << "Iterator is at end" << std::endl;
    }

    return 0;
}
