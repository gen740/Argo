module;

#include <cassert>
#include <print>

#include "Argo/ArgoMacros.hh"

export module Argo:ParserImpl;

import :TypeTraits;
import :MetaAssigner;
import :Parser;
import :MetaLookup;
import :MetaParse;
import :HelpGenerator;
import :Arg;
import :std_module;
import :Exceptions;

// generator start here

namespace Argo {

using namespace std;

ARGO_ALWAYS_INLINE auto splitStringView(string_view str, char delimeter)
    -> vector<string_view> {
  vector<string_view> ret;
  while (str.contains(delimeter)) {
    auto pos = str.find(delimeter);
    ret.push_back(str.substr(0, pos));
    str = str.substr(pos + 1);
  }
  ret.push_back(str);
  return ret;
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::resetArgs() -> void {
  ValueReset<Args>();
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::setArg(
    string_view key, const span<string_view>& val) const -> void {
  if constexpr (!is_same_v<HArg, void>) {
    if (key == HArg::name.getKey()) {
      std::cout << formatHelp() << std::endl;
      exit(0);
    }
  }
  Assigner<Args, PArgs>(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::setShortKeyArg(
    string_view key, const span<string_view>& val) const -> void {
  if (ShortArgAssigner<Args, PArgs, HArg>(key, val)) [[unlikely]] {
    std::cout << formatHelp() << std::endl;
    exit(0);
  };
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::parse(int argc,
                                                                char* argv[])
    -> void {
  if (this->parsed_) [[unlikely]] {
    throw ParseError("Cannot parse twice");
  }
  auto assigned_keys = vector<string_view>();
  tuple_type_visit<decltype(tuple_cat(declval<Args>(), declval<PArgs>()))>(
      [&assigned_keys]<class T>(T) {
        if (T::type::assigned) {
          assigned_keys.push_back(T::type::name.getKey());
        }
      });
  if (!assigned_keys.empty()) [[unlikely]] {
    throw ParseError(format("keys {} already assigned", assigned_keys));
  }

  string_view key{};
  string_view short_keys{};
  vector<string_view> values{};

  assert(this->info_);  // this->info_ cannot be nullptr
  if (!this->info_->program_name) {
    this->info_->program_name = string_view(argv[0]);
  }

  // Search for subcommand
  int32_t subcmd_found_idx = -1;
  int32_t cmd_end_pos = argc;

  for (int i = argc - 1; i > 0; i--) {
    if constexpr (!is_same_v<SubParsers, tuple<>>) {
      if (subcmd_found_idx == -1) {
        subcmd_found_idx = ParserIndex(subParsers, argv[i]);
        if (subcmd_found_idx != -1) {
          cmd_end_pos = i;
        }
      }
    }
  }
  bool is_flag = false;
  string_view arg;

  for (int i = 1; i < cmd_end_pos + 1; i++) {
    if (i != cmd_end_pos) {
      arg = argv[i];
      is_flag = arg.starts_with('-');
      if (arg.size() > 1 and arg.at(1) >= '0' and arg.at(1) <= '9') {
        is_flag = SearchIndexFromShortName<Args>(arg.at(1)) != -1;
      }
    } else {
      is_flag = true;
    }

    if (i != 1 and is_flag) {
      if (!key.empty()) {
        goto SetArgSection;
      }
      if (!short_keys.empty()) {
        goto SetShortArgSection;
      }
      if constexpr (!is_same_v<PArgs, tuple<>>) {
        if (!values.empty()) {
          goto SetArgSection;
        }
      } else {
        if (!values.empty()) [[unlikely]] {
          throw InvalidArgument(
              format("Invalid positional argument: {}", values));
        }
      }
    SetArgSection:
      this->setArg(key, values);
      key = "";
      values.clear();
      goto End;
    SetShortArgSection:
      this->setShortKeyArg(short_keys, values);
      short_keys = "";
      values.clear();
    End:
    }

    if (i == cmd_end_pos) {
      break;
    }

    if (is_flag) {
      if (arg.size() > 1 and arg.at(1) == '-') {
        if (arg.contains('=')) [[unlikely]] {
          auto equal_pos = arg.find('=');
          key = arg.substr(2, equal_pos - 2);
          values.push_back(arg.substr(equal_pos + 1));
          is_flag = true;
        } else {
          key = arg.substr(2);
        }
      } else {
        short_keys = arg.substr(1);
      }
    } else {
      values.push_back(arg);
    }
  }

  auto required_keys = vector<string_view>();
  tuple_type_visit<decltype(tuple_cat(declval<Args>(), declval<PArgs>()))>(
      [&required_keys]<class T>(T) {
        if constexpr (derived_from<typename T::type, ArgTag>) {
          if ((T::type::required && !T::type::assigned)) {
            required_keys.push_back(T::type::name.getKey());
          }
        }
      });

  if (!required_keys.empty()) [[unlikely]] {
    throw InvalidArgument(format("Requried {}", required_keys));
  }
  if (subcmd_found_idx != -1) {
    MetaParse(subParsers, subcmd_found_idx, argc - cmd_end_pos,
              &argv[cmd_end_pos]);
  }
  this->parsed_ = true;
}

struct AnsiEscapeCode {
  bool isEnabled;

  string bold = "\x1B[1m";
  string underline = "\x1B[4m";
  string reset = "\x1B[0m";

  [[nodiscard]] auto getBold() const {
    return isEnabled ? bold : "";
  }

  [[nodiscard]] auto getUnderline() const {
    return isEnabled ? underline : "";
  }

  [[nodiscard]] auto getReset() const {
    return isEnabled ? reset : "";
  }

  [[nodiscard]] auto getBoldUnderline() const {
    return isEnabled ? bold + underline : "";
  }
};

inline size_t max_option_width = 48;

constexpr auto createUsageSection(const auto& program_name,
                                  const auto& help_info, const auto& pargs_info,
                                  const auto& sub_commands) {
  string ret;
  ret.append(program_name);
  for (const auto& i : help_info) {
    if (i.required) {
      ret.append(format(" {} {}",
                        i.shortName != '\0' ? format("-{}", i.shortName)
                                            : format("--{}", i.name),
                        i.typeName));
    }
  }
  ret.append(" [options...]");
  for (const auto& i : pargs_info) {
    if (i.required) {
      ret.append(format(" {}", i.name));
    } else {
      ret.append(format(" [{}]", i.name));
    }
  }
  if (!sub_commands.empty()) {
    ret.append(" {");
    for (const auto& command : sub_commands) {
      ret.append(command.name);
      ret.push_back(',');
    }
    ret.pop_back();
    ret.push_back('}');
  }
  ret.push_back('\n');
  return ret;
}

constexpr auto createSubcommandSection(const auto& ansi,
                                       const auto& sub_commands) {
  string ret;
  ret.push_back('\n');
  for (const auto& command : sub_commands) {
    auto description = splitStringView(command.description, '\n');
    if (command.name.size() < max_option_width and description[0] != "") {
      ret.append(
          format("  {0}{1}{2}{3}{4}\n",
                 ansi.getBold(),                                           // 1
                 command.name,                                             // 2
                 ansi.getReset(),                                          // 3
                 string(max_option_width - 2 - command.name.size(), ' '),  // 4
                 description[0]));
    } else {
      ret.append(format("  {0}{1}{2}\n", ansi.getBold(), command.name,
                        ansi.getReset()));
      if (description[0] != "") {
        ret.append(
            format("{0}{1}\n", string(max_option_width, ' '), description[0]));
      }
    }
    for (size_t i = 1; i < description.size(); i++) {
      ret.append(
          format("{0}{1}\n", string(max_option_width, ' '), description[i]));
    }
  }
  return ret;
}

constexpr auto createOptionsSection(const auto& ansi, const auto& help_info) {
  string ret;

  ret.push_back('\n');
  for (const auto& option : help_info) {
    auto description = splitStringView(option.description, '\n');
    string option_string = format(
        "{0}{1} --{2}{3} {4}",
        ansi.getBold(),                                                 // 0
        (option.shortName == '\0') ? "   "                              //
                                   : format("-{},", option.shortName),  // 1
        option.name,                                                    // 2
        ansi.getReset(),                                                // 3
        option.typeName                                                 // 4
    );

    if (option_string.size() - ansi.getBold().size() - ansi.getReset().size() <
            max_option_width and
        description[0] != "") {
      ret.append(format(
          "  {0}{1}{2}\n",
          option_string,  // 1
          string(max_option_width - option_string.size() +
                     ansi.getBold().size() + ansi.getReset().size() - 2,
                 ' '),    // 3
          description[0]  // 4
          ));
    } else {
      ret.append(format("  {0}{1}{2}\n",
                        ansi.getBold(),  // 0
                        option_string,   // 1
                        ansi.getReset()  // 2
                        ));
      if (description[0] != "") {
        ret.append(
            format("{0}{1}\n", string(max_option_width, ' '), description[0]));
      }
    }
    for (size_t i = 1; i < description.size(); i++) {
      ret.append(format("{0}{1}\n",                     //
                        string(max_option_width, ' '),  // 0
                        description[i]                  // 1
                        ));
    }
  }
  return ret;
}

constexpr auto createPositionalArgumentSection(const auto& ansi,
                                               const auto& pargs_info) {
  string ret;
  ret.push_back('\n');
  for (const auto& i : pargs_info) {
    auto description = splitStringView(i.description, '\n');

    if (i.name.size() < max_option_width and description[0] != "") {
      ret.append(format("  {0}{1}{2}{3}{4}\n",
                        ansi.getBold(),                                     // 1
                        i.name,                                             // 2
                        ansi.getReset(),                                    // 3
                        string(max_option_width - 2 - i.name.size(), ' '),  // 4
                        description[0]));
    } else {
      ret.append(
          format("  {0}{1}{2}\n", ansi.getBold(), i.name, ansi.getReset()));
      if (description[0] != "") {
        ret.append(
            format("{0}{1}\n", string(max_option_width, ' '), description[0]));
      }
    }
    for (size_t i = 1; i < description.size(); i++) {
      ret.append(
          format("{0}{1}\n", string(max_option_width, ' '), description[i]));
    }
  }
  return ret;
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::formatHelp(
    bool no_color) const -> string {
  string ret;

  AnsiEscapeCode ansi((::isatty(1) != 0) and !no_color);

  assert(this->info_);  // this->info_ cannot be nullptr

  vector<ArgInfo> help_info;
  if constexpr (is_same_v<HArg, void>) {
    help_info = HelpGenerator<Args>();
  } else {
    help_info = HelpGenerator<tuple_append_t<Args, HArg>>();
  }
  vector<ArgInfo> pargs_info = HelpGenerator<PArgs>();

  auto sub_commands = SubParserInfo(subParsers);

  if (this->info_->help) {
    ret.append(this->info_->help.value());
    ret.push_back('\n');
  } else {
    // Description Section
    if (this->info_->description) {
      ret.append(this->info_->description.value());
      ret.push_back('\n');
    }

    // Usage Section
    ret.push_back('\n');
    ret.append(ansi.getBoldUnderline() + "Usage:" + ansi.getReset() + "\n");
    ret.append("  ");

    ret.append(this->info_->usage.value_or(
        createUsageSection(this->info_->program_name.value_or("no_name"),
                           help_info, pargs_info, sub_commands)));

    // Subcommand Section
    if constexpr (!is_same_v<SubParsers, tuple<>>) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() + "Subcommands:" + ansi.getReset());
      ret.append(this->info_->subcommand_help.value_or(
          createSubcommandSection(ansi, sub_commands)));
    }

    if constexpr (!is_same_v<PArgs, tuple<>>) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() +
                 "Positional Argument:" + ansi.getReset());
      ret.append(this->info_->positional_argument_help.value_or(
          createPositionalArgumentSection(ansi, pargs_info)));
    }

    // Options section
    if (!help_info.empty()) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() + "Options:" + ansi.getReset());
      ret.append(this->info_->options_help.value_or(
          createOptionsSection(ansi, help_info)));
    }
  }

  return ret;
}

}  // namespace Argo

// generator end here
