installed    :- 1 { criterion(newpackage;remove,L) }.
installed(P) :- installed(P,X), installed.

% choose optimization criteria violations

{ violated(change,     P,     1) } :- installed(P),                      criterion(change,L).
{ violated(change,     P,     1) } :- unit(P,X),                         criterion(change,L).
{ violated(newpackage, P,     1) } :- unit(P,X), not installed(P),       criterion(newpackage,L).
{ violated(remove,     P,     1) } :- installed(P),                      criterion(remove,L).
{ violated(uptodate,   P,     1) } :- unit(P,X), not newestversion(P,X), criterion(uptodate,L).
{ violated(recommend,r(P,X,D),R) } :- unit(P,X), recommends(P,X,D,R),    criterion(recommend,L).
{ violated(sum(K),   a(P,X),  V) } :- unit(P,X),  attribute(P,X,K,V),    criterion(sum(K),L).

:- installed(P,X), not in(P,X), not violated(change,     P,     1), criterion(change,L).
:- in(P,X), not installed(P,X), not violated(change,     P,     1), criterion(change,L).
:- in(P),   not installed(P),   not violated(newpackage, P,     1), criterion(newpackage,L).
:- installed(P),   not in(P),   not violated(remove,     P,     1), criterion(remove,L).
:- in(P),          not in(P,X), not violated(uptodate,   P,     1), criterion(uptodate,L),  newestversion(P,X).
:- in(P,X), not satisfied(D),   not violated(recommend,r(P,X,D),R), criterion(recommend,L), recommends(P,X,D,R).
:- in(P,X), attribute(P,X,K,V), not violated(sum(K),   a(P,X),  V), criterion(sum(K),L).

% post optimization criteria ("dummy" prevents omission of empty priority levels in clasp output)

dummy.
#minimize[ dummy           = 0 @ |L| : criterion(C,L) ].
#minimize[ violated(C,P,W) = W @ -L  : criterion(C,L) : L < 0 ].
#maximize[ violated(C,P,W) = W @  L  : criterion(C,L) : L > 0 ].
