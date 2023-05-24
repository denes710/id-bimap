#include "id_bimap.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

TEST(IdBimapTest, F0_types)
{
  string_id_bimap SM;
  EXPECT_TRUE((std::is_same_v<decltype(SM)::mapped_type, std::string>));

  struct T {};
  id_bimap<T> TM;
  EXPECT_TRUE(sizeof(decltype(TM)::key_type) == sizeof(&TM));
  EXPECT_TRUE((std::is_same_v<decltype(TM)::mapped_type, T>));

  id_bimap<T, short> STM;
  EXPECT_TRUE((std::is_same_v<decltype(STM)::key_type, short>));
  EXPECT_TRUE((std::is_same_v<decltype(STM)::mapped_type, T>));

  kchar_id_bimap<T> CTM;
  EXPECT_TRUE((std::is_same_v<decltype(CTM)::key_type, char>));
  EXPECT_TRUE((std::is_same_v<decltype(CTM)::mapped_type, T>));
}

TEST(IdBimapTest, F1_operations)
{
  string_id_bimap SM;

  EXPECT_TRUE(SM.size() == 0 && SM.empty());

  // Exceptions.
  try
  {
    auto S1 = SM["gsd"];
    EXPECT_TRUE(false && "Unreachable.");
  }
  catch (const std::domain_error&)
  {}

  try
  {
    auto S2 = SM[8];
    EXPECT_TRUE(false && "Unreachable.");
  }
  catch (const std::out_of_range&)
  {}

  // Insert and query.
  auto IR1 = SM.insert("gsd");
  EXPECT_TRUE(IR1.second);
  EXPECT_TRUE(IR1.first->first == 0 && IR1.first->second == "gsd");

  auto S1 = SM[0];
  EXPECT_TRUE(S1 == "gsd");
  auto S2 = SM["gsd"];
  EXPECT_TRUE(S2 == 0);

  EXPECT_TRUE(SM.size() == 1 && !SM.empty());

  // Support equality-based lookup, not identity-based.
  std::string W{"Whisperity"};

  auto IR2 = SM.insert(W);
  EXPECT_TRUE(IR2.second && IR2.first->first == 1 && IR2.first->second == W &&
         IR2.first->second == "Whisperity");
  // Idempotence.
  EXPECT_TRUE(SM[SM[W]] == W);
  EXPECT_TRUE(SM[SM[1]] == 1);
  try
  {
    SM[2];
    EXPECT_TRUE(false && "Unreachable.");
  }
  catch (const std::out_of_range&)
  {}

  EXPECT_TRUE(SM.size() == 2);

  // Non-unique insertion.
  std::string G{"gsd"};
  auto IR3 = SM.insert(G);
  EXPECT_TRUE(IR3.second == false && IR3.first->first == 0 &&
         IR3.first->second == "gsd");
  EXPECT_TRUE(SM.size() == 2);

  // Copy.
  const string_id_bimap CSM = SM;
  {
      string_id_bimap SM2 = SM;
      SM2.clear();
      EXPECT_TRUE(SM2.size() == 0 && SM2.empty());
  }

  // Proper copy!
  EXPECT_TRUE(CSM.size() == 2 && !CSM.empty() && CSM["gsd"] == 0 &&
         CSM["Whisperity"] == 1);
  try
  {
    CSM[CSM["Xazax"]];
    EXPECT_TRUE(false && "Unreachable.");
  } catch (const std::domain_error&) {}

  // Find.
  EXPECT_TRUE(CSM.find("Whisperity") != CSM.end());
  EXPECT_TRUE(CSM.find("Xazax") == CSM.end());
  EXPECT_TRUE(CSM.find("gsd") != CSM.end() && CSM.find("gsd")->second == "gsd");

  // Foreach iterator.
  unsigned short Idx = 0;
  for (const auto& E : CSM)
  {
    if (Idx == 0 && E.first == Idx)
    {
      ++Idx;
      EXPECT_TRUE(E.second == G);
    }
    else if (Idx == 1 && E.first == Idx)
    {
      ++Idx;
      EXPECT_TRUE(E.second == W);
    }
    else
    {
      EXPECT_TRUE(false && "Expected only 2 elements in the copy!");
    }
  }

  // Erase.
  SM.erase("gsd");
  EXPECT_TRUE(SM.size() == 1 && SM[1] == "Whisperity");
  SM.erase(1);
  EXPECT_TRUE(SM.empty());
  EXPECT_TRUE(CSM.size() == 2);

  // Initialisation list.
  string_id_bimap SMInit = {"gsd", "Whisperity", "Bjarne", "Herb"};
  std::ostringstream OSS;
  for (const auto& E : SMInit)
    OSS << E.second << ", ";
  EXPECT_TRUE(OSS.str() == "gsd, Whisperity, Bjarne, Herb, ");
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

TEST(IdBimapTest, F2_advanced)
{
  SMFCounter::reset();

  id_bimap<SMFCounter> SMFM;
  auto ER1 = SMFM.emplace(8); // +1 construction.
  EXPECT_TRUE(SMFM.size() == 1 && ER1.second == true && ER1.first->first == 0 &&
         ER1.first->second.id() == 8);
  EXPECT_TRUE(SMFCounter::Ctor == 1 && SMFCounter::CCtor == 0 &&
         SMFCounter::MCtor == 0 && SMFCounter::CAsg == 0 &&
         SMFCounter::MAsg == 0 && SMFCounter::Dtor == 0);

  EXPECT_TRUE(SMFM.find(4) == SMFM.end()); // +1 construction, +1 destruction.
  EXPECT_TRUE(SMFCounter::Ctor == 2 && SMFCounter::CCtor == 0 &&
         SMFCounter::MCtor == 0 && SMFCounter::CAsg == 0 &&
         SMFCounter::MAsg == 0 && SMFCounter::Dtor == 1);

  EXPECT_TRUE(SMFM[0u].id() == 8); // Index-based lookup, no ctor/dtor.
  EXPECT_TRUE(SMFCounter::Ctor == 2 && SMFCounter::CCtor == 0 &&
         SMFCounter::MCtor == 0 && SMFCounter::CAsg == 0 &&
         SMFCounter::MAsg == 0 && SMFCounter::Dtor == 1);

  SMFM.clear(); // +1 destruction.
  EXPECT_TRUE(SMFM.size() == 0 && SMFM.empty());
  EXPECT_TRUE(SMFCounter::Ctor == 2 && SMFCounter::CCtor == 0 &&
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
  EXPECT_TRUE(USM.size() == 1 && ER2.second == true &&
         (*ER2.first->second) == "Xazax");

  for (int I = 0; I < 64; ++I)
  {
    auto ER3 = USM.emplace(std::make_unique<std::string>(std::to_string(I)));
    EXPECT_TRUE(ER3.second == true);
  }
  EXPECT_TRUE(USM.size() == 1 + 64);

  const auto& CUSM = USM;
  auto FIR1 = CUSM.find_if([](auto&& E) -> bool { return *E == "Xazax"; });
  EXPECT_TRUE(FIR1 == CUSM.begin());
  EXPECT_TRUE(FIR1 != CUSM.end());

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
  EXPECT_TRUE(USM.size() == 1 + 32);

  int IndexAccumulator = 0;
  std::ostringstream OSS;
  for (const auto& E : USM)
  {
    IndexAccumulator += E.first; // 0 + 1 + 2 + ... + 32
    OSS << *E.second << ", ";
  }

  EXPECT_TRUE(IndexAccumulator == 1024);
  EXPECT_TRUE(OSS.str() ==
         "Xazax, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, "
         "32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, ");
}

TEST(IdBimapTest, F3_logicalDelete)
{
  {
    id_bimap<char, short> SCM;
    EXPECT_TRUE((std::is_same_v<decltype(SCM.next_index()), short>));
  }

  string_id_bimap SM;

  EXPECT_TRUE(SM.size() == 0 && SM.capacity() == 0 &&
         SM.next_index() == 0); // No elements yet, so 1st element is index 0.

  SM.insert("gsd");
  SM.insert("Whisperity");
  SM.insert("John");
  SM.insert("Hyrum");

  EXPECT_TRUE(SM.size() == 4 && SM.capacity() == 4);
  EXPECT_TRUE(SM.next_index() == 4); // [0, 1, 2, 3] are allocated indices.
  EXPECT_TRUE(SM.is_contiguous());

  SM.erase("gsd");

  EXPECT_TRUE(SM.size() == 3 && SM.capacity() == 4 && SM.next_index() == 0);
  EXPECT_TRUE(!SM.is_contiguous());

  try
  {
    SM["gsd"];
    EXPECT_TRUE(false && "Unreachable.");
  } catch (const std::domain_error&) {}

  SM.delete_all([](auto&& E) -> bool
  {
    EXPECT_TRUE(E != "gsd" && "Deleted element remained in iteration?");
    return false; // Don't delete anything!
  });

  SM.erase("Bryce"); // Deleting non-existent element.
  EXPECT_TRUE(!SM.is_contiguous() && SM.size() == 3 && SM.capacity() == 4 &&
         SM.next_index() == 0);

  SM.erase("John");
  EXPECT_TRUE(SM.next_index() == 0); // [-, 1, -, 3] are allocated indices.
  EXPECT_TRUE(SM.size() == 2 && SM.capacity() == 4);
  EXPECT_TRUE(!SM.is_contiguous());

  try
  {
    SM[2];
    EXPECT_TRUE(false && "Unreachable.");
  } catch (const std::out_of_range&) {}

  auto IR1 = SM.insert("Hyrum");
  EXPECT_TRUE(IR1.second == false && IR1.first->first == 3);

  auto IR2 = SM.insert("Bjarne");
  EXPECT_TRUE(IR2.second == true &&
         IR2.first->first == 0); // Insert at first "good" hole.
  EXPECT_TRUE(SM.size() == 3 && SM.capacity() == 4);
  EXPECT_TRUE(!SM.is_contiguous());

  EXPECT_TRUE(SM.next_index() == 2);
  auto IR3 = SM.insert("Herb");
  EXPECT_TRUE(IR3.second == true &&
         IR3.first->first == 2); // Insert at first "good" hole.
  EXPECT_TRUE(SM.is_contiguous());
  EXPECT_TRUE(SM.capacity() == 4);

  SM.insert("Alexandrescu");
  EXPECT_TRUE(SM.size() == 5 && SM.next_index() == 5 && SM.capacity() == 5);
  EXPECT_TRUE(SM.is_contiguous());

  id_bimap<SMFCounter> SMFM;
  SMFM.emplace(1);
  SMFM.emplace(2);

  SMFCounter::reset();
  SMFM.erase(0);
  EXPECT_TRUE(SMFM.size() == 1 && SMFM.capacity() == 2);
  EXPECT_TRUE(SMFCounter::Ctor == 0 && SMFCounter::CCtor == 0 &&
         SMFCounter::MCtor == 0 && SMFCounter::CAsg == 0 &&
         SMFCounter::MAsg == 0 && SMFCounter::Dtor == 1 &&
         "erase() should destroy removed elements!");
}

TEST(IdBimapTest, F3b_reserve)
{
  string_id_bimap SM;
  SM.reserve(4);

  EXPECT_TRUE(SM.size() == 0 && SM.capacity() == 4);

  SM.insert("gsd");        // 0
  SM.insert("Whisperity"); // 1
  SM.insert("Herb");       // 2
  SM.insert("Xazax");      // 3
  SM.insert("Bryce");      // 4

  EXPECT_TRUE(SM.size() == 5 && SM.capacity() == 5);

  SM.erase("Herb");
  EXPECT_TRUE(SM.size() == 4 && SM.capacity() == 5);
  try
  {
    SM[2];
    EXPECT_TRUE(false && "Unreachable.");
  } catch (const std::out_of_range&) {}

  auto S1 = SM[0];
  auto S2 = SM[3];

  // Valid: [0, 1, -, 3, 4, -]

  SM.reserve(3); // Nothing happens, as there would be elements AFTER the
                 // potentially shrinked size!
  EXPECT_TRUE(SM.size() == 4 && SM.capacity() == 5);
  // Valid: [0, 1, -, 3, 4, -]

  SM.reserve(8);
  EXPECT_TRUE(SM.size() == 4 && SM.capacity() == 8);
  // Valid: [0, 1, -, 3, 4, -, -, -]

  SM.reserve(5); // prolematic
  // Valid: [0, 1, -, 3, 4]
  EXPECT_TRUE(SM.size() == 4 && SM.capacity() == 5);

  SMFCounter::reset();
  id_bimap<SMFCounter> SMFM;
  SMFM.reserve(1024);
  EXPECT_TRUE(SMFM.size() == 0 && SMFM.capacity() == 1024);
  EXPECT_TRUE(SMFCounter::Ctor == 0 && SMFCounter::CCtor == 0 &&
         SMFCounter::MCtor == 0 && SMFCounter::CAsg == 0 &&
         SMFCounter::MAsg == 0 && SMFCounter::Dtor == 0 &&
         "reserve() should not directly construct any elements!");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}