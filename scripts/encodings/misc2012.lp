%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% options (values: 0 for off, 1 for on) %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% group clique elements sharing relations
#const cliqueRelations = 1.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% reify (hard) package relations %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

relate(P,V,con,D) :- unit(P,V,in), conflict(P,V,D).
relate(P,V,dep,D) :- unit(P,V,in), depends(P,V,D).
relate(P,V,sat,D) :- unit(P,V,in), satisfies(P,V,D).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% analyze clique dependencies %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clique(I) :- clique(I,_,_).

realClique(I)     :- clique(I), 2 { unit(P,V,in) : clique(I,P,V) }.
realClique(I,P,V) :- realClique(I), clique(I,P,V), unit(P,V,in).

relaClique(I,P,V,R,D) :- realClique(I,P,V), relate(P,V,R,D), cliqueRelations == 1.
relaClique(I,R,D)     :- relaClique(I,_,_,R,D).

joinClique(I,R,D)   :- relaClique(I,R,D), 2 { relaClique(I,_,_,R,D) }.
joinClique(I,R,D,N) :- joinClique(I,R,D), N = { relaClique(I,_,_,R,D) }.

dropClique(P,V,R,D) :- relaClique(I,P,V,R,D), joinClique(I,R,D).

equiClique(I,R1,D1,R2,D2) :- joinClique(I,R1,D1,N), joinClique(I,R2,D2,N), (R1,D1) < (R2,D2),
                             relaClique(I,P,V,R1,D1) : relaClique(I,P,V,R2,D2).

mapsClique(I,R1,D1,R1,D1) :- joinClique(I,R1,D1),
                             not equiClique(I,R,D,R1,D1) : equiClique(I,R,D,R1,D1).
mapsClique(I,R1,D1,R2,D2) :- equiClique(I,R1,D1,R2,D2), mapsClique(I,R1,D1,R1,D1).

%%%%%%%%%%%%%%%%%%%%%
% generate solution %
%%%%%%%%%%%%%%%%%%%%%

{ in(P,V) } :- unit(P,V,in).

activeClique(I,R1,D1) :- in(P,V), relaClique(I,P,V,R1,D1), mapsClique(I,R1,D1,R1,D1).

active(R,D) :- activeClique(I,R1,D1), mapsClique(I,R1,D1,R,D).
active(R,D) :- in(P,V), relate(P,V,R,D), not dropClique(P,V,R,D).

 :- active(con,D),     active(sat,D).
 :- active(dep,D), not active(sat,D).
 :-    request(D), not active(sat,D).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% calculate target sets and elements of objectives %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

installed(P) :- installed(P,_).

target(T) :- criterion(_,T,_,_).

  up(P,M) :- target(up),   installed(P), M := #max[ installed(P,V) = V ].
down(P,M) :- target(down), installed(P), M := #min[ installed(P,V) = V ].

target(sol,solution,P,V) :- unit(P,V,in),   target(solution).
target(sol,changed ,P,V) :- unit(P,V,in),   target(changed), not installed(P,V).
target(sub,changed ,P,V) :- installed(P,V), target(changed).
target(sol,new     ,P,V) :- unit(P,V,in),   target(new),     not installed(P).
target(sub,removed ,P,V) :- installed(P,V), target(removed).
target(sol,up      ,P,V) :- unit(P,V,in),     up(P,M), M < V.
target(sol,down    ,P,V) :- unit(P,V,in),   down(P,M), V < M.

oneval(T,A,X,B,Y) :- target(sub,T,P,V), criterion(_,T,aligned(A,B),_),
                     attribute(P,V,A,X), attribute(P,V,B,Y).
oneval(T,A,X,B,Y) :- target(sol,T,P,V), criterion(_,T,aligned(A,B),_),
                     attribute(P,V,A,X), attribute(P,V,B,Y),
                     attribute(Q,W,B,Y) : attribute(Q,W,A,X) : target(sol,T,Q,W) :
                     realClique(I,Q,W) : realClique(I,P,V).
oneval(T,A,X,B,Y) :- target(sol,T,P,V), oneval(T,A,X,B), realClique(_,P,V),
                     attribute(P,V,A,X), attribute(P,V,B,Y).
oneval(T,A,X,B)   :- oneval(T,A,X,B,_).

twoval(T,A,X,B,I) :- target(sol,T,P,V), criterion(_,T,aligned(A,B),_), realClique(I,P,V),
                     attribute(P,V,A,X), not oneval(T,A,X,B).
twoval(T,A,X,B)   :- twoval(T,A,X,B,_).

keyval(T,A,X,B) :- oneval(T,A,X,B), 2 { oneval(T,A,X,B,_) }.
keyval(T,A,X,B) :- twoval(T,A,X,B), 2 { twoval(T,A,X,B,_) }.

object(count           ,S,T,P,V) :- target(S,T,P,V), criterion(_,T,count,_).
object(sum(A)          ,S,T,P,V) :- target(S,T,P,V), criterion(_,T,sum(A),_),
                                    attribute(P,V,A,X), X != 0.
object(notuptodate     ,S,T,P,V) :- target(S,T,P,V), criterion(_,T,notuptodate,_),
                                    not maxversion(P,V).
object(unsat_recommends,S,T,P,V) :- target(S,T,P,V), criterion(_,T,unsat_recommends,_),
                                    recommends(P,V,_,_).
object(aligned(A,B)    ,S,T,P,V) :- target(S,T,P,V), criterion(_,T,aligned(A,B),_),
                                    attribute(P,V,A,X), keyval(T,A,X,B).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% calculate constituents of objectives %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

optClique(I,C,S,T,P,V) :- object(C,S,T,P,V), clique(I,P,V).
optClique(I,C,S,T)     :- optClique(I,C,S,T,_,_).

one(count;notuptodate).

hasClique(I,C               ,S,T,P,V,1)        :- optClique(I,C               ,S,T,P,V), one(C).
hasClique(I,sum(A)          ,S,T,P,V,X)        :- optClique(I,sum(A)          ,S,T,P,V),
                                                  attribute(P,V,A,X).
hasClique(I,unsat_recommends,S,T,P,V,rec(D,W)) :- optClique(I,unsat_recommends,S,T,P,V),
                                                  recommends(P,V,D,W).
hasClique(I,aligned(A,B)    ,S,T,P,V,ali(X,Y)) :- optClique(I,aligned(A,B)    ,S,T,P,V),
                                                  attribute(P,V,A,X), attribute(P,V,B,Y).
hasClique(I,C,S,T,Z)                           :- hasClique(I,C,S,T,_,_,Z).

offClique(I,aligned(A,B),sub,T,Z) :- hasClique(I,aligned(A,B),sub,T,Z),
                                     2 { hasClique(I,aligned(A,B),sub,T,_,_,Z) }.

resClique(I,C,S,T,P,V,Z) :- hasClique(I,C,S,T,P,V,Z), not offClique(I,C,S,T,Z).
resClique(I,C,S,T,P,V)   :- resClique(I,C,S,T,P,V,_).
resClique(I,C,S,T)       :- optClique(I,C,S,T), 2 { resClique(I,C,S,T,_,_) }.
resClique(I,C,S,T,Z)     :- resClique(I,C,S,T,_,_,Z), resClique(I,C,S,T).

nonClique(C,S,T,P,V)     :- object(C,S,T,P,V), not clique(I,P,V) : clique(I,P,V).
nonClique(I,C,S,T,P,V,Z) :- resClique(I,C,S,T,P,V,Z), not resClique(I,C,S,T).

simClique(I,C,S,T,Z) :- resClique(I,C,S,T,Z), resClique(I,C,S,T,P,V,Z) : resClique(I,C,S,T,P,V).
simClique(I,C,S,T)   :- resClique(I,C,S,T), simClique(I,C,S,T,Z) : resClique(I,C,S,T,Z).

eleClique(I,C,S,T,N) :- simClique(I,C,S,T), N = { resClique(I,C,S,T,_,_) }.

uniClique(I,C1,S1,T1,C2,S2,T2) :- eleClique(I,C1,S1,T1,N), eleClique(I,C2,S2,T2,N),
                                  (C1,S1,T1) < (C2,S2,T2),
                                  resClique(I,C1,S1,T1,P,V) : resClique(I,C2,S2,T2,P,V).

mapClique(I,C1,S1,T1,C1,S1,T1) :- resClique(I,C1,S1,T1), not simClique(I,C1,S1,T1).
mapClique(I,C1,S1,T1,C1,S1,T1) :- simClique(I,C1,S1,T1),
                                  not uniClique(I,C,S,T,C1,S1,T1) : uniClique(I,C,S,T,C1,S1,T1).
mapClique(I,C1,S1,T1,C2,S2,T2) :- uniClique(I,C1,S1,T1,C2,S2,T2), mapClique(I,C1,S1,T1,C1,S1,T1).

%%%%%%%%%%%%%%
% objectives %
%%%%%%%%%%%%%%

mult(minimize, 1).
mult(maximize,-1).

sign(sol, 1).
sign(sub,-1).

% count & notuptodate
#minimize[ in(P,V) = E*F @ L :
           nonClique(C,S,T,P,V) : criterion(M,T,C,L) : mult(M,E) : sign(S,F) : one(C) %, ...
         ].
