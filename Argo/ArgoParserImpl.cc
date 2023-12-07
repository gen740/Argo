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
import :MetaChecker;

// generator start here

namespace Argo {

inline auto splitStringView(std::string_view str,
                            char delimeter) -> std::vector<std::string_view> {
  std::vector<std::string_view> ret;
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
    std::string_view key, std::span<std::string_view> val) const -> void {
  if constexpr (!std::is_same_v<HArg, void>) {
    if (key == HArg::name) {
      std::println("{}", formatHelp());
      std::exit(0);
    }
  }
  Assigner<Args, PArgs>::assign(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::setArg(
    std::span<char> key, std::span<std::string_view> val) const -> void {
  if constexpr (!std::is_same_v<HArg, void>) {
    for (const auto& i : key) {
      if constexpr (HArg::name.shortName != '\0') {
        if (i == HArg::name.shortName) {
          std::println("{}", formatHelp());
          std::exit(0);
        }
      }
    }
  }
  Assigner<Args, PArgs>::assign(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::parse(int argc,
                                                      char* argv[]) -> void {
  if (this->parsed_) [[unlikely]] {
    throw ParseError("Cannot parse twice");
  }
  auto assigned_keys = AssignChecker<Args>::check();
  if (!assigned_keys.empty()) [[unlikely]] {
    throw ParseError(std::format("keys {} already assigned", assigned_keys));
  }

  int64_t subcmd_found_idx = -1;
  int64_t cmd_end_pos = argc;
  if constexpr (!std::is_same_v<SubParsers, std::tuple<>>) {
    for (int i = argc - 1; i > 0; i--) {
      subcmd_found_idx = ParserIndex(subParsers, argv[i]);
      if (subcmd_found_idx != -1) {
        cmd_end_pos = i;
        break;
      }
    }
  }

  std::string_view key{};
  std::vector<char> short_keys{};
  short_keys.reserve(10);
  std::vector<std::string_view> values{};
  values.reserve(10);

  assert(this->info_);  // this->info_ cannot be nullptr
  if (!this->info_->program_name) {
    this->info_->program_name = std::string_view(argv[0]);
  }

  for (int i = 1; i < cmd_end_pos; i++) {
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
          if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
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
        if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
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
      if constexpr (std::is_same_v<PArgs, std::tuple<>>) {
        if (key.empty() && short_keys.empty()) {
          throw InvalidArgument(std::format("No keys specified"));
        }
      }
      values.push_back(arg);

      if (i == cmd_end_pos - 1) {
        if (!key.empty()) {
          this->setArg(key, values);
        } else if (!short_keys.empty()) {
          this->setArg(short_keys, values);
        } else {
          if constexpr (std::is_same_v<PArgs, std::tuple<>>) {
            throw InvalidArgument(std::format("No keys specified"));
          } else {
            this->setArg(key, values);
          }
        }
      }
    }
  }
  auto required_keys = RequiredChecker<Args>::check();
  if (!required_keys.empty()) {
    throw InvalidArgument(std::format("Requried {}", required_keys));
  }
  if (subcmd_found_idx != -1) {
    MetaParse(subParsers, subcmd_found_idx, argc - cmd_end_pos,
              &argv[cmd_end_pos]);
  }
  this->parsed_ = true;
}

struct AnsiEscapeCode {
  bool isEnabled;

  std::string bold = "\x1B[1m";
  std::string underline = "\x1B[4m";
  std::string reset = "\x1B[0m";

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
  std::string ret;
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
  std::string ret;
  size_t max_command_length = 0;
  for (const auto& command : sub_commands) {
    if (max_command_length < command.name.size()) {
      max_command_length = command.name.size();
    }
  }
  for (const auto& command : sub_commands) {
    ret.push_back('\n');
    auto description = splitStringView(command.description, '\n');

    ret.append(
        std::format("  {}{} {}{} {}", ansi.getBold(), command.name,
                    std::string(max_command_length - command.name.size(), ' '),
                    ansi.getReset(), description[0]));
    for (size_t i = 1; i < description.size(); i++) {
      ret.push_back('\n');
      ret.append(std::format("    {}{}",  //
                             std::string(max_command_length, ' '),
                             description[i]));
    }

    // erase trailing spaces
    auto pos = ret.find_last_not_of(' ');
    ret = ret.substr(0, pos + 1);
  }

  ret.push_back('\n');
  return ret;
}

inline auto createOptionsSection(const auto& ansi, const auto& help_info) {
  std::string ret;
  size_t max_name_len = 0;
  for (const auto& option : help_info) {
    if (max_name_len < option.name.size() + option.typeName.size()) {
      max_name_len = option.name.size() + option.typeName.size();
    }
  }
  for (const auto& option : help_info) {
    ret.push_back('\n');
    auto description = splitStringView(option.description, '\n');

    ret.append(std::format(
        "  {}{} --{} {}{}{}  {}",
        (option.shortName == '\0') ? "   "
                                   : std::format("-{},", option.shortName),
        ansi.getBold(), option.name, ansi.getReset(), option.typeName,
        std::string(max_name_len - option.name.size() - option.typeName.size(),
                    ' '),
        description[0]));
    for (size_t i = 1; i < description.size(); i++) {
      ret.push_back('\n');
      ret.append(std::format("      {}     {}", std::string(max_name_len, ' '),
                             description[i]));
    }

    auto pos = ret.find_last_not_of(' ');
    ret = ret.substr(0, pos + 1);
  }
  return ret;
}

template <ArgType PArgs>
auto createPositionalArgumentSection(const auto& ansi) {
  std::string ret;

  ret.push_back('\n');
  auto desc = splitStringView(PArgs::description, '\n');
  ret.append(std::format("  {}{}{}  {}", ansi.getBold(),
                         std::string_view(PArgs::name), ansi.getReset(),
                         desc[0]));

  for (size_t i = 1; i < desc.size(); i++) {
    ret.push_back('\n');
    ret.append(std::format("{}{}", std::string(8, ' '), desc[i]));
  }

  ret.push_back('\n');
  return ret;
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::formatHelp(bool no_color) const
    -> std::string {
  std::string ret;

  AnsiEscapeCode ansi((::isatty(1) != 0) and !no_color);

  assert(this->info_);  // this->info_ cannot be nullptr

  auto help_info = HelpGenerator<tuple_append_t<Args, HArg>>::generate();
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
    if constexpr (!std::is_same_v<SubParsers, std::tuple<>>) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() + "Subcommands:" + ansi.getReset());
      ret.append(this->info_->subcommand_help.value_or(
          createSubcommandSection(ansi, sub_commands)));
    }

    if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
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
