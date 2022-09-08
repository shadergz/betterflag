#pragma once

#include <cstring>

#include <vector>
#include <algorithm>
#include <memory>
#include <optional>

namespace BetterFlag {

    enum FlagType {
        FLAG_INTEGER, FLAG_UNSIGNED_INTEGER,
        FLAG_STRING

    };

    class FlagOption {
    public:
        FlagOption(FlagType type, void* flagValuePtr, const char* flagName, const char* flagDesc) :
            mType(type), mFlagValuePtr(flagValuePtr), mFlagName(flagName),
            mFlagDesc(flagDesc) {}
        const FlagType mType;
        const char* mFlagName;
        const char* mFlagDesc;

        void* mFlagValuePtr;
        uint8_t mDefaultValue[sizeof(void*)]{};

        bool mUsed = false;
    };

    typedef void (*VisitCallback)(const std::shared_ptr<FlagOption>& flagOption);

    enum FlagStatus {
        FLAG_CONTINUE, FLAG_PARSED, FLAG_WOUT_VALUE
    };

    class Flag {
    public:
        Flag(int argc, const char **argList) :mArgc(argc), mArgList(argList) {
            mNonArguments.reserve(mArgc);
        }
        [[maybe_unused]] void PrintDefaults() const {
            char usageBuffer[200];
            for (const auto& flagOpt : mAvaFlags) {
                std::snprintf(usageBuffer, sizeof(usageBuffer),
                              "-%s ?", flagOpt->mFlagName);
                switch (flagOpt->mType) {

                #define FLAG_FILL_USAGE_BUFFER(valueType, flagOut, format, ...) \
                sprintf(strpbrk(usageBuffer, "?"), valueType"\n\t?");\
                    snprintf(strpbrk(usageBuffer, "?"),\
                             sizeof(usageBuffer)/2, "%s (default ?", flagOpt->mFlagDesc);\
                    sprintf(strpbrk(usageBuffer, "?"), format")",\
                    __VA_ARGS__)

                case FLAG_INTEGER:
                    FLAG_FILL_USAGE_BUFFER("int", flagOpt, "%d",
                                           *reinterpret_cast<int*>(flagOpt->mDefaultValue));
                    break;
                case FLAG_UNSIGNED_INTEGER:
                    FLAG_FILL_USAGE_BUFFER("unsigned int", flagOut, "%u",
                                           *reinterpret_cast<unsigned int*>(flagOpt->mDefaultValue));
                    break;

                case FLAG_STRING:
                    FLAG_FILL_USAGE_BUFFER("string", flagOpt, "%s",
                                           *reinterpret_cast<const char**>(flagOpt->mDefaultValue));
                    break;
                }
                std::fprintf(mPrintOut, "%s\n", usageBuffer);
            }
        }

        [[nodiscard]] auto NFlag() const {
            return mNArgs;
        }
        /* Set sets the value of the named command-line flag */
        [[maybe_unused]] void Set(const char *flagName, const char *flagValue) {
            for (const auto& flagAva : mAvaFlags) {
                if (strcmp(flagAva->mFlagName, flagName) != 0) {
                    continue;
                }
                DefineFlagValue(flagAva, flagValue);
            }
        }

        /* Visit visits the command-line flags in lexicographical order, calling fn for each.
         * It visits only those flags that have been set.
        */
        [[maybe_unused]] void Visit(VisitCallback visitCallback) {
            for (const auto& flagAva : mAvaFlags) {
                if (flagAva->mUsed) {
                    visitCallback(flagAva);
                }
            }
        }
        [[maybe_unused]] std::optional<const std::shared_ptr<FlagOption>> Lookup(const char *flagName) {
            for (const auto& flagAva : mAvaFlags) {
                if (strcmp(flagAva->mFlagName, flagName) == 0) {
                    return flagAva;
                }
            }
            return {};
        }

        [[maybe_unused]] [[nodiscard]] auto Arg(int index) const {
            if (index >= mArgc) {
                return "";
            }
            return mArgList[index];
        }

        /* VisitAll visits the command-line flags in lexicographical order, calling fn for each.
         * It visits all flags, even those not set.
        */
        void VisitAll(VisitCallback visitCallback) {
            for (const auto& flagAva : mAvaFlags) {
                visitCallback(flagAva);
            }
        }

        /* Output returns the destination for usage and error messages.
         * stderr is returned if output was not set or was set to nil. */
        [[nodiscard]] [[maybe_unused]] auto Output() const {
            return mPrintOut;
        }

        [[maybe_unused]] void SetOutput(FILE *outPrint) {
            mPrintOut = outPrint;
        }

        void Parse() {
            /* Setting defaults before starts */
            for (const auto& flagOption : mAvaFlags) {
                switch (flagOption->mType) {
                case FLAG_INTEGER:
                    *static_cast<int*>(flagOption->mFlagValuePtr) = *reinterpret_cast<int*>(flagOption->mDefaultValue);
                    break;
                case FLAG_STRING:
                    *static_cast<const char**>(flagOption->mFlagValuePtr) =
                            *reinterpret_cast<const char**>(flagOption->mDefaultValue);
                    break;
                case FLAG_UNSIGNED_INTEGER:
                    *static_cast<unsigned int*>(flagOption->mFlagValuePtr) =
                            *reinterpret_cast<unsigned int*>(flagOption->mDefaultValue);
                    break;
                }
            }

            for (auto argStr = *++mArgList; *mArgList; argStr = *++mArgList) {

                if (strpbrk(argStr, "-") == nullptr) {
                    /* argStr isn't an valid flag */
                    if (mStatus == FlagStatus::FLAG_WOUT_VALUE) {
                        DefineFlagValue(mContextOption.lock(), argStr);
                        mStatus = FlagStatus::FLAG_CONTINUE;
                        continue;
                    } else {
                        mNonArguments.emplace_back(*mArgList);
                    }
                    continue;
                }
                while (*argStr == '-') argStr++;
                const char* flagArg = strpbrk(argStr, "=");

                for (const auto& flagAva : mAvaFlags) {
                    if (strncmp(flagAva->mFlagName, argStr, flagArg - argStr) != 0) {
                        continue;
                    }
                    /* Flag matches */
                    if (flagArg == nullptr) {
                        /* We will save this context for the next flag */
                        mContextOption = flagAva;
                        mStatus = FlagStatus::FLAG_WOUT_VALUE;
                        break;
                    }
                    DefineFlagValue(flagAva, ++flagArg);
                }

            }
            mStatus = FlagStatus::FLAG_PARSED;
        }
        [[maybe_unused]] void UIntVar(unsigned int* intPtr, const char* flagName, unsigned int defaultValue,
                                      const char* flagDesc) {
            auto flagInt = std::make_shared<FlagOption>(
                    FlagType::FLAG_UNSIGNED_INTEGER, static_cast<void*>(intPtr), flagName, flagDesc);
            *reinterpret_cast<unsigned int*>(flagInt->mDefaultValue) = defaultValue;
            mAvaFlags.push_back(flagInt);
        }

        [[maybe_unused]] void IntVar(int* intPtr, const char* flagName, int defaultValue, const char* flagDesc) {
            auto flagInt = std::make_shared<FlagOption>(
                    FlagType::FLAG_UNSIGNED_INTEGER, static_cast<void*>(intPtr), flagName, flagDesc);
            *reinterpret_cast<int*>(flagInt->mDefaultValue) = defaultValue;
            mAvaFlags.push_back(flagInt);
        }

        [[maybe_unused]] void StringVar(char** stringPtr, const char* flagName, const char* defaultValue,
                                        const char* flagDesc) {
            auto flagInt = std::make_shared<FlagOption>(
                    FlagType::FLAG_STRING, static_cast<void*>(stringPtr), flagName, flagDesc);
            *reinterpret_cast<const char**>(flagInt->mDefaultValue) = defaultValue;
            mAvaFlags.push_back(flagInt);
        }

        bool Passed() {
            return mStatus == FLAG_PARSED;
        }
        [[nodiscard]] std::vector<const char*> Args() const {
            return mNonArguments;
        }
    private:
        void DefineFlagValue(const std::shared_ptr<FlagOption>& flagAva, const char* flagArg) {
            switch (flagAva->mType) {
            case FLAG_INTEGER:
                *static_cast<int*>(flagAva->mFlagValuePtr) =
                        static_cast<int>(strtoul(flagArg, nullptr, 0));
                break;
            case FLAG_UNSIGNED_INTEGER:
                *static_cast<unsigned int*>(flagAva->mFlagValuePtr) =
                        static_cast<unsigned int>(strtoul(flagArg, nullptr, 0));
                break;
            case FLAG_STRING:
                *static_cast<const char**>(flagAva->mFlagValuePtr) = flagArg;
                break;
            }
            mNArgs++;
            flagAva->mUsed = true;
        }

        int mArgc;
        const char **mArgList;
        FlagStatus mStatus = FlagStatus::FLAG_CONTINUE;
        std::vector<const char*> mNonArguments;
        std::vector<std::shared_ptr<FlagOption>> mAvaFlags;

        /* Number of argument flags that has been setted */
        ssize_t mNArgs{};
        std::weak_ptr<FlagOption> mContextOption;

        /* Default 'PrintDefaults' output FILE */
        FILE* mPrintOut = stderr;

    };

}


