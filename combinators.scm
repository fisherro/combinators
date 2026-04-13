#lang racket/base
(require racket/set)
(require racket/function)
;(require srfi/26)

;;; Inspired by
;;; Combinatory Logic and Combinators in Array Languages
;;; by Conor Hoekstra

;(define-syntax-rule (define-combinator name (param ...) (arg ...) body)
;  (define name
;    (case-lambda
;      ((param ...) (lambda (arg ...) body))
;      ((param ... arg ...) ((name param ...) arg ...)))))

(define-syntax define-combinator
  (syntax-rules ()
    [(_ name (param ...) (arg ...) body)
     (define name
       (case-lambda
         ((param ...) (lambda (arg ...) body))
         ((param ... arg ...) ((name param ...) arg ...))))]
    [(_ name (param ...) rest body)
     (define name
       (case-lambda
         ((param ...) (lambda rest body))
         ((param ... . rest) (apply (name param ...) rest))))]))


(define-syntax-rule (show expr)
  (printf "~a: ~a~n" 'expr expr))

;; Combinators
(define (I x) x)
;(define (K x y) x)
(define-combinator K (x) (y) x)
; S is a specialization of phi
; S is (also?) a specialization of D
(define-combinator S (f g) (x) (f x (g x)))
(define-combinator B (f g) (x) (f (g x)))
(define-combinator B1 (f g) (x y) (f (g x y)))
;(define B*
;  (case-lambda
;    ((f g) (lambda args (f (apply g args))))
;    ((f g . args) (apply (B* f g) args))))
(define-combinator B* (f g) args (f (apply g args)))
(define-combinator C (f) (x y) (f y x))
; W is a specialization of S
; W is (also?) a specialization of psi
(define-combinator W (f) (x) (f x x))
; Psi is a specialization of D2
(define-combinator psi (f g) (x y) (f (g x) (g y)))
(define-combinator phi (f g h) (x) (g (f x) (h x)))
; Phi1 is a specialization of D2
(define-combinator phi1 (f g h) (x y) (g (f x y) (h x y)))
; D is a specialization of D2
(define-combinator D (f g) (x y) (f x (g y)))
(define-combinator D2 (f g h) (x y) (g (f x) (h y)))

;; Helpers
(define (sum lyst) (apply + lyst))
;(define (string-reverse s) (list->string (reverse (string->list s))))
(define string-reverse (B list->string (B reverse string->list)))
;(define (string-sort s) (list->string (sort (string->list s) char<?)))
;(define string-sort
;  (B list->string (B (curryr sort char<?) string->list)))
(define string-sort
  (B list->string (phi string->list sort (K char<?))))

;; Examples
(define avg (phi sum / length))
(show (avg '(1 2 3 4)))

(define plusOrMinus (phi1 + list -))
(show (plusOrMinus 10 5))

(define absoluteDifference (B1 abs -))
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

;(define isDisjoint (phi1 '() equal? set-intersect))
(define isDisjoint (B1 null? set-intersect))
(show (isDisjoint '(1 2) '(3 4 5)))
(show (isDisjoint '(2 3) '(3 4 5)))

;(define isPrefixOf (B1 first find))
;(show (isPrefixOf "cat" "catch"))
;(show (isPrefixOf "dog" "catch"))

(show (S K K 10))
