module;

export module Argo:ParserImpl;

import :MetaAssigner;
import :Parser;
import :MetaLookup;
import :HelpGenerator;
import :NArgs;
import :Arg;
import :std_module;
import :Exceptions;
import :MetaChecker;

namespace Argo {

auto splitStringView(std::string_view str, char delimeter) -> std::vector<std::string_view> {
  std::vector<std::string_view> ret;
  while (str.contains(delimeter)) {
    auto pos = str.find(delimeter);
    ret.push_back(str.substr(0, pos));
    str = str.substr(pos + 1);
  }
  ret.push_back(str);
  return ret;
}

template <ParserID ID, class Args, class PositionalArg, bool HelpEnabled>
auto Parser<ID, Args, PositionalArg, HelpEnabled>::setArg(
    std::string_view key, std::span<std::string_view> val) const -> void {
  if constexpr (HelpEnabled) {
    if (key == "help") {
      std::println("{}", formatHelp());
      std::exit(0);
    }
  }
  Assigner<Arguments, PositionalArgument>::assign(key, val);
}

template <ParserID ID, class Args, class PositionalArg, bool HelpEnabled>
auto Parser<ID, Args, PositionalArg, HelpEnabled>::setArg(
    std::span<char> key, std::span<std::string_view> val) const -> void {
  if constexpr (HelpEnabled) {
    for (const auto& i : key) {
      if (i == 'h') {
        std::println("{}", formatHelp());
        std::exit(0);
      }
    }
  }
  Assigner<Arguments, PositionalArgument>::assign(key, val);
}

template <ParserID ID, class Args, class PositionalArg, bool HelpEnabled>
auto Parser<ID, Args, PositionalArg, HelpEnabled>::parse(int argc, char* argv[]) -> void {
  std::string_view key{};
  std::vector<char> short_keys{};
  short_keys.reserve(10);
  std::vector<std::string_view> values{};
  values.reserve(10);

  for (int i = 1; i < argc; i++) {
    std::string_view arg = argv[i];
    if (arg.starts_with('-')) {
      if (arg.starts_with("--")) {  // start with --
        if (!key.empty()) {
          this->setArg(key, values);
          key = "";
          values.clear();
        } else if (!short_keys.empty()) {
          this->setArg(short_keys, values);
          short_keys.clear();
          values.clear();
        } else if (!values.empty()) {
          if constexpr (!std::is_same_v<PositionalArgument, void>) {
            this->setArg(key, values);
            values.clear();
          }
        }
        if (arg.contains('=')) {
          auto equal_pos = arg.find('=');
          auto value = std::vector<std::string_view>{arg.substr(equal_pos + 1)};
          this->setArg(arg.substr(2, equal_pos - 2), value);
          continue;
        }
        key = arg.substr(2);
        if (i == (argc - 1)) {
          this->setArg(key, {});
        }
        continue;
      }
      if (!key.empty()) {
        this->setArg(key, values);
        key = "";
        values.clear();
      } else if (!short_keys.empty()) {
        this->setArg(short_keys, values);
        short_keys.clear();
        values.clear();
      } else if (!values.empty()) {
        if constexpr (!std::is_same_v<PositionalArgument, void>) {
          this->setArg(key, values);
          values.clear();
        }
      }
      if (key.empty() && short_keys.empty()) {
        for (const auto& j : arg.substr(1)) {
          short_keys.push_back(j);
        }
        if (i == (argc - 1)) {
          this->setArg(short_keys, {});
        }
        continue;
      }
    } else {
      if constexpr (std::is_same_v<PositionalArgument, void>) {
        if (key.empty() && short_keys.empty()) {
          throw InvalidArgument(std::format("No keys specified"));
        }
      }
      values.push_back(arg);

      if (i == argc - 1) {
        if (!key.empty()) {
          this->setArg(key, values);
        } else if (!short_keys.empty()) {
          this->setArg(short_keys, values);
        } else {
          if constexpr (std::is_same_v<PositionalArgument, void>) {
            throw InvalidArgument(std::format("No keys specified"));
          } else {
            this->setArg(key, values);
          }
        }
      }
    }
  }
  auto required_keys = RequiredChecker<Arguments>::check();
  if (!required_keys.empty()) {
    throw InvalidArgument(std::format("Requried {}", required_keys));
  }
  this->parsed_ = true;
}

template <ParserID ID, class Args, class PositionalArg, bool HelpEnabled>
auto Parser<ID, Args, PositionalArg, HelpEnabled>::formatHelp() const -> std::string {
  std::string help;
  auto helpInfo = HelpGenerator<Arguments>::generate();
  std::size_t maxFlagLength = 0;
  for (const auto& option : helpInfo) {
    if (maxFlagLength < option.name.size()) {
      maxFlagLength = option.name.size();
    }
  }
  help.append("Options:");
  for (const auto& option : helpInfo) {
    help.push_back('\n');
    auto description = splitStringView(option.description, '\n');

    help.append(std::format(                                                             //
        "  {} --{} {} {}",                                                               //
        (option.shortName == NULLCHAR) ? "   " : std::format("-{},", option.shortName),  //
        option.name,                                                                     //
        std::string(maxFlagLength - option.name.size(), ' '),                            //
        description[0]                                                                   //
        ));
    for (std::size_t i = 1; i < description.size(); i++) {
      help.push_back('\n');
      help.append(std::format(              //
          "      {}    {}",                 //
          std::string(maxFlagLength, ' '),  //
          description[i]                    //
          ));
    }

    // erase trailing spaces
    auto pos = help.find_last_not_of(' ');
    help = help.substr(0, pos + 1);
  }
  return help;
}

}  // namespace Argo
