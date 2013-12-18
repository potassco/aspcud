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

set(solution,P,V) :-     in(P,V).
set(changed,P,V)  :-     in(P,V), not installed(P,V).
set(changed,P,V)  :- not in(P,V),     installed(P,V).
set(new,P,V)      :-     in(P,V), not installed(P).
set(removed,P,V)  :- not in(P),       installed(P,V).
set(up,P,V)       :-     in(P,V); not installed(P,W) : installed(P,W), W >= V; installed(P).
set(down,P,V)     :-     in(P,V); not installed(P,W) : installed(P,W), W <= V; installed(P).

opt(P,V,1,O, 1,L) :- criterion(O,S,count,L),            set(S,P,V).
opt(P,V,1,O, W,L) :- criterion(O,S,sum(A),L),           set(S,P,V), attribute(P,V,A,W).
opt(P,V,1,O, 1,L) :- criterion(O,S,notuptodate,L),      set(S,P,V), not maxversion(P,V).
opt(P,V,D,O, W,L) :- criterion(O,S,unsat_recommends,L), set(S,P,V), recommends(P,V,D,W), not satisfied(D).
opt(H,B,1,O, 1,L) :- criterion(O,S,aligned(G,A),L),     set(S,P,V), attribute(P,V,G,H), attribute(P,V,A,B).
opt(H,1,2,O,-1,L) :- criterion(O,S,aligned(G,A),L),     set(S,P,V), attribute(P,V,G,H).

#minimize { 0@L                : criterion(_,_,_,L)      }.
#minimize { W@L,P,V,X,minimize : opt(P,V,X,minimize,W,L) }.
#maximize { W@L,P,V,X,maximize : opt(P,V,X,maximize,W,L) }.

% output projection

#show in/2.

