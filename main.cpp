#include "id_bimap.h"

#include <iostream>
#include <string>
#include <type_traits>

void F0_types()
{
#ifndef NFAIL
  // EXPECT FAIL: Template parameter "value" must always be specified.
  id_bimap<> A;

  // EXPECT FAIL: Template parameter "value" must always be specified.
  kchar_id_bimap<> B;

  // EXPECT FAIL: Key must be integer!
  id_bimap<std::string, float> F;

  // EXPECT FAIL: Key and value must be separate types.
  id_bimap<int, int> I;
  id_bimap<std::string, std::string> S;
  id_bimap<const std::string, std::string> S2;
#endif

  string_id_bimap SM;
  static_assert(std::is_same_v<decltype(SM)::mapped_type, std::string>);

  struct T {};
  id_bimap<T> TM;
  static_assert(sizeof(decltype(TM)::key_type) == sizeof(&TM));
  static_assert(std::is_same_v<decltype(TM)::mapped_type, T>);

  id_bimap<T, short> STM;
  static_assert(std::is_same_v<decltype(STM)::key_type, short>);
  static_assert(std::is_same_v<decltype(STM)::mapped_type, T>);

  kchar_id_bimap<T> CTM;
  static_assert(std::is_same_v<decltype(CTM)::key_type, char>);
  static_assert(std::is_same_v<decltype(CTM)::mapped_type, T>);
}

int main()
{
    F0_types();
    std::cout << "Test end!" << std::endl;

    return 0;
}