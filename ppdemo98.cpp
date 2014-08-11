/* This is a demonstration of prettyprint.hpp library.
   We define a few containers and print them.
   Some customization options are demonstrated, too.
*/

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <tr1/array>
#include <tr1/tuple>

#include "prettyprint98.hpp"

/* Customization option 1: Direct partial/full specialization.
   Here we specialize for std::vector<double>.
*/
template<> const pretty_print::delimiters_values<char> pretty_print::delimiters<std::vector<double>, char>::values = { "|| ", " : ", " ||" };

/* Customization option 2: Create a delimiters class for later use and reuse. */
struct MyDelims { static const pretty_print::delimiters_values<char> values; };
const pretty_print::delimiters_values<char> MyDelims::values = { "<", "; ", ">" };


/* Demo: run with a couple of command-line arguments. */

int main(int argc, char * argv[])
{
  std::string cs;
  std::tr1::unordered_map<int, std::string> um;
  std::map<int, std::string> om;
  std::tr1::unordered_set<std::string> ss;
  std::vector<std::string> v;
  std::vector<std::vector<std::string> > vv;
  std::vector<std::pair<int, std::string> > vp;
  std::vector<double> vd;
  v.reserve(argc - 1);
  vv.reserve(argc - 1);
  vp.reserve(argc - 1);

  std::cout << "Printing pairs." << std::endl;

  for (int i = 1; i < argc; ++i)
  {
    std::string s(argv[i]);
    std::pair<int, std::string> p(i, s);

    um[i] = s;
    om[i] = s;
    v.push_back(s);
    vv.push_back(v);
    vp.push_back(p);
    vd.push_back(1./double(i));
    ss.insert(s);
    cs += s;

    /* Demo: pretty-printing for std::pair<S, T> */
    std::cout << "  " << p << std::endl;
  }

  /* Demo: pretty-printing for various containers. */

  std::tr1::array<char, 5> a; a[0] = 'h'; a[1] = 'e'; a[2] = a[3] = 'l'; a[4] = 'o';

  std::cout << "Vector: " << v << std::endl                // vector of strings
            << "Incremental vector: " << vv << std::endl   // nestes vector of vectors
            << "Pairs: " << vp << std::endl                // vector of pairs
            << "Another vector: " << vd << std::endl       // vector of doubles using Customization #1
            << "Set: " << ss << std::endl                  // set of strings, using partially specialized default set delimiters
            << "OMap: " << om << std::endl                 // associative container (value type is a std::pair)
            << "UMap: " << um << std::endl                 // ditto
            << "String: " << cs << std::endl               // just a plain string, note that std::string has begin()/end()
            << "Array: " << a << std::endl                 // an std::array
  ;

  /* Demo: Here we use our reusable delimiter class MyDelims by directly accessing some interna. */
  std::cout << pretty_print::print_container_helper<std::vector<std::string>, char, std::char_traits<char>, MyDelims>(v) << std::endl;

  /* Demo: Here we achieve the same using a type-erasing helper class. */
  std::cout << pretty_print::custom_delims<MyDelims>(v) << std::endl;

  /* Demo: We can pretty-print std::pair and std::tuple.
     (You already saw pairs in the associative containers above.)
  */
  std::pair<std::string, int> a1 = std::make_pair(std::string("Jello"), 9);
  //std::tr1::tuple<int> a2 = std::tr1::make_tuple(1729);
  std::tr1::tuple<std::string, std::tr1::tuple<std::string, int>, int> a3 = std::tr1::make_tuple("Qrgh", a1, 11);
  std::tr1::tuple<int, int, std::pair<double, std::string> > a4 = std::tr1::make_tuple(1729, 2875, std::pair<double, std::string>(1.5, "abc"));


  /* Demo: raw arrays can be printed with a helper wrapper. */
  int arr[] = { 1, 4, 9, 16 };
  int err[] = { 2 };

  /* For dynamic arrays, we need a wrapper. */
  int * drr = new int[3]; drr[0] = drr[1] = drr[2] = 8;

  std::cout << "Static C array direct: " << arr << std::endl
            << "Static C array direct: " << err << std::endl
            << "C dynamic array: " << pretty_print_array(drr, 3) << std::endl
            << "Pair:    " << a1 << std::endl
    //<< "1-tuple: " << a2 << std::endl
    //<< "n-tuple: " << a3 << std::endl
    //<< "n-tuple: " << a4 << std::endl
  ;

  delete[] drr;
}
