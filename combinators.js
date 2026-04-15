'use strict';

// Inspired by
// Combinatory Logic and Combinators in Array Languages
// by Conor Hoekstra
//
// Modeled on combinators.scm.

// --- Test harness ------------------------------------------------------

function deepEqual(a, b) {
    if (a === b) return true;
    if (Array.isArray(a) && Array.isArray(b)) {
        if (a.length !== b.length) return false;
        for (let i = 0; i < a.length; ++i) {
            if (!deepEqual(a[i], b[i])) return false;
        }
        return true;
    }
    return false;
}

function format(value) {
    if (Array.isArray(value)) return '[' + value.map(format).join(', ') + ']';
    if (typeof value === 'string') return JSON.stringify(value);
    return String(value);
}

function test(exprStr, result, expected) {
    const passed = deepEqual(result, expected);
    console.log('[' + (passed ? 'PASS' : 'FAIL') + '] ' + exprStr
        + ': ' + format(result));
}

// --- Combinators -------------------------------------------------------

const I = x => x;
// K mirrors the scheme case-lambda: called with one argument it returns a
// function that ignores its argument; called with two it returns the first.
const K = (x, ...rest) => rest.length > 0 ? x : (..._) => x;
const S = (f, g) => x => f(x, g(x));
const B = (f, g) => (...args) => f(g(...args));
const C = f => (x, y) => f(y, x);
const W = f => x => f(x, x);
const PSI = (f, g) => (x, y) => f(g(x), g(y));
const PHI = (f, g, h) => (...args) => f(g(...args), h(...args));
const D = (f, g) => (x, y) => f(x, g(y));
const D2 = (f, g, h) => (x, y) => f(g(x), h(y));

// --- Friendlier aliases ------------------------------------------------

const identity = I;
const konst = K; // `const` is a reserved word
const hook = S;
const compose = B;
const flip = C;
const dup = W;
const on = PSI;
const fork = PHI;

// --- Helpers -----------------------------------------------------------

const add = (a, b) => a + b;
const sub = (a, b) => a - b;
const mul = (a, b) => a * b;
const div = (a, b) => a / b;
const equal = (a, b) => deepEqual(a, b);
const length = xs => xs.length;

const sum = xs => xs.reduce((a, b) => a + b, 0);
const stringReverse = s => [...s].reverse().join('');
const stringSort = s => [...s].sort().join('');
const intersect = (xs, ys) => xs.filter(x => ys.some(y => deepEqual(x, y)));
const stringAppend = (a, b) => a + b;
const stringUpcase = s => s.toUpperCase();
const stringDowncase = s => s.toLowerCase();
const findSubstring = (haystack, needle) => haystack.indexOf(needle);

// --- Examples ----------------------------------------------------------

test('S(K, K)(10)', S(K, K)(10), 10);

const avg = PHI(div, sum, length);
test('avg([1,2,3,4])', avg([1, 2, 3, 4]), 2.5);

const plusOrMinus = PHI((a, b) => [a, b], add, sub);
test('plusOrMinus(10, 5)', plusOrMinus(10, 5), [15, 5]);

const absoluteDifference = B(Math.abs, sub);
test('absoluteDifference(10, 7)', absoluteDifference(10, 7), 3);
test('absoluteDifference(7, 10)', absoluteDifference(7, 10), 3);

const palindrome1 = PHI(equal, stringReverse, I);
const palindrome2 = S(equal, stringReverse);
test('palindrome1("tacocat")', palindrome1('tacocat'), true);
test('palindrome2("tacocat")', palindrome2('tacocat'), true);
test('palindrome1("tacodog")', palindrome1('tacodog'), false);
test('palindrome2("tacodog")', palindrome2('tacodog'), false);

const anagram = PSI(equal, stringSort);
test('anagram("owls", "slow")', anagram('owls', 'slow'), true);
test('anagram("cats", "dogs")', anagram('cats', 'dogs'), false);

const disjoint = B(xs => xs.length === 0, intersect);
test('disjoint([1,2], [3,4,5])', disjoint([1, 2], [3, 4, 5]), true);
test('disjoint([2,3], [3,4,5])', disjoint([2, 3], [3, 4, 5]), false);

// prefixOf: (PHI = (x, y) => findSubstring(y, x) === 0)
// Mirrors the scheme version, which uses PHI with K for a constant and C to
// flip findSubstring's arguments so the prefix is the needle.
const prefixOf = PHI((a, b) => a === b, K(0), C(findSubstring));
test('prefixOf("cat", "catch")', prefixOf('cat', 'catch'), true);
test('prefixOf("dog", "catch")', prefixOf('dog', 'catch'), false);

const square = W(mul);
test('square(5)', square(5), 25);

test('D(stringAppend, stringUpcase)("hello ", "world")',
    D(stringAppend, stringUpcase)('hello ', 'world'), 'hello WORLD');

test('D2(stringAppend, stringUpcase, stringDowncase)("hello ", "WORLD")',
    D2(stringAppend, stringUpcase, stringDowncase)('hello ', 'WORLD'),
    'HELLO world');
