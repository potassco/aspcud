% impose conditions on new packages

skip_newpkg   :- criterion(C,L), C != remove, 0 < L.

precommends(P,D) :- recommends(P,X,D), criterion(recommend,L), not skip_newpkg.

dependency(D) :- request(D), not skip_newpkg.
dependency(D) :- pdepends(P,D), not skip_newpkg.
dependency(D) :- precommends(P,D), criterion(recommend,L), not skip_newpkg,
                 not criterion(newpackage,M) : criterion(newpackage,M) : M < L,
                 not criterion(change,N)     : criterion(change,N)     : N < L.

satisfier(P)  :- psatisfies(P,D), dependency(D), not installed(P), % satisfied(D),
                 not in(Q,Y) : satisfies(Q,Y,D) : Q != P.
satisfier(P)  :- psatisfies(P,D), dependency(D), not installed(P), % satisfied(D),
                 not in(Q,Y) : satisfies(Q,Y,D) : Q != P : not satisfies(Q,D),
                 not in(R)   : satisfies(R,  D) : R != P.

:- in(P), not installed(P), not satisfier(P), not skip_newpkg.
