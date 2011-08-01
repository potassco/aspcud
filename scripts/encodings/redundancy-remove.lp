% impose conditions on removed package versions

skip_remove   :- criterion(C,L), 0 < L.

con_ok(P,X)   :- unit(P,X), installed(P,X), not satisfied(D) : conflict(P,X,D),
                 not forbidden(D) : satisfies(P,X,D), not skip_remove.

dep_ok(P,X)   :- unit(P,X), installed(P,X), satisfied(D) : depends(P,X,D), not skip_remove.

rec_ok(P,X)   :- unit(P,X), installed(P,X),
                 not criterion(recommend,L) : criterion(recommend,L), not skip_remove.
rec_ok(P,X)   :- unit(P,X), installed(P,X), satisfied(D) : recommends(P,X,D),
                 criterion(recommend,L), not skip_remove.

%*
recommends(P) :- installed(P), precommends(P,D), criterion(recommend,L), not skip_remove.

rec_ok(P)     :- installed(P),  not recommends(P), not skip_remove.
rec_ok(P)     :- recommends(P), not satisfied(recommend,P).
*%

utd_ok(P)     :- installed(P), not criterion(uptodate,L) : criterion(uptodate,L),
                 not skip_remove.
utd_ok(P)     :- installed(P), newestversion(P,X), in(P,X), criterion(uptodate,L),
                 not skip_remove.
utd_ok(P)     :- installed(P), violated(uptodate,P),
                 not skip_remove.

:- con_ok(P,X), dep_ok(P,X), rec_ok(P,X), utd_ok(P), not in(P,X), not newestversion(P,X).
% :- con_ok(P,X), dep_ok(P,X), rec_ok(P),   utd_ok(P), not in(P,X), not newestversion(P,X).
:- con_ok(P,X), dep_ok(P,X), rec_ok(P,X),            not in(P,X),     newestversion(P,X).
% :- con_ok(P,X), dep_ok(P,X), rec_ok(P),              not in(P,X),     newestversion(P,X).
