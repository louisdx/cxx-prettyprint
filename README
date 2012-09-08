cxx-prettyprint
===============

A pretty printing library for C++ containers.

Synopsis:
  Simply by including this header-only library in your source file,
  you can say "std::cout << x" for any container object x. Sensible
  defaults are provided, but the behaviour (i.e. the delimiters) are
  compile-time customizable to a great extent via partial specializiation.

Usage:
  Just add "#include "prettyprint.hpp" to your source file and make sure
  that prettyprint.hpp is findable.

Language requirements: C++0x for prettyprint.hpp, C++98/03 for prettyprint98.hpp

Example:
  Some usage examples are provided by ppdemo.cpp.

  Using GCC, compile with
    g++ -W -Wall -pedantic -O2 -s ppdemo.cpp -o ppdemo -std=c++0x 
    g++ -W -Wall -pedantic -O2 -s ppdemo98.cpp -o ppdemo98

For the C++98/03-version, define "NO_TR1" to prevent any inclusion of
TR1 headers and to disable std::tr1::tuple support.

For details, please see the website (http://louisdx.github.com/cxx-prettyprint/).

License: Boost Software License, Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.
