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

//TODO: Make B variadic.
template <typename F, typename G>
struct B_helper {
    B_helper(F f, G g) : f(f), g(g) {}
    template <typename... Args>
    auto operator()(Args&&... args) const {
        // Should std::forward.
        return f(g(args...));
    }
private:
    F f;
    G g;
};
auto B = [](auto f, auto g) { return B_helper(f, g); };

auto C = [](auto f) { return [=](auto x, auto y) { return f(y, x); }; };
auto W = [](auto f) { return [=](auto x) { return f(x, x); }; };
auto PSI = [](auto f, auto g) { return [=](auto x, auto y) { return f(g(x), g(y)); }; };

#if 0
auto PHI = [](auto f, auto g, auto h) {
    return [=](auto... x) {
        return f(g(x...), h(x...));
    };
};
#else
template <typename F, typename G, typename H>
struct PHI_helper {
    PHI_helper(F f, G g, H h) : f(f), g(g), h(h) {}
    template <typename... Args>
    auto operator()(Args&&... args) const {
        // Should std::forward.
        return f(g(args...), h(args...));
    }
private:
    F f;
    G g;
    H h;
};
auto PHI = [](auto f, auto g, auto h) {
    return PHI_helper(f, g, h);
};
#endif

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
