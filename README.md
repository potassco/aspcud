Aspcud - A solver for package problems in CUDF format using ASP
---------------------------------------------------------------

The aspcud project provides the converter/preprocessor `cudf2lp` to translate a
CUDF specification into a set of facts. These facts together with an encoding
can then be passed to an ASP grounder and solver to solve the package problem.
The small C program `aspcud` takes care of this, calling the necessary tools
and printing the result in CUDF format.

To run aspcud, use the installed aspcud binary (take a look at the examples
folder to get started):

    aspcud problem.cudf solution.cudf <criteria>

For this to work, clasp and gringo have to be installed.  Solver and grounder
are available at:

- https://github.com/potassco/clasp
- https://github.com/potassco/clingo

Check the [installation instructions](INSTALL.md) for more details.

The specification of CUDF documents and the supported criteria along with a lot
of instances are available at the mancoosi homepage:

- http://www.mancoosi.org/
- http://www.mancoosi.org/cudf/
- http://www.mancoosi.org/misc-2012/criteria/
- http://data.mancoosi.org/misc2012/problems/

Either the encoding `misc2012.lp` or `specification.lp` can be used for solving
the package problem (can be selected with option `-e`). The first encoding can
typically solve harder problems than the second one. The second encoding is
often faster on simpler instance though.

Aspcud is distributed under the [MIT Licence](LICENSE.md).
