#include <algorithm>
#include <format>
#include <functional>
#include <numeric>
#include <print>
#include <ranges>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std::literals;

template<typename... Ts>
struct std::formatter<std::tuple<Ts...>> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const std::tuple<Ts...>& t, std::format_context& ctx) const {
        auto out = ctx.out();
        *out++ = '(';
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ([&] {
                if constexpr (Is > 0) out = std::format_to(out, ", ");
                out = std::format_to(out, "{}", std::get<Is>(t));
            }(), ...);
        }(std::index_sequence_for<Ts...>{});
        *out++ = ')';
        return out;
    }
};

template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

#define TEST(expr, expected) \
    do { \
        auto result = (expr); \
        auto expected_value = (expected); \
        bool passed = (result == (expected_value)); \
        std::string status = passed ? "PASS" : "FAIL"; \
        std::string op = passed ? "==" : "!="; \
        std::println("[{}] {}: {} {} {}", status, #expr, result, op, expected_value); \
    } while (0)

// STATIC_TEST proves the expression is a core constant expression. If the
// chain pipes through any non-constexpr function, the build fails and the
// compiler points at the offending call. Works at any -O level.
#define STATIC_TEST(expr, expected) static_assert((expr) == (expected))

#if 0
// Basic...
auto I = [](auto x) { return x; };
// Better...
auto I = [](auto&& x) -> decltype(auto) { return std::forward<decltype(x)>(x); };
#else
// Best...
constexpr auto I = std::identity{};
#endif
constexpr auto K = overloads {
    [](auto x) { return [=](auto...) { return x; }; },
    [](auto x, auto...) { return x; }
};
constexpr auto S = [](auto f, auto g) { return [=](auto x) { return f(x, g(x)); }; };
constexpr auto B = [](auto f, auto g) { return [=](auto... args) { return f(g(args...)); }; };
constexpr auto C = [](auto f) { return [=](auto x, auto y) { return f(y, x); }; };
constexpr auto W = [](auto f) { return [=](auto x) { return f(x, x); }; };
constexpr auto PSI = [](auto f, auto g) { return [=](auto x, auto y) { return f(g(x), g(y)); }; };
constexpr auto PHI = [](auto f, auto g, auto h) { return [=](auto... x) { return f(g(x...), h(x...)); }; };
constexpr auto D = [](auto f, auto g) { return [=](auto x, auto y) { return f(x, g(y)); }; };
constexpr auto D2 = [](auto f, auto g, auto h) { return [=](auto x, auto y) { return f(g(x), h(y)); }; };

int main()
{
    TEST(S(K, K)(42), 42);

    constexpr auto sum = [](auto ns) { return std::ranges::fold_left_first(ns, std::plus<int>{}).value_or(0); };
    constexpr auto avg = PHI(std::divides<double>{}, sum, std::ranges::size);
    TEST(avg(std::vector{1, 2, 3, 4}), 5. / 2.);

    constexpr auto make_tuple = [](auto... args) { return std::make_tuple(args...); };
    constexpr auto plus_or_minus = PHI(make_tuple, std::plus<int>{}, std::minus<int>{});
    TEST(plus_or_minus(10, 5), make_tuple(15, 5));

    constexpr auto abs = [](auto x) { return std::abs(x); };
    constexpr auto abs_diff = B(abs, std::minus<int>{});
    TEST(abs_diff(10, 7), 3);
    TEST(abs_diff(7, 10), 3);

    constexpr auto string_reverse = [](std::string_view sv) { return std::string(sv.rbegin(), sv.rend()); };
    constexpr auto palindrome1 = PHI(std::equal_to<std::string>{}, string_reverse, I);
    TEST(palindrome1("tacocat"s), true);
    TEST(palindrome1("tacodog"s), false);
    constexpr auto palindrome2 = S(std::equal_to<std::string>{}, string_reverse);
    TEST(palindrome2("tacocat"s), true);
    TEST(palindrome2("tacodog"s), false);

    constexpr auto string_sort = [](std::string_view sv) {  std::string s{sv}; std::ranges::sort(s); return s; };
    constexpr auto anagram = PSI(std::equal_to<std::string>{}, string_sort);
    TEST(anagram("owls"sv, "slow"sv), true);
    TEST(anagram("cats"sv, "dogs"sv), false);

    constexpr auto intersect = [](auto xs, auto ys) {
        // We'll be functional and not modify the input.
        // We could check the type of the range and, if it is a forward or better,
        // we could use std::ranges::is_sorted before copying and sorting it.
        // Full support for std::ranges::to isn't in GCC 14.
        using xs_value_type = std::remove_reference_t<decltype(*xs.begin())>;
        using ys_value_type = std::remove_reference_t<decltype(*ys.begin())>;
        using value_type = std::common_type_t<xs_value_type, ys_value_type>;
        std::vector<xs_value_type> sorted_xs(xs.begin(), xs.end());
        std::vector<ys_value_type> sorted_ys(ys.begin(), ys.end());
        std::ranges::sort(sorted_xs);
        std::ranges::sort(sorted_ys);
        std::vector<value_type> result;
        std::ranges::set_intersection(sorted_xs, sorted_ys, std::back_inserter(result));
        return result;
    };
    constexpr auto is_disjoint = B(std::ranges::empty, intersect);
    TEST(is_disjoint(std::vector{1, 2}, std::vector{3, 4, 5}), true);
    TEST(is_disjoint(std::vector{2, 3}, std::vector{3, 4, 5}), false);

    // Std::string_view::starts_with exists, but we're here to demo combinators.
    constexpr auto find_substring = [](std::string_view sv, std::string_view prefix) { return sv.find(prefix); };
    constexpr auto is_prefix_of = PHI(std::equal_to{}, K(0), C(find_substring));
    TEST(is_prefix_of("cat"sv, "catch"sv), true);
    TEST(is_prefix_of("dog"sv, "catch"sv), false);

    constexpr auto square = W(std::multiplies<>{});
    TEST(square(5), 25);

    // ::toupper and ::tolower are not constexpr, so we provide these.
    constexpr auto ascii_to_upper = [](char c) { return ((c >= 'a') and (c <= 'z')) ? static_cast<char>(c - ('a' - 'A')) : c; };
    constexpr auto ascii_to_lower = [](char c) { return ((c >= 'A') and (c <= 'Z')) ? static_cast<char>(c + ('a' - 'A')) : c; };

    constexpr auto string_append = [](std::string_view a, std::string_view b) { return std::string(a).append(b); };
    // I might have used a sv | std::ranges::transform(...) | std::ranges::to<std::string> if available.
    constexpr auto string_upcase = [](std::string_view sv) { std::string s{sv}; std::ranges::transform(s, s.begin(), ascii_to_upper); return s; };
    constexpr auto string_downcase = [](std::string_view sv) { std::string s{sv}; std::ranges::transform(s, s.begin(), ascii_to_lower); return s; };
    TEST(D(string_append, string_upcase)("hello "sv, "world"sv), "hello WORLD");
    TEST(D2(string_append, string_upcase, string_downcase)("hello "sv, "WORLD"sv), "HELLO world");

    // --- Practical examples: where combinators actually earn their keep ---

    // PSI is Haskell's `on`: build a comparator from a projection.
    // Ranges algorithms take a projection parameter that covers this
    // for the algorithm case - std::ranges::sort(words, std::less<>{},
    // string_downcase) works fine. But *containers* - std::set,
    // std::map, std::priority_queue - take a Compare *type*, not a
    // projection, and that's where PSI earns its keep: it gives you a
    // ready-made Compare without writing a boilerplate comparator struct.
    auto ci_less = PSI(std::less<std::string>{}, string_downcase);
    std::set<std::string, decltype(ci_less)> words(ci_less);
    words.insert("banana");
    words.insert("Apple");
    words.insert("APPLE");  // dupe of "Apple" under ci_less - not inserted
    words.insert("cherry");
    TEST(words.size(), 3uz);
    TEST(*words.begin(), "Apple"s);  // set ordered case-insensitively

    // Same story, sorting records by a field: ranges::sort would take
    // a projection, but std::set wants a Compare type. PSI + std::mem_fn
    // orders a set of records by a data member without a boilerplate
    // comparator struct.
    struct employee { std::string_view name; int salary; };
    auto by_salary = PSI(std::less<>{}, std::mem_fn(&employee::salary));
    std::set<employee, decltype(by_salary)> payroll(by_salary);
    payroll.insert({"alice", 50});
    payroll.insert({"bob",   80});
    payroll.insert({"carol", 65});
    TEST(payroll.begin()->name,  "alice"sv);  // lowest salary first
    TEST(payroll.rbegin()->name, "bob"sv);    // highest salary last

    // PHI fuses two reductions over the same input into one combined
    // result. `avg` above is sum/size; `range_span` is max-min. Pattern
    // shows up constantly in numerical and statistical code.
    constexpr auto range_span = PHI(std::minus<int>{}, std::ranges::max, std::ranges::min);
    TEST(range_span(std::vector{3, 1, 4, 1, 5, 9, 2, 6}), 8);

    // Pointfree compound predicates: combine two unary predicates with
    // logical_and via PHI, then hand the result straight to a ranges
    // algorithm. No named helper, no captured-state lambda.
    constexpr auto is_even     = [](int n) { return n % 2 == 0; };
    constexpr auto is_positive = [](int n) { return n > 0; };
    constexpr auto even_and_positive = PHI(std::logical_and<>{}, is_even, is_positive);
    TEST(std::ranges::count_if(std::vector{-2, -1, 0, 1, 2, 3, 4}, even_and_positive), 2);

    // Compile-time evaluation checks. Each STATIC_TEST forces the expression
    // into a constant-evaluated context; if it fails to compile, the chain
    // contains a non-constexpr call.
    STATIC_TEST(S(K, K)(42), 42);
    STATIC_TEST(avg(std::vector{1, 2, 3, 4}), 5. / 2.);
    STATIC_TEST(plus_or_minus(10, 5), std::make_tuple(15, 5));
    STATIC_TEST(abs_diff(10, 7), 3);
    STATIC_TEST(abs_diff(7, 10), 3);
    STATIC_TEST(palindrome1("tacocat"s), true);
    STATIC_TEST(palindrome1("tacodog"s), false);
    STATIC_TEST(palindrome2("tacocat"s), true);
    STATIC_TEST(palindrome2("tacodog"s), false);
    STATIC_TEST(anagram("owls"sv, "slow"sv), true);
    STATIC_TEST(anagram("cats"sv, "dogs"sv), false);
    STATIC_TEST(is_disjoint(std::vector{1, 2}, std::vector{3, 4, 5}), true);
    STATIC_TEST(is_disjoint(std::vector{2, 3}, std::vector{3, 4, 5}), false);
    STATIC_TEST(is_prefix_of("cat"sv, "catch"sv), true);
    STATIC_TEST(is_prefix_of("dog"sv, "catch"sv), false);
    STATIC_TEST(square(5), 25);
    STATIC_TEST(D(string_append, string_upcase)("hello "sv, "world"sv), "hello WORLD"s);
    STATIC_TEST(D2(string_append, string_upcase, string_downcase)("hello "sv, "WORLD"sv), "HELLO world"s);
}
