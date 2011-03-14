installed(P)     :- installed(P,X).

% analyze package relationships

pconflict(P,D)   :- conflict(P,X,D).
conflict(P,D)    :- pconflict(P,D),    conflict(P,X,D) : unit(P,X).

pdepends(P,D)    :- depends(P,X,D).
depends(P,D)     :- pdepends(P,D),     depends(P,X,D) : unit(P,X).

% precommends(P,D) :- recommends(P,X,D), criterion(recommend,L).
% recommends(P,D)  :- precommends(P,D),  recommends(P,X,D) : unit(P,X).

psatisfies(P,D)  :- satisfies(P,X,D).
satisfies(P,D)   :- psatisfies(P,D),   satisfies(P,X,D) : unit(P,X).

% generate installation

{ in(P,X) }      :- unit(P,X).
in(P)            :- in(P,X).

forbidden(D)     :- in(P),   conflict(P,D).
forbidden(D)     :- in(P,X), conflict(P,X,D).

requested(D)     :- in(P),   depends(P,D).
requested(D)     :- in(P,X), depends(P,X,D).

satisfied(D)     :- in(P),   satisfies(P,D).
satisfied(D)     :- in(P,X), satisfies(P,X,D).

:- request(D),   not satisfied(D).
:- requested(D), not satisfied(D).
:- forbidden(D),     satisfied(D).

% output projection

#hide.
#show in/2.
