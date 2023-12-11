module;

#include <cassert>

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

inline auto splitStringView(string_view str, char delimeter)
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
auto Parser<ID, Args, PArgs, HArg, SubParsers>::resetArgs() -> void {
  ValueReset<Args>();
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::setArg(
    string_view key, span<string_view> val) const -> void {
  if constexpr (!is_same_v<HArg, void>) {
    if (key == HArg::name) {
      std::cout << formatHelp() << '\n';
      exit(0);
    }
  }
  Assigner<Args, PArgs>(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::setArg(
    span<char> key, span<string_view> val) const -> void {
  if constexpr (!is_same_v<HArg, void>) {
    for (const auto& i : key) {
      if constexpr (HArg::name.shortName != '\0') {
        if (i == HArg::name.shortName) {
          std::cout << formatHelp() << '\n';
          exit(0);
        }
      }
    }
  }
  Assigner<Args, PArgs>(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::parse(int argc, char* argv[])
    -> void {
  if (this->parsed_) [[unlikely]] {
    throw ParseError("Cannot parse twice");
  }
  auto assigned_keys = vector<string_view>();
  tuple_type_visit<decltype(tuple_cat(declval<Args>(), declval<PArgs>()))>(
      [&assigned_keys]<class T>(T) {
        if (T::type::assigned) {
          assigned_keys.push_back(string_view(T::type::name));
        }
      });
  if (!assigned_keys.empty()) [[unlikely]] {
    throw ParseError(format("keys {} already assigned", assigned_keys));
  }

  // Search for subcommand
  int64_t subcmd_found_idx = -1;
  int64_t cmd_end_pos = argc;
  if constexpr (!is_same_v<SubParsers, tuple<>>) {
    for (int i = argc - 1; i > 0; i--) {
      subcmd_found_idx = ParserIndex(subParsers, argv[i]);
      if (subcmd_found_idx != -1) {
        cmd_end_pos = i;
        break;
      }
    }
  }

  string_view key{};
  vector<char> short_keys{};
  short_keys.reserve(10);
  vector<string_view> values{};
  values.reserve(10);

  assert(this->info_);  // this->info_ cannot be nullptr
  if (!this->info_->program_name) {
    this->info_->program_name = string_view(argv[0]);
  }

  for (int i = 1; i < cmd_end_pos; i++) {
    string_view arg = argv[i];
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
          if constexpr (!is_same_v<PArgs, tuple<>>) {
            this->setArg(key, values);
            values.clear();
          }
        }
        if (arg.contains('=')) {
          auto equal_pos = arg.find('=');
          auto value = vector<string_view>{arg.substr(equal_pos + 1)};
          this->setArg(arg.substr(2, equal_pos - 2), value);
          continue;
        }
        key = arg.substr(2);
        if (i == (cmd_end_pos - 1)) {
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
        if constexpr (!is_same_v<PArgs, tuple<>>) {
          this->setArg(key, values);
          values.clear();
        }
      }
      if (key.empty() && short_keys.empty()) {
        for (const auto& j : arg.substr(1)) {
          short_keys.push_back(j);
        }
        if (i == (cmd_end_pos - 1)) {
          this->setArg(short_keys, {});
        }
        continue;
      }
    } else {
      if constexpr (is_same_v<PArgs, tuple<>>) {
        if (key.empty() && short_keys.empty()) {
          throw InvalidArgument("No keys specified");
        }
      }
      values.push_back(arg);

      if (i == cmd_end_pos - 1) {
        if (!key.empty()) {
          this->setArg(key, values);
        } else if (!short_keys.empty()) {
          this->setArg(short_keys, values);
        } else {
          if constexpr (is_same_v<PArgs, tuple<>>) {
            throw InvalidArgument("No keys specified");
          } else {
            this->setArg(key, values);
          }
        }
      }
    }
  }

  auto required_keys = vector<string_view>();
  tuple_type_visit<decltype(tuple_cat(declval<Args>(), declval<PArgs>()))>(
      [&required_keys]<class T>(T) {
        if ((T::type::required && !T::type::assigned)) {
          required_keys.push_back(string_view(T::type::name));
        }
      });

  if (!required_keys.empty()) {
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

inline auto createUsageSection(const auto& program_name,
                               [[maybe_unused]] const auto& help_info,
                               const auto& sub_commands) {
  string ret;
  ret.append(program_name);

  // for (const auto& i : help_info) {
  //   ret.push_back(' ');
  //   if (!i.required) {
  //     ret.push_back('[');
  //   }
  //   ret.append("--");
  //   ret.append(i.name);
  //   if (!i.typeName.empty()) {
  //     ret.push_back(' ');
  //     ret.append(i.typeName);
  //   }
  //   if (!i.required) {
  //     ret.push_back(']');
  //   }
  // }

  ret.append(" [options...]");

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

inline auto createSubcommandSection(const auto& ansi,
                                    const auto& sub_commands) {
  string ret;
  size_t max_command_length = 0;
  for (const auto& command : sub_commands) {
    if (max_command_length < command.name.size()) {
      max_command_length = command.name.size();
    }
  }
  for (const auto& command : sub_commands) {
    ret.push_back('\n');
    auto description = splitStringView(command.description, '\n');

    ret.append(format("  {}{} {}{} {}", ansi.getBold(), command.name,
                      string(max_command_length - command.name.size(), ' '),
                      ansi.getReset(), description[0]));
    for (size_t i = 1; i < description.size(); i++) {
      ret.push_back('\n');
      ret.append(format("    {}{}",  //
                        string(max_command_length, ' '), description[i]));
    }

    // erase trailing spaces
    auto pos = ret.find_last_not_of(' ');
    ret = ret.substr(0, pos + 1);
  }

  ret.push_back('\n');
  return ret;
}

inline auto createOptionsSection(const auto& ansi, const auto& help_info) {
  string ret;
  size_t max_name_len = 0;
  for (const auto& option : help_info) {
    if (max_name_len < option.name.size() + option.typeName.size()) {
      max_name_len = option.name.size() + option.typeName.size();
    }
  }
  for (const auto& option : help_info) {
    ret.push_back('\n');
    auto description = splitStringView(option.description, '\n');

    ret.append(format(
        "  {}{} --{} {}{}{}  {}",
        (option.shortName == '\0') ? "   " : format("-{},", option.shortName),
        ansi.getBold(), option.name, ansi.getReset(), option.typeName,
        string(max_name_len - option.name.size() - option.typeName.size(), ' '),
        description[0]));
    for (size_t i = 1; i < description.size(); i++) {
      ret.push_back('\n');
      ret.append(
          format("      {}     {}", string(max_name_len, ' '), description[i]));
    }

    auto pos = ret.find_last_not_of(' ');
    ret = ret.substr(0, pos + 1);
  }
  return ret;
}

template <class PArgs>
auto createPositionalArgumentSection(const auto& ansi) {
  string ret;

  vector<ArgInfo> info = HelpGenerator<PArgs>::generate();

  for (const auto& i : info) {
    ret.push_back('\n');
    auto desc = splitStringView(i.description, '\n');
    ret.append(format("  {}{}{}  {}", ansi.getBold(), string_view(i.name),
                      ansi.getReset(), desc[0]));
    for (size_t i = 1; i < desc.size(); i++) {
      ret.push_back('\n');
      ret.append(format("{}{}", string(8, ' '), desc[i]));
    }
  }

  ret.push_back('\n');
  return ret;
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::formatHelp(bool no_color) const
    -> string {
  string ret;

  AnsiEscapeCode ansi((::isatty(1) != 0) and !no_color);

  assert(this->info_);  // this->info_ cannot be nullptr

  vector<ArgInfo> help_info;
  if constexpr (is_same_v<HArg, void>) {
    help_info = HelpGenerator<Args>::generate();
  } else {
    help_info = HelpGenerator<tuple_append_t<Args, HArg>>::generate();
  }

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
    ret.append(ansi.getBoldUnderline() + "USAGE:" + ansi.getReset() + "\n");
    ret.append("  ");

    ret.append(this->info_->usage.value_or(
        createUsageSection(this->info_->program_name.value_or("no_name"),
                           help_info, sub_commands)));

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
          createPositionalArgumentSection<PArgs>(ansi)));
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
