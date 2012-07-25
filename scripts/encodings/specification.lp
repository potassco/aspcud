% Note: simple encoding directly derived from the specification
%       without any optimizations

{ in(P,X) }  :- unit(P,X).
in(P)        :- in(P,X), inp.

forbidden(D) :- in(P,X), conflict(P,X,D).
requested(D) :- in(P,X), depends(P,X,D).
satisfied(D) :- in(P,X), satisfies(P,X,D).

:-   request(D), not satisfied(D).
:- requested(D), not satisfied(D).
:- forbidden(D),     satisfied(D).

installed(P) :- installed(P,_).

set(solution,P,V) :-     in(P,V).
set(changed,P,V)  :-     in(P,V), not installed(P,V).
set(changed,P,V)  :- not in(P,V),     installed(P,V).
set(new,P,V)      :-     in(P,V), not installed(P).
set(removed,P,V)  :- not in(P,V),     installed(P), unit(P,V).
set(up,P,V)       :-     in(P,V), not installed(P,VV) : installed(P,VV) : VV >= V.
set(down,P,V)     :-     in(P,V), not installed(P,VV) : installed(P,VV) : VV <= V.

opt(P,V,1,O, 1,L) :- criterion(O,S,count,L),            set(S,P,V).
opt(P,V,1,O, W,L) :- criterion(O,S,sum(A),L),           set(S,P,V), attribute(P,V,A,W).
opt(P,V,1,O, 1,L) :- criterion(O,S,notuptodate,L),      set(S,P,V), not maxversion(P,V).
opt(P,V,D,O, W,L) :- criterion(O,S,unsat_recommends,L), set(S,P,V), recommends(P,V,D,W), not satisfied(D).
opt(H,B,1,O, 1,L) :- criterion(O,S,aligned(G,A),L),     set(S,P,V), attribute(P,V,G,H), attribute(P,V,A,B).
opt(H,1,2,O,-1,L) :- criterion(O,S,aligned(G,A),L),     set(S,P,V), attribute(P,V,G,H).

#minimize [ criterion(_,_,_,L)      = 0 @ L ].
#minimize [ opt(minimize,P,V,X,W,L) = W @ L ].
#maximize [ opt(maximize,P,V,X,W,L) = W @ L ].

% output projection

#hide.
#show in/2.

