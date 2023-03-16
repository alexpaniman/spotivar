#include "search.h"
//#include "server.h"

#include "axp/exceptions.h"

void bar() {
    throw axp::exception { "I can't do this anymore!" };
}


void foo() {
    bar();
}

void other_foo() {
    try {
        foo();
    } catch (...) {
        throw axp::nested_exception { "Foo couldn't do it's job!" };
    }
}

void master_of_foos() {
    try {
        other_foo();
    } catch (...) {
        throw axp::nested_exception { "It all is falling apart!" };
    }
}


int main() {

    master_of_foos();

}
