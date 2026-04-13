#include <functional>
#include <numeric>
#include <string>
#include <vector>
#include <print>

#define TEST(expr, expected) \
    do { \
        auto result = (expr); \
        bool passed = (result == (expected)); \
        std::string status = passed ? "PASS" : "FAIL"; \
        std::string op = passed ? "==" : "!="; \
        std::print("[{}] {}: {} {} {}\n", status, #expr, result, op, expected); \
    } while (0)

auto I = [](auto x) { return x; };
auto K = [](auto x) { return [=](auto y) { return x; }; };
auto S = [](auto f, auto g) { return [=](auto x) { return f(x)(g(x)); }; };
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

    auto sum = [](auto ns) { return std::accumulate(std::begin(ns), std::end(ns), 0); };
    auto size = [](auto ns) { return ns.size(); };
    auto avg = PHI(std::divides<double>{}, sum, size);
    TEST(avg(std::vector{1, 2, 3, 4}), 5. / 2.);

    auto make_tuple = [](auto... args) { return std::make_tuple(args...); };
    auto plus_or_minus = PHI(make_tuple, std::plus<int>{}, std::minus<int>{});
#if 0
    TEST(plus_or_minus(10, 5), make_tuple(15, 5));
#else
    auto [plus_result, minus_result] = plus_or_minus(10, 5);
    TEST(plus_result, 15);
    TEST(minus_result, 5);
#endif

    auto abs = [](auto x) { return std::abs(x); };
    auto abs_diff = B(abs, std::minus<int>{});
    TEST(abs_diff(10, 7), 3);
    TEST(abs_diff(7, 10), 3);
}
