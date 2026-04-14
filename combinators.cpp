#include <algorithm>
#include <format>
#include <functional>
#include <numeric>
#include <print>
#include <ranges>
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
        bool passed = (result == (expected)); \
        std::string status = passed ? "PASS" : "FAIL"; \
        std::string op = passed ? "==" : "!="; \
        std::println("[{}] {}: {} {} {}", status, #expr, result, op, expected); \
    } while (0)

#if 0
// Basic...
auto I = [](auto x) { return x; };
// Better...
auto I = [](auto&& x) -> decltype(auto) { return std::forward<decltype(x)>(x); };
#else
// Best...
auto I = std::identity{};
#endif
const auto K = overloads {
    [](auto x) { return [=](auto...) { return x; }; },
    [](auto x, auto...) { return x; }
};
auto S = [](auto f, auto g) { return [=](auto x) { return f(x, g(x)); }; };
auto B = [](auto f, auto g) { return [=](auto... args) { return f(g(args...)); }; };
auto C = [](auto f) { return [=](auto x, auto y) { return f(y, x); }; };
auto W = [](auto f) { return [=](auto x) { return f(x, x); }; };
auto PSI = [](auto f, auto g) { return [=](auto x, auto y) { return f(g(x), g(y)); }; };
auto PHI = [](auto f, auto g, auto h) { return [=](auto... x) { return f(g(x...), h(x...)); }; };
auto D = [](auto f, auto g) { return [=](auto x, auto y) { return f(x, g(y)); }; };
auto D2 = [](auto f, auto g, auto h) { return [=](auto x, auto y) { return f(g(x), h(y)); }; };

int main()
{
    TEST(S(K, K)(42), 42);

    auto sum = [](auto ns) { return std::ranges::fold_left_first(ns, std::plus<int>{}).value_or(0); };
    auto avg = PHI(std::divides<double>{}, sum, std::ranges::size);
    TEST(avg(std::vector{1, 2, 3, 4}), 5. / 2.);

    auto make_tuple = [](auto... args) { return std::make_tuple(args...); };
    auto plus_or_minus = PHI(make_tuple, std::plus<int>{}, std::minus<int>{});

    TEST(plus_or_minus(10, 5), make_tuple(15, 5));

    auto abs = [](auto x) { return std::abs(x); };
    auto abs_diff = B(abs, std::minus<int>{});
    TEST(abs_diff(10, 7), 3);
    TEST(abs_diff(7, 10), 3);

    auto string_reverse = [](std::string_view sv) { return std::string(sv.rbegin(), sv.rend()); };
    auto palindrome1 = PHI(std::equal_to<std::string>{}, string_reverse, I);
    TEST(palindrome1("tacocat"s), true);
    TEST(palindrome1("tacodog"s), false);
    auto palindrome2 = S(std::equal_to<std::string>{}, string_reverse);
    TEST(palindrome2("tacocat"s), true);
    TEST(palindrome2("tacodog"s), false);

    auto string_sort = [](std::string_view sv) {  std::string s{sv}; std::ranges::sort(s); return s; };
    auto anagram = PSI(std::equal_to<std::string>{}, string_sort);
    TEST(anagram("owls"sv, "slow"sv), true);
    TEST(anagram("cats"sv, "dogs"sv), false);

    auto intersect = [](auto xs, auto ys) {
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
    auto is_disjoint = B(std::ranges::empty, intersect);
    TEST(is_disjoint(std::vector{1, 2}, std::vector{3, 4, 5}), true);
    TEST(is_disjoint(std::vector{2, 3}, std::vector{3, 4, 5}), false);

    // Std::string_view::starts_with exists, but we're here to demo combinators.
    auto find_substring = [](std::string_view sv, std::string_view prefix) { return sv.find(prefix); };
    auto is_prefix_of = PHI(std::equal_to{}, K(0), C(find_substring));
    TEST(is_prefix_of("cat"sv, "catch"sv), true);
    TEST(is_prefix_of("dog"sv, "catch"sv), false);

    auto square = W(std::multiplies<>{});
    TEST(square(5), 25);

    auto string_append = [](std::string_view a, std::string_view b) { return std::string(a).append(b); };
    // I might have used a sv | std::ranges::transform(...) | std::ranges::to<std::string> if available.
    auto string_upcase = [](std::string_view sv) { std::string s{sv}; std::ranges::transform(s, s.begin(), ::toupper); return s; };
    auto string_downcase = [](std::string_view sv) { std::string s{sv}; std::ranges::transform(s, s.begin(), ::tolower); return s; };
    TEST(D(string_append, string_upcase)("hello "sv, "world"sv), "hello WORLD");
    TEST(D2(string_append, string_upcase, string_downcase)("hello "sv, "WORLD"sv), "HELLO world");
}
