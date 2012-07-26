% Note: simple encoding directly derived from the specification
%       without any optimizations

{ in(P,V) } :- unit(P,V,in).

forbidden(D) :- in(P,V), conflict(P,V,D).
requested(D) :- in(P,V), depends(P,V,D).
satisfied(D) :- in(P,V), satisfies(P,V,D).

:-   request(D), not satisfied(D).
:- requested(D), not satisfied(D).
:- forbidden(D),     satisfied(D).

in(P)        :- in(P,_).
installed(P) :- installed(P,_).

% self-conflicts
set(solution,P,V) :-     in(P,V).
set(changed,P,V)  :-     in(P,V), not installed(P,V).
set(new,P,V)      :-     in(P,V), not installed(P).
set(up,P,V)       :-     in(P,V), not installed(P,W) : installed(P,W) : W >= V.
set(down,P,V)     :-     in(P,V), not installed(P,W) : installed(P,W) : W <= V.
% chain these
set(removed,P,V)  :- not in(P),       installed(P,V).
set(changed,P,V)  :- not in(P,V),     installed(P,V).

% self-conflicts
opt(P,V,1,O, 1,L) :- criterion(O,S,count,L),            set(S,P,V).
opt(P,V,1,O, W,L) :- criterion(O,S,sum(A),L),           set(S,P,V), attribute(P,V,A,W).
opt(P,V,1,O, 1,L) :- criterion(O,S,notuptodate,L),      set(S,P,V), not maxversion(P,V).
% ordering possible (same package - same recommendations?)
opt(P,V,D,O, W,L) :- criterion(O,S,unsat_recommends,L), set(S,P,V), recommends(P,V,D,W), not satisfied(D).
% the minus one has to go
opt(H,B,1,O, 1,L) :- criterion(O,S,aligned(G,A),L),     set(S,P,V), attribute(P,V,G,H), attribute(P,V,A,B).
opt(H,1,2,O,-1,L) :- criterion(O,S,aligned(G,A),L),     set(S,P,V), attribute(P,V,G,H).

#minimize [ criterion(_,_,_,L)      = 0 @ L ].
#minimize [ opt(P,V,X,minimize,W,L) = W @ L ].
#maximize [ opt(P,V,X,maximize,W,L) = W @ L ].

% output projection

#hide.
#show in/2.

