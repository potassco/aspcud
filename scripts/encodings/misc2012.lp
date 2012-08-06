%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% options (values: 0 for off, 1 for on) %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% group clique elements sharing relations
#const cliqueRelations = 1.

%

%%%%%%%%%%%%%%%%%%%%%%%%%%%
% reify package relations %
%%%%%%%%%%%%%%%%%%%%%%%%%%%

relate(P,V,con,D) :- unit(P,V,in), conflict(P,V,D).
relate(P,V,dep,D) :- unit(P,V,in), depends(P,V,D).
relate(P,V,rec,D) :- unit(P,V,in), recommends(P,V,D,_).
relate(P,V,sat,D) :- unit(P,V,in), satisfies(P,V,D).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% analyze conflict cliques %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clique(I) :- clique(I,_,_).

realClique(I)     :- clique(I), 2 { unit(P,V,in) : clique(I,P,V) }.
realClique(I,P,V) :- realClique(I), clique(I,P,V), unit(P,V,in).
realClique(I,P)   :- realClique(I,P,_).

relaClique(I,P,V,R,D) :- realClique(I,P,V), relate(P,V,R,D), cliqueRelations == 1.
relaClique(I,R,D)     :- relaClique(I,_,_,R,D).

joinClique(I,R,D)   :- relaClique(I,R,D),   2 { relaClique(I,_,_,R,D) }.
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

solution(P) :- unit(P,_,in).

criterion(T,C) :- criterion(_,T,C,_).

target(T) :- criterion(T,_).

  up(P,M) :- target(up),   installed(P), M := #max[ installed(P,V) = V ].
down(P,M) :- target(down), installed(P), M := #min[ installed(P,V) = V ].

target(sol,solution,P,V) :- unit(P,V,in), target(solution).
target(sol,changed ,P,V) :- unit(P,V,in), target(changed), not installed(P,V).
target(sol,new     ,P,V) :- unit(P,V,in), target(new),     not installed(P).
target(sol,up      ,P,V) :- unit(P,V,in),   up(P,M), M < V.
target(sol,down    ,P,V) :- unit(P,V,in), down(P,M), V < M.

target(sub,changed,P,V) :- installed(P,V), target(changed).
target(sub,removed,P,V) :- installed(P,V), target(removed).
target(sub,removed,P)   :- target(sub,removed,P,_).

delete(changed,P,V) :- target(sub,changed,P,V), not unit(P,V,in).
delete(removed,P,V) :- target(sub,removed,P,V), not solution(P).

remClique(P)   :- target(sub,removed,P), 1 { realClique(_,P) } 1.
remClique(I,P) :- realClique(I,P), remClique(P).

delrec(T,D) :- target(sub,T,P,V), criterion(T,unsat_recommends), delete(T,P,V),
               recommends(P,V,D,_).

static(T,unsat_recommends(D),W) :- delrec(T,D),
                                   W = [ recommends(P,V,D,U) = U : delete(T,P,V) ].

oneval(T,A,X,B,Y) :- target(sub,T,P,V), criterion(T,aligned(A,B)),
                     attribute(P,V,A,X), attribute(P,V,B,Y).
oneval(T,A,X,B,Y) :- target(sol,T,P,V), criterion(T,aligned(A,B)),
                     attribute(P,V,A,X), attribute(P,V,B,Y),
                     attribute(Q,W,B,Y) : attribute(Q,W,A,X) : target(sol,T,Q,W) :
                     realClique(I,Q,W) : realClique(I,P,V).
oneval(T,A,X,B,Y) :- target(sol,T,P,V), oneval(T,A,X,B), realClique(_,P,V),
                     attribute(P,V,A,X), attribute(P,V,B,Y).
oneval(T,A,X,B)   :- oneval(T,A,X,B,_).

twoval(T,A,X,B,I) :- target(sol,T,P,V), criterion(T,aligned(A,B)), realClique(I,P,V),
                     attribute(P,V,A,X), not oneval(T,A,X,B).
twoval(T,A,X,B)   :- twoval(T,A,X,B,_).

keyval(T,A,X,B) :- oneval(T,A,X,B), 2 { oneval(T,A,X,B,_) }.
keyval(T,A,X,B) :- twoval(T,A,X,B), 2 { twoval(T,A,X,B,_) }.

remval(I,changed,P,V,A,X,B,Y) :- target(sub,changed,P,V), criterion(changed,aligned(A,B)),
                                 keyval(T,A,X,B), attribute(P,V,A,X), attribute(P,V,B,Y),
                                 realClique(I,P,V).
remval(I,removed,P,0,A,X,B,Y) :- target(sub,removed,P,V), criterion(removed,aligned(A,B)),
                                 keyval(T,A,X,B), attribute(P,V,A,X), attribute(P,V,B,Y),
                                 remClique(I,P).
remval(I,T,A,X,B,Y)           :- remval(I,T,_,_,A,X,B,Y).

static(T,aligned(A,X,B,Y)) :- target(sub,T,P,V), delete(T,P,V),
                              keyval(T,A,X,B), attribute(P,V,A,X), attribute(P,V,B,Y).
static(T,aligned(A,X,B,Y)) :- target(sub,T,P,V), remval(I,T,A,X,B,Y),
                              2 { remval(I,T,_,_,A,X,B,Y) }.

object(1,count              ,S,T,P,V) :- target(S,T,P,V), criterion(T,count),
                                         not delete(T,P,V).
object(1,notuptodate        ,S,T,P,V) :- target(S,T,P,V), criterion(T,notuptodate),
                                         not delete(T,P,V), not maxversion(P,V).
object(1,sum(A)             ,S,T,P,V) :- target(S,T,P,V), criterion(T,sum(A)),
                                         not delete(T,P,V), attribute(P,V,A,X), X != 0.
object(1,unsat_recommends(D),S,T,P,V) :- target(S,T,P,V), criterion(T,unsat_recommends),
                                         not delete(T,P,V), recommends(P,V,D,_).
object(0,aligned(A,X,B,Y)   ,S,T,P,V) :- target(S,T,P,V), criterion(T,aligned(A,B)),
                                         keyval(T,A,X,B), attribute(P,V,A,X), attribute(P,V,B,Y),
                                         not static(T,aligned(A,X,B,Y)).

remove(C,P) :- object(_,C,sub,removed,P,_).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% analyze objectives' conflict cliques %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

conClique(I,C,P) :- remClique(I,P), remove(C,P).
conClique(I,C)   :- conClique(I,C,_).

mutClique(I,C)   :- conClique(I,C), 2 { conClique(I,C,_) }.
mutClique(I,C,P) :- conClique(I,C,P), mutClique(I,C).

subClique(I,C,sub,removed,P,V) :- unit(P,V,in), conClique(I,C,P).
subClique(I,C,S,T        ,P,V) :- object(1,C,S,T,P,V), realClique(I,P,V), T != removed.
subClique(I,C,S,T)             :- subClique(I,C,S,T,_,_).

resClique(I,C,S,T)     :- subClique(I,C,S,T), 2 { subClique(I,C,S,T,_,_) }.
resClique(I,C,S,T,P,V) :- subClique(I,C,S,T,P,V), resClique(I,C,S,T).
resClique(C,S,T,P,V)   :- resClique(_,C,S,T,P,V).

numClique(I,C,S,T,N) :- resClique(I,C,S,T), N = { resClique(I,C,S,T,_,_) }.

simClique(I,C1,S1,T1,C2,S2,T2) :- numClique(I,C1,S1,T1,N), numClique(I,C2,S2,T2,N),
                                  (C1,S1,T1) < (C2,S2,T2),
                                  resClique(I,C1,S1,T1,P,V) : resClique(I,C2,S2,T2,P,V).

mapClique(I,C1,S1,T1,C1,S1,T1) :- resClique(I,C1,S1,T1),
                                  not simClique(I,C,S,T,C1,S1,T1) : simClique(I,C,S,T,C1,S1,T1).
mapClique(I,C1,S1,T1,C2,S2,T2) :- simClique(I,C1,S1,T1,C2,S2,T2), mapClique(I,C1,S1,T1,C1,S1,T1).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% calculate constituents of objectives %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% auxiliary predicates

cont(count;notuptodate).
spec(unsat_recommends(D),sub) :- resClique(_,unsat_recommends(D),sub,_).

order(F,Y,X) :- value(F,X;Y), |X| < |Y|, 0 < X*Y,
                |Z| <= |X| : value(F,Z) : |Z| < |Y| : 0 < Z*Y.
order(F,Y,0) :- value(F,Y), X*Y < 0 : value(F,X) : |X| < |Y|.

order(F) :- order(F,_,_).

plain(F) :- order(F), { order(F,_,_) } 1.

contribute(C,S,T,in(P,V),abs(N))            :- T != removed,
     quant(C,S,T,P,V,N), not resClique(C,S,T,P,V).
contribute(C,S,T,in(I,C1,S1,T1),abs(N))     :- T != removed, not spec(C,S),
     quant(C,S,T,P,V,N), resClique(I,C,S,T,P,V), mapClique(I,C1,S1,T1,C,S,T),
     plain(f(I,C,S,T)).
contribute(C,S,T,in(P,V),ord(N,f(I,C,S,T))) :- T != removed, not spec(C,S),
     quant(C,S,T,P,V,N), resClique(I,C,S,T,P,V), not plain(f(I,C,S,T)).

contribute(C,sub,removed,in(P),abs(N))                    :-
     quant(C,sub,removed,P,0,N), not resClique(I,C,sub,removed) : conClique(I,C,P).
contribute(C,sub,removed,in(I,C1,S1,T1),abs(N))           :- not spec(C,sub),
     quant(C,sub,removed,P,0,N), conClique(I,C,P), mapClique(I,C1,S1,T1,C,sub,removed),
     plain(f(I,C,sub,removed)) : mutClique(I,C,P).
contribute(C,sub,removed,in(P),ord(N,f(I,C,sub,removed))) :- not spec(C,sub),
     quant(C,sub,removed,P,0,N), mutClique(I,C,P), not plain(f(I,C,sub,removed)).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% count & notuptodate

quant(C,S  ,T      ,P,V,1) :- object(_,C,S,T,P,V), cont(C), T != removed.
quant(C,sub,removed,P,0,N) :- remove(C,P), cont(C), N = { object(_,C,sub,removed,P,_) }.

value(f(I,C,S  ,T)      ,1) :- resClique(I,C,S,T), cont(C), T != removed.
value(f(I,C,sub,removed),N) :- quant(C,sub,removed,P,0,N), cont(C), mutClique(I,C,P).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% sum(A)

quant(sum(A),S  ,T      ,P,V,X) :- object(_,sum(A),S,T,P,V), attribute(P,V,A,X), T != removed.
quant(sum(A),sub,removed,P,0,X) :- remove(sum(A),P),  X := [ attribute(P,V,A,Y) = Y :
                                                             object(_,sum(A),sub,removed,P,V) ].

value(f(I,sum(A),S   ,T)     ,X) :- quant(sum(A),S  ,T      ,P,V,X), T != removed,
                                    resClique(I,sum(A),S,T,P,V).
value(f(I,sum(A),sub,removed),X) :- quant(sum(A),sub,removed,P,0,X), mutClique(I,sum(A),P).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% unsat_recommends(D)

quant(unsat_recommends(D),S  ,T      ,P,V,W) :- object(_,unsat_recommends(D),S,T,P,V),
                                                recommends(P,V,D,W), T != removed.
quant(unsat_recommends(D),sub,removed,P,0,W) :- remove(unsat_recommends(D),P),
                                                W := [ recommends(P,V,D,U) = U :
                                                object(_,unsat_recommends(D),sub,removed,P,V) ].

resum(I,unsat_recommends(D),sub,changed,W) :- resClique(I,unsat_recommends(D),sub,changed),
                                              W := [ recommends(P,V,D,U) = U :
                                              resClique(I,unsat_recommends(D),sub,changed,P,V) ].
resum(I,unsat_recommends(D),sub,removed,W) :- resClique(I,unsat_recommends(D),sub,removed),
                                              W := [ recommends(P,V,D,U) = U :
                                              conClique(I,unsat_recommends(D),P) :
                                              object(_,unsat_recommends(D),sub,removed,P,V) ].

value(f(I,unsat_recommends(D),S,T),W) :-
  quant(unsat_recommends(D),S,T,P,V,W), resClique(I,unsat_recommends(D),S,T,P,V), S != sub.
value(f(I,unsat_recommends(D),sub,T),W) :-
  resum(I,unsat_recommends(D),sub,T,W).
value(f(I,unsat_recommends(D),sub,changed),W-U) :-
  resum(I,unsat_recommends(D),sub,changed,W), resClique(I,unsat_recommends(D),sub,changed,P,V),
  quant(unsat_recommends(D),sub,changed,P,V,U).
value(f(I,unsat_recommends(D),sub,removed),W-U) :-
  resum(I,unsat_recommends(D),sub,removed,W), mutClique(I,unsat_recommends(D),P),
  quant(unsat_recommends(D),sub,removed,P,0,U).

contribute(unsat_recommends(D),sub,T,in(I,C1,S1,T1),abs(W)) :-
  resum(I,unsat_recommends(D),sub,T,W), mapClique(I,C1,S1,T1,unsat_recommends(D),sub,T),
  plain(f(I,unsat_recommends(D),sub,T)).
contribute(unsat_recommends(D),sub,T,in(I,C1,S1,T1),
           ord(W,f(I,unsat_recommends(D),sub,T))) :-
  resum(I,unsat_recommends(D),sub,T,W), mapClique(I,C1,S1,T1,unsat_recommends(D),sub,T),
  not plain(f(I,unsat_recommends(D),sub,T)).
contribute(unsat_recommends(D),sub,changed,in(P,V),
           ord(W-U,f(I,unsat_recommends(D),sub,changed))) :-
  resum(I,unsat_recommends(D),sub,changed,W), resClique(I,unsat_recommends(D),sub,changed,P,V),
  quant(unsat_recommends(D),sub,changed,P,V,U), not plain(f(I,unsat_recommends(D),sub,changed)).
contribute(unsat_recommends(D),sub,removed,in(P),
           ord(W-U,f(I,unsat_recommends(D),sub,removed))) :-
  resum(I,unsat_recommends(D),sub,removed,W), mutClique(I,unsat_recommends(D),P),
  quant(unsat_recommends(D),sub,removed,P,0,U), not plain(f(I,unsat_recommends(D),sub,removed)).








%*
fact(I,C,S  ,T     ,1) :- simClique(I,C,S,T), C != aligned, S != removed.
fact(I,C,sub,remove,N) :- numClique(I,C,sub,remove,N).

cont(count;notuptodate).

contribute(C2,S2,T2,in(I,C1,S1,T1),N) :-
  mapClique(I,C1,S1,T1,C2,S2,T2), fact(I,C2,S2,T2,N), cont(C2).

contribute(sum(A),S2,T2,in(I,C1,S1,T1),N*X) :-
  mapClique(I,C1,S1,T1,sum(A),S2,T2), fact(I,C2,S2,T2,N),
  resClique(I,sum(A),S2,T2,X).

contribute(unsat_recommends,sub,changed,true,rec(D,(N-1)*W)) :-
  mapClique(I,C1,S1,T1,unsat_recommends,sub,changed),
  numClique(I,unsat_recommends,sub,changed,N),
  resClique(I,unsat_recommends,sub,changed,rec(D,W)).
contribute(unsat_recommends,S2,T2,in(I,C1,S1,T1),rec(D,N*W)) :-
  mapClique(I,C1,S1,T1,unsat_recommends,S2,T2), fact(I,C2,S2,T2,N),
  resClique(I,unsat_recommends,S2,T2,rec(D,W)).

contribute(aligned(A,B),S2,T2,in(I,C1,S1,T1),Z) :-
  mapClique(I,C1,S1,T1,aligned(A,B),S2,T2),
  resClique(I,aligned(A,B),S2,T2,Z).

contribute(C,S,T,in(I),Z) :- resClique(I,C,S,T,Z), not simClique(I,C,S,T).
contribute(C,S,T,in(I))   :- contribute(C,S,T,in(I),_).

contribute(C,S,T,in(P,V),Z) :- object(C,S,T,P,V,Z), not aligns(C,T,Z), not static(C,T,Z),
                               not clique(I,P,V) : clique(I,P,V).
contribute(C,S,T,in(P,V),Z) :- resClique(I,C,S,T,P,V,Z), not resClique(I,C,S,T).

mutual(C,sub,removed,in(I),P) :- contribute(C,sub,removed,in(I)),
                                 remClique(I,C,removed,P), remClique(I,P).
mutual(C,sub,removed,in(I))   :- contribute(C,sub,removed,in(I)),
                                 2 { mutual(C,sub,removed,in(I),_) }.



%%%%%%%%%%%%%%
% objectives %
%%%%%%%%%%%%%%

fact(minimize, 1).
fact(maximize,-1).
sign(sol, 1).
sign(sub,-1).

in(I,C,S,T) :- in(P,V), resClique(I,C,S,T,P,V), mapClique(I,C,S,T,C,S,T).





% count & notuptodate
#minimize[ in(P,V) = Z*F*G @ L :
           contribute(C,S,T,in(P,V),Z) : criterion(M,T,C,L) : fact(M,F) : sign(S,G) : cont(C),
           in(I,C,S,T) = Z*F*G @ L :
           contribute(C,S,T,in(I,C,S,T),Z) : criterion(M,T,C,L) : fact(M,F) : sign(S,G) : cont(C) ].
*%