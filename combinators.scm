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

(define-syntax test
  (syntax-rules ()
    ((_ expr expected)
     (let ((result expr))
       (display "[")
       (display (if (equal? result expected) "PASS" "FAIL"))
       (display "] ")
       (write 'expr)
       (display ": ")
       (display result)
       (newline)))))

;; Combinators
(define (I x) x)
(define-combinator K (x) (y) x)
(define-combinator S (f g) (x) (f x (g x)))
(define-combinator B (f g) args (f (apply g args)))
(define-combinator C (f) (x y) (f y x))
(define-combinator W (f) (x) (f x x))
(define-combinator PSI (f g) (x y) (f (g x) (g y)))
(define-combinator PHI (f g h) args (g (apply f args) (apply h args)))
(define-combinator D (f g) (x y) (f x (g y)))
(define-combinator D2 (f g h) (x y) (g (f x) (h y)))

;; Helpers
(define (sum lyst) (apply + lyst))
(define string-reverse (B list->string (B reverse string->list)))
(define string-sort
  (B list->string (PHI string->list sort (K char<?))))
(define intersect (cut lset-intersection equal? <> <>))

;; Examples
(test (S K K 10) 10)

(define avg (PHI sum / length))
(test (avg '(1 2 3 4)) 5/2)

(define plus-or-minus (PHI + list -))
(test (plus-or-minus 10 5) '(15 5))

(define absolute-difference (B abs -))
(test (absolute-difference 10 7) 3)
(test (absolute-difference 7 10) 3)

(define palindrome1? (PHI string-reverse equal? I))
(define palindrome2? (S equal? string-reverse))
(test (palindrome1? "tacocat") #t)
(test (palindrome2? "tacocat") #t)
(test (palindrome1? "tacodog") #f)
(test (palindrome2? "tacodag") #f)

(define anagram? (PSI equal? string-sort))
(test (anagram? "owls" "slow") #t)
(test (anagram? "cats" "dogs") #f)

;(define disjoint? (PHI '() equal? intersect))
(define disjoint? (B null? intersect))
(test (disjoint? '(1 2) '(3 4 5)) #t)
(test (disjoint? '(2 3) '(3 4 5)) #f)

;(define prefix-of? (B first find))
;(show (prefix-of? "cat" "catch"))
;(show (prefix-of? "dog" "catch"))
