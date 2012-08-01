% reify package relations

relate(P,V,con,D) :- unit(P,V,in), conflict(P,V,D).
relate(P,V,dep,D) :- unit(P,V,in), depends(P,V,D).
relate(P,V,sat,D) :- unit(P,V,in), satisfies(P,V,D).
% ADD recommendations as required

% analyze clique dependencies

clique(I) :- clique(I,_,_).

realClique(I)     :- clique(I), 2 { unit(P,V,in) : clique(I,P,V) }.
realClique(I,P,V) :- realClique(I), clique(I,P,V), unit(P,V,in).
% realClique(I,N)   :- realClique(I), N = { realClique(I,_,_) }.

relaClique(I,P,V,R,D) :- realClique(I,P,V), relate(P,V,R,D).
relaClique(I,R,D)     :- relaClique(I,_,_,R,D).
relaClique(I,R,D,N)   :- relaClique(I,R,D), N = { relaClique(I,_,_,R,D) }, 1 < N.

equiClique(I,R1,D1,R2,D2) :- relaClique(I,R1,D1,N), relaClique(I,R2,D2,N), (R1,D1) < (R2,D2),
                             relaClique(I,P,V,R1,D1) : relaClique(I,P,V,R2,D2).

mapsClique(I,R,D,R,D)     :- relaClique(I,R,D,_),
                             not equiClique(I,RR,DD,R,D) : equiClique(I,RR,DD,R,D).
mapsClique(I,R1,D1,R2,D2) :- mapsClique(I,R1,D1,R1,D1), equiClique(I,R1,D1,R2,D2).

dropClique(P,V,R,D) :- mapsClique(I,_,_,R,D), relaClique(I,P,V,R,D).

% generate solution

{ in(P,V) } :- unit(P,V,in).

activeClique(I,R,D) :- mapsClique(I,R,D,R,D), relaClique(I,P,V,R,D), in(P,V).

active(R,D) :- activeClique(I,RR,DD), mapsClique(I,RR,DD,R,D).
active(R,D) :- relate(P,V,R,D), not dropClique(P,V,R,D).

 :- active(con,D),     active(sat,D).
 :- active(dep,D), not active(sat,D).
 :-    request(D), not active(sat,D).
