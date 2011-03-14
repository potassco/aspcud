installed(P)     :- installed(P,X).


% generate installation

{ in(P,X) }      :- unit(P,X).
in(P)            :- in(P,X).

:- conflict(P,X,Q,Y), in(P,X), in(Q,Y).

requested(D)    :- in(P,X), depends(P,X,D).

satisfied(D)    :- in(P,X), satisfies(P,X,D).

:- request(D),   not satisfied(D).
:- requested(D), not satisfied(D).


% optimize installation

violated(change,P)     :- installed(P,X), not in(P,X),                  criterion(change,L).
violated(change,P)     :- in(P,X), not installed(P,X),                  criterion(change,L).
violated(newpackage,P) :- in(P), not installed(P),                      criterion(newpackage,L).
violated(remove,P)     :- installed(P), not in(P),                      criterion(remove,L).
violated(uptodate,P)   :- in(P), newestversion(P,X), not in(P,X),       criterion(uptodate,L).
violated(recommend,P)  :- in(P,X), recommends(P,X,D), not satisfied(D), criterion(recommend,L).

{dummy} :- criterion(C,L).
#minimize { violated(C,P) @ L : criterion(C,L), dummy @ L : criterion(C,L) }.


% output projection

#hide.
#show in/2.
