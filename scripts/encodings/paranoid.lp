% Note: encoding optimized for the paranoid criteria

{ in(P,V) } :- unit(P,V,in).

forbidden(D) :- in(P,V), conflict(P,V,D).
requested(D) :- in(P,V), depends(P,V,D).
satisfied(D) :- in(P,V), satisfies(P,V,D).

:-   request(D), not satisfied(D).
:- requested(D), not satisfied(D).
:- forbidden(D),     satisfied(D).

in(P)        :- in(P,_).
installed(P) :- installed(P,_).

%changed(P,V,1)   :- not in(P,V),     installed(P,V).
%changed(P,V,1)   :-     in(P,V), not installed(P,V).
%removed(P,1,W) :- installed(P), not in(P), W = { installed(P,V) }.

% the two above optimized for cliques of size two ...
clique(I)   :- clique(I,_,_).
clique(P,V) :- clique(_,P,V).

rclique(I)     :- clique(I), 2 { clique(I,P,V) : installed(P,V) } 2.
rclique(I,P,V) :- rclique(I), clique(I,P,V), installed(P,V).
rclique(I,P)   :- rclique(I,P,_).

rinstalled(P,V) :- not rclique(I) : clique(I,P,V), installed(P,V).
rinstalled(P)   :- rinstalled(P,_).

%*
changed(I,1,1) :-     in(P,V), not installed(P,V),     clique(I,P,V).
changed(P,V,1) :-     in(P,V), not installed(P,V), not clique(P,V).
changed(P,V,1) :- not in(P,V),    rinstalled(P,V).
changed(I,2,1) :- not in(P,V), rclique(I,P,V).
changed(I,3,1) :- not in(P,V) : rclique(I,P,V), rclique(I).
*%

inclique(I) :- clique(I,P,V), in(P,V).
ninstalled(I,N) :- clique(I), N = { installed(P,V) : clique(I,P,V) }.

changed(P,V,1)   :- not in(P,V),     installed(P,V), not clique(P,V).
changed(P,V,1)   :-     in(P,V), not installed(P,V), not clique(P,V).
changed(I,1,N-1) :- ninstalled(I,N), N > 1.
changed(I,2,1)   :- clique(I), not inclique(I), ninstalled(I,N), N > 0.
changed(I,2,1)   :- clique(I,P,V), in(P,V), not installed(P,V), ninstalled(I,N), N > 0.
changed(I,3,1)   :- clique(I,P,V), in(P,V), not installed(P,V).

removed(P,1,W) :- not in(P), rinstalled(P), W = { rinstalled(P,V) }.
removed(I,2,1) :- not in(P), rclique(I,P).
removed(I,3,1) :- not in(P) : rclique(I,P), rclique(I).

#minimize [ removed(P,X,W) = W @ 2 ].
#minimize [ changed(P,V,W) = W @ 1 ].

% output projection

#hide.
#show in/2.
