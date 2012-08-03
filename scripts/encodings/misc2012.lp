%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% options (values: 0 for off, 1 for on) %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% group dependencies shared by clique elements
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
oneval(T,A,X,B,Y) :- target(sol,T,P,V), oneval(T,A,X,B), realClique(I,P,V),
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
