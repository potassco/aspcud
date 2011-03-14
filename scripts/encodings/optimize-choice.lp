opt_choice.

% guess optimization criteria violations

{ satisfied(change,P) }     :- installed(P,X),                    criterion(change,L).
{ satisfied(change,P) }     :- unit(P,X),                         criterion(change,L).
{ satisfied(newpackage,P) } :- unit(P,X), not installed(P),       criterion(newpackage,L).
{ satisfied(remove,P) }     :- installed(P),                      criterion(remove,L).
{ satisfied(uptodate,P) }   :- unit(P,X), not newestversion(P,X), criterion(uptodate,L).
% { satisfied(recommend,P) }  :- precommends(P,D).
{ satisfied(recommend,r(P,X,D)) } :- unit(P,X), recommends(P,X,D).

% force optimization criteria violations

:- installed(P,X), not in(P,X), satisfied(change,P).
:- in(P,X), not installed(P,X), satisfied(change,P).
:- in(P),     not installed(P), satisfied(newpackage,P).
:- installed(P),     not in(P), satisfied(remove,P).
:- in(P),   newestversion(P,X), satisfied(uptodate,P),  not in(P,X).
:- in(P,X),  recommends(P,X,D), satisfied(recommend,r(P,X,D)), not satisfied(D).
% :- in(P),      recommends(P,D), satisfied(recommend,P), not satisfied(D).

% post optimization statement (dummy used to avoid empty priority levels)

dummy.
#minimize [ dummy = 0 @ |L| : criterion(C,L) ].
#maximize { satisfied(C,P) @ -L : criterion(C,L) : L < 0 }.
#minimize { satisfied(C,P) @  L : criterion(C,L) : L > 0 }.
