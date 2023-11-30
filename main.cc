import Argo;

#include <print>
#include <tuple>

template <typename... Args>
std::tuple<size_t, char**> createArgcArgv(Args... args) {
  const size_t N = sizeof...(Args);
  char** array = new char*[N];
  size_t i = 0;
  (..., (array[i++] = strdup(args)));
  return std::make_tuple(N, array);
}

auto [argc, argv] = createArgcArgv("./main", "--arg1", "23", "-h");

template <size_t N>
struct cstring {
  char data[N];

  consteval cstring(char const* str) {
    for (auto i = 0zu; i < N; ++i) {
      data[i] = str[i];
    }
  }
};

template <size_t N>
cstring(const char (&)[N]) -> cstring<N>;

template <std::size_t N>
struct ParserID {
  int idInt = 0;
  const char idName[N];

  ParserID(int id) : idInt(id){};

  ParserID(const char (&id)[N + 1]) {
    for (int i = 0; i < N; i++) {
      idName[i] = id[i];
    }
  };
};

ParserID(int) -> ParserID<0>;

template <std::size_t N>
ParserID(const char (&)[N]) -> ParserID<N - 1>;

auto main() -> int {
  [[maybe_unused]] auto a = ParserID(0);

  auto argo = Argo::Parser<"HoegHoge">();

  // auto parser = argo  //
  //                   .addArg<key("dummy", 'e'), int>(Argo::withDescription("Hello\nWorld!"))
  //                   .addArg<"arg1", int>(Argo::withDescription("Hello\nWorld!"))
  //                   .addHelp();
  auto parser = argo  //
                    .addArg<"arg1,o", int>();

  parser.parse(argc, argv);
  //
  // std::println("{}", parser.getArg<key("arg1")>());
  // // std::println("{}", parser.formatHelp());

  return 0;
}
