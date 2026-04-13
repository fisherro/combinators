(import (scheme base)
        (scheme case-lambda)
        (scheme write)
        (srfi 1)
        (srfi 26)
        (srfi 95))

;;; Inspired by
;;; Combinatory Logic and Combinators in Array Languages
;;; by Conor Hoekstra

(define-syntax define-combinator
  (syntax-rules ()
    ((_ name (param ...) (arg ...) body)
     (define name
       (case-lambda
         ((param ...) (lambda (arg ...) body))
         ((param ... arg ...) ((name param ...) arg ...)))))
    ((_ name (param ...) rest body)
     (define name
       (case-lambda
         ((param ...) (lambda rest body))
         ((param ... . rest) (apply (name param ...) rest)))))))

(define-syntax show
  (syntax-rules ()
    ((_ expr)
     (begin
      (write 'expr)
      (display ": ")
      (display expr)
      (newline)))))

;; Combinators
(define (I x) x)
(define-combinator K (x) (y) x)
(define-combinator S (f g) (x) (f x (g x)))
(define-combinator B (f g) args (f (apply g args)))
(define-combinator C (f) (x y) (f y x))
(define-combinator W (f) (x) (f x x))
(define-combinator psi (f g) (x y) (f (g x) (g y)))
(define-combinator phi (f g h) args (g (apply f args) (apply h args)))
(define-combinator D (f g) (x y) (f x (g y)))
(define-combinator D2 (f g h) (x y) (g (f x) (h y)))

;; Helpers
(define (sum lyst) (apply + lyst))
(define string-reverse (B list->string (B reverse string->list)))
(define string-sort
  (B list->string (phi string->list sort (K char<?))))
(define intersect (cut lset-intersection equal? <> <>))

;; Examples
(define avg (phi sum / length))
(show (avg '(1 2 3 4)))

(define plusOrMinus (phi + list -))
(show (plusOrMinus 10 5))

(define absoluteDifference (B abs -))
(show (absoluteDifference 10 7))
(show (absoluteDifference 7 10))

(define isPalindrome1 (phi string-reverse equal? I))
(define isPalindrome2 (S equal? string-reverse))
(show (isPalindrome1 "tacocat"))
(show (isPalindrome2 "tacocat"))
(show (isPalindrome1 "tacodog"))
(show (isPalindrome2 "tacodag"))

(define isAnagram (psi equal? string-sort))
(show (isAnagram "owls" "slow"))
(show (isAnagram "cats" "dogs"))

;(define isDisjoint (phi '() equal? intersect))
(define isDisjoint (B null? intersect))
(show (isDisjoint '(1 2) '(3 4 5)))
(show (isDisjoint '(2 3) '(3 4 5)))

;(define isPrefixOf (B first find))
;(show (isPrefixOf "cat" "catch"))
;(show (isPrefixOf "dog" "catch"))

(show (S K K 10))
