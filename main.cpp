#include "id_bimap.h"

#include <iostream>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <sstream>

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

void F1_operations()
{
  string_id_bimap SM;

  assert(SM.size() == 0 && SM.empty());

  // Exceptions.
  try
  {
    auto S1 = SM["gsd"];
    assert(false && "Unreachable.");
  } catch (const std::domain_error&) {}

  try
  {
    auto S2 = SM[8];
    assert(false && "Unreachable.");
  } catch (const std::out_of_range&) {}

  // Insert and query.
  auto IR1 = SM.insert("gsd");
  assert(IR1.second);
  assert(IR1.first->first == 0 && IR1.first->second == "gsd");

  auto S1 = SM[0];
  assert(S1 == "gsd");
  auto S2 = SM["gsd"];
  assert(S2 == 0);

  assert(SM.size() == 1 && !SM.empty());

// TODO
#ifndef NFAIL
  // EXPECT FAIL: Do not allow update through operator[].
  SM[0] = "not-gsd";
#endif

  // Support equality-based lookup, not identity-based.
  std::string W{"Whisperity"};

  auto IR2 = SM.insert(W);
  assert(IR2.second && IR2.first->first == 1 && IR2.first->second == W &&
         IR2.first->second == "Whisperity");
  // Idempotence.
  assert(SM[SM[W]] == W);
  assert(SM[SM[1]] == 1);
  try
  {
    SM[2];
    assert(false && "Unreachable.");
  } catch (const std::out_of_range&) {}

  assert(SM.size() == 2);

  // Non-unique insertion.
  std::string G{"gsd"};
  auto IR3 = SM.insert(G);
  assert(IR3.second == false && IR3.first->first == 0 &&
         IR3.first->second == "gsd");
  assert(SM.size() == 2);

  // Copy.
  const string_id_bimap CSM = SM;
  {
      string_id_bimap SM2 = SM;
      SM2.clear();
      assert(SM2.size() == 0 && SM2.empty());
  }

  // Proper copy!
  assert(CSM.size() == 2 && !CSM.empty() && CSM["gsd"] == 0 &&
         CSM["Whisperity"] == 1);
  try
  {
    CSM[CSM["Xazax"]];
    assert(false && "Unreachable.");
  } catch (const std::domain_error&) {}

  // Find.
  assert(CSM.find("Whisperity") != CSM.end());
  assert(CSM.find("Xazax") == CSM.end());
  assert(CSM.find("gsd") != CSM.end() && CSM.find("gsd")->second == "gsd");

  // Foreach iterator.
  unsigned short Idx = 0;
  for (const auto& E : CSM)
  {
    if (Idx == 0 && E.first == Idx)
    {
      ++Idx;
      assert(E.second == G);
    }
    else if (Idx == 1 && E.first == Idx)
    {
      ++Idx;
      assert(E.second == W);
    } else
      assert(false && "Expected only 2 elements in the copy!");
  }

  // Erase.
  SM.erase("gsd");
  assert(SM.size() == 1 && SM[0] == "Whisperity");
  SM.erase(0);
  assert(SM.empty());
  assert(CSM.size() == 2);

  // Initialisation list.
  string_id_bimap SMInit = {"gsd", "Whisperity", "Bjarne", "Herb"};
  std::ostringstream OSS;
  for (const auto& E : SMInit)
    OSS << E.second << ", ";
  assert(OSS.str() == "gsd, Whisperity, Bjarne, Herb, ");
}

// IGNORE! Helper class for testing.
struct SMFCounter
{
  static std::size_t Ctor, CCtor, MCtor, CAsg, MAsg, Dtor;
  static void reset() { Ctor = CCtor = MCtor = CAsg = MAsg = Dtor = 0; }

  SMFCounter() : ID(0) { ++Ctor; }
  SMFCounter(int V) : ID(V) { ++Ctor; }
  SMFCounter(const SMFCounter& R) : ID(R.ID) { ++CCtor; }
  SMFCounter(SMFCounter&& R) : ID(R.ID) { ++MCtor; }
  SMFCounter &operator=(const SMFCounter& R)
  {
    ID = R.ID;
    ++CAsg;
    return *this;
  }
  SMFCounter &operator=(SMFCounter&& R)
  {
    ID = R.ID;
    ++MAsg;
    return *this;
  }
  ~SMFCounter() { ++Dtor; }

  int id() const { return ID; }

  bool operator==(const SMFCounter& R) const { return ID == R.ID; }
  bool operator!=(const SMFCounter& R) const { return ID != R.ID; }
  bool operator<(const SMFCounter& R) const { return ID < R.ID; }
  bool operator<=(const SMFCounter& R) const { return ID <= R.ID; }
  bool operator>=(const SMFCounter& R) const { return ID >= R.ID; }
  bool operator>(const SMFCounter& R) const { return ID > R.ID; }

private:
  int ID = 0;
};
std::size_t SMFCounter::Ctor, SMFCounter::CCtor, SMFCounter::MCtor,
    SMFCounter::CAsg, SMFCounter::MAsg, SMFCounter::Dtor;

// IGNORE! Helper type: something that's nice enough not to be copyable,
// but useful without a lot of hassle.
using non_copyable = std::unique_ptr<std::string>;

void F2_advanced()
{
  SMFCounter::reset();

  id_bimap<SMFCounter> SMFM;
  auto ER1 = SMFM.emplace(8); // +1 construction.
  assert(SMFM.size() == 1 && ER1.second == true && ER1.first->first == 0 &&
         ER1.first->second.id() == 8);
  assert(SMFCounter::Ctor == 1 && SMFCounter::CCtor == 0 &&
         SMFCounter::MCtor == 0 && SMFCounter::CAsg == 0 &&
         SMFCounter::MAsg == 0 && SMFCounter::Dtor == 0);

  assert(SMFM.find(4) == SMFM.end()); // +1 construction, +1 destruction.
  assert(SMFCounter::Ctor == 2 && SMFCounter::CCtor == 0 &&
         SMFCounter::MCtor == 0 && SMFCounter::CAsg == 0 &&
         SMFCounter::MAsg == 0 && SMFCounter::Dtor == 1);

  assert(SMFM[0u].id() == 8); // Index-based lookup, no ctor/dtor.
  assert(SMFCounter::Ctor == 2 && SMFCounter::CCtor == 0 &&
         SMFCounter::MCtor == 0 && SMFCounter::CAsg == 0 &&
         SMFCounter::MAsg == 0 && SMFCounter::Dtor == 1);

  SMFM.clear(); // +1 destruction.
  assert(SMFM.size() == 0 && SMFM.empty());
  assert(SMFCounter::Ctor == 2 && SMFCounter::CCtor == 0 &&
         SMFCounter::MCtor == 0 && SMFCounter::CAsg == 0 &&
         SMFCounter::MAsg == 0 && SMFCounter::Dtor == 2);

  id_bimap<non_copyable> USM;

#ifndef NFAIL
  // EXPECT FAIL: The mapped type is not copyable!
  id_bimap<non_copyable> USMC = USM;
#endif

  {
    id_bimap<non_copyable> X;
    // EXPECT PASS: The mapped type is moveable.
    id_bimap<non_copyable> USMM = std::move(X);
  }

  auto ER2 = USM.emplace(std::make_unique<std::string>("Xazax"));
  assert(USM.size() == 1 && ER2.second == true &&
         (*ER2.first->second) == "Xazax");

  for (int I = 0; I < 64; ++I)
  {
    auto ER3 = USM.emplace(std::make_unique<std::string>(std::to_string(I)));
    assert(ER3.second == true);
  }
  assert(USM.size() == 1 + 64);

  const auto& CUSM = USM;
  auto FIR1 = CUSM.find_if([](auto&& E) -> bool { return *E == "Xazax"; });
  assert(FIR1 == CUSM.begin());
  assert(FIR1 != CUSM.end());

  USM.delete_all([](auto&& E) -> bool
  {
    try
    {
      int NumericValue = std::stoi(*E);
      if (NumericValue % 2 == 1) // Odd number.
        return true;
      return false;
    }
    catch (const std::invalid_argument&) { return false; }
    catch (const std::out_of_range&) { return false; }

    return false;
  });

  // 1->64 has 32 even numbers. The "Xazax" shall be ignored by delete.
  assert(USM.size() == 1 + 32);

  int IndexAccumulator = 0;
  std::ostringstream OSS;
  for (const auto& E : USM)
  {
    IndexAccumulator += E.first; // 0 + 1 + 2 + ... + 32
    OSS << *E.second << ", ";
  }
  assert(IndexAccumulator == ((32 * (32 + 1)) / 2));
  assert(OSS.str() ==
         "Xazax, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, "
         "32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, ");
}

int main()
{
    F0_types();
    F1_operations();
    F2_advanced();
    std::cout << "Test end!" << std::endl;

    return 0;
}