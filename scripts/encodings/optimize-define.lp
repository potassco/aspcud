opt_define.

% define optimization criteria violations

violated(change,P)     :- installed(P,X), not in(P,X),                   criterion(change,L).
violated(change,P)     :- in(P,X), not installed(P,X),                   criterion(change,L).
violated(newpackage,P) :- in(P),     not installed(P),                   criterion(newpackage,L).
violated(remove,P)     :- installed(P),     not in(P),                   criterion(remove,L).
violated(uptodate,P)   :- in(P),   newestversion(P,X), not in(P,X),      criterion(uptodate,L).
violated(recommend,r(P,X,D)) :- in(P,X),  recommends(P,X,D), not satisfied(D), criterion(recommend,L).
% violated(recommend,P)  :- in(P),    recommends(P,D),   not satisfied(D).

% post optimization statement (dummy used to avoid empty priority levels)

dummy.
#minimize [ dummy = 0 @ |L| : criterion(C,L) ].
#minimize { violated(C,P) @ -L : criterion(C,L) : L < 0 }.
#maximize { violated(C,P) @  L : criterion(C,L) : L > 0 }.
