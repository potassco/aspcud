% define optimization criteria violations

violated(change,P,1)           :- installed(P,X), not in(P,X),                     criterion(change,L).
violated(change,P,1)           :- in(P,X), not installed(P,X),                     criterion(change,L).
violated(newpackage,P,1)       :- in(P),     not installed(P),                     criterion(newpackage,L).
violated(remove,P,1)           :- installed(P),     not in(P),                     criterion(remove,L).
violated(uptodate,P,1)         :- in(P),   newestversion(P,X), not in(P,X),        criterion(uptodate,L).
violated(recommend,r(P,X,D),W) :- in(P,X),  recommends(P,X,D,W), not satisfied(D), criterion(recommend,L).
violated(sum(K),a(P,X,K),V)    :- installed(P,X), attribute(P,X,K,V),              criterion(sum(K),L).

% post optimization statement (dummy used to avoid empty priority levels)
dummy.
#minimize [ dummy = 0 @ |L| : criterion(C,L) ].
#minimize [ violated(C,P,W) = W @ -L : criterion(C,L) : L < 0 ].
#maximize [ violated(C,P,W) = W @  L : criterion(C,L) : L > 0 ].
