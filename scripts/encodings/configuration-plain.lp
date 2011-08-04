inp :- 1 { criterion(newpackage;remove;uptodate,L) }.

% generate installation

{ in(P,X) }  :- unit(P,X).
in(P)        :- in(P,X), inp.

forbidden(D) :- in(P,X), conflict(P,X,D).

requested(D) :- in(P,X), depends(P,X,D).

satisfied(D) :- in(P,X), satisfies(P,X,D).

:-   request(D), not satisfied(D).
:- requested(D), not satisfied(D).
:- forbidden(D),     satisfied(D).

% output projection

#hide.
#show in/2.
