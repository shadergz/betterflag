# betterflag

It's a flag (golang package) implementation in C++, only useful methods has been implemented

## Impl

The main implementation is distributed inside a C++ header file, making everything more portable

### Usage example

~~~c++

BetterFlag::Flag flag(argc, const_cast<const char**>(argv));

unsigned int xValue{};
char *userName;

flag.UIntVar(&xValue, "xValue", 100, "Define X value, default is 100");
flag.StringVar(&userName, "username", "Gabriel Correia", "Setups user name");
flag.Parse();
    
fmt::print("X value: {}\n", xValue);
fmt::print("Username: {}\n", userName);

~~~

> Some properties are inherited from Golang implementation

### Supported methods

- void PrintDefaults() const;
- auto NFlag() const; 
- void Set(const char *flagName, const char *flagValue);
- void Visit(VisitCallback visitCallback);
- std::optional<const std::shared_ptr<FlagOption>> Lookup(const char *flagName);
- auto Arg(int index) const;
- void VisitAll(VisitCallback visitCallback);
- auto Output() const;
- void SetOutput(FILE *outPrint);
- void Parse();
- void UIntVar(unsigned int* intPtr, const char* flagName, unsigned int defaultValue,
  const char* flagDesc);
- void IntVar(int* intPtr, const char* flagName, int defaultValue, const char* flagDesc);
- StringVar(char** stringPtr, const char* flagName, const char* defaultValue, const char* flagDesc);
- bool Passed();
- std::vector<const char*> Args();
- void DefineFlagValue(const std::shared_ptr<FlagOption>& flagAva, const char* flagArg);
- Float64(double* intPtr, const char* flagName, double defaultValue, const char* flagDesc);

### Supported values

- FLAG_INTEGER (int)
- FLAG_UNSIGNED_INTEGER (unsigned int)
- FLAG_STRING (const char*)
- FLAG_FLOAT_64 (double)
