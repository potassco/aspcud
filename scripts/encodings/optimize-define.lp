installed    :- 1 { criterion(newpackage;remove,L) }.
installed(P) :- installed(P,X), installed.

% define optimization criteria violations

% Note: to optimize this we would need information about packages that are mutually exclusive
% the next competition wants us to use packages here not just names
violated(change,     P,     1) :- installed(P,X), not in(P,X),                    criterion(change,L).
violated(change,     P,     1) :- in(P,X), not installed(P,X),                    criterion(change,L).
violated(newpackage, P,     1) :- in(P),   not installed(P),                      criterion(newpackage,L).
violated(remove,     P,     1) :- installed(P),   not in(P),                      criterion(remove,L).
violated(uptodate,   P,     1) :- in(P),   newestversion(P,X),  not in(P,X),      criterion(uptodate,L).
% Note: self-conflicts might help here
violated(recommend,r(P,X,D),R) :- in(P,X), recommends(P,X,D,R), not satisfied(D), criterion(recommend,L).
violated(sum(K),   a(P,X),  V) :- in(P,X),  attribute(P,X,K,V),                   criterion(sum(K),L).



% post optimization criteria ("dummy" prevents omission of empty priority levels in clasp output)

dummy.
#minimize[ dummy           = 0 @ |L| : criterion(C,L) ].
#minimize[ violated(C,P,W) = W @ -L  : criterion(C,L) : L < 0 ].
#maximize[ violated(C,P,W) = W @  L  : criterion(C,L) : L > 0 ].
