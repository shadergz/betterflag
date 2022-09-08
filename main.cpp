#include <fmt/format.h>

#include "betterflag.h"

void PrintAllOptions(const std::shared_ptr<BetterFlag::FlagOption>& flagOption) {

    fmt::print("Option: {} is used? {}\n", flagOption->mFlagName, flagOption->mUsed ? "yes" : "no");

}

int main(int argc, char **argv) {
    BetterFlag::Flag flag(argc, const_cast<const char**>(argv));

    unsigned int xValue{};

    char *userName;

    flag.UIntVar(&xValue, "xValue", 100, "Define X value, default is 100");
    flag.StringVar(&userName, "username", "Gabriel Correia", "Setups user name");

    flag.Parse();

    fmt::print("Flag has parsed? {}\n", flag.Passed() ? "yes" : "no");

    auto nonArgs = flag.Args();

    for (auto argStr : nonArgs) {
        fmt::print("Non flag value: {}\n", argStr);
    }

    /* flag.Set("xValue", "198"); */

    flag.PrintDefaults();
    flag.VisitAll(PrintAllOptions);

    fmt::print("Number of arguments setted: {}\n", flag.NFlag());

    fmt::print("X value: {}\n", xValue);
    fmt::print("Username: {}\n", userName);

    return 0;
}
