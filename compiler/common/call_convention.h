#pragma once
#include <asmjit/a64.h>
#include <asmjit/x86.h>

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <type_traits>

namespace wasmcc {

// Registers come in 2 flavors, general purpose or
enum class RegGroup : uint8_t { kGp = 0, kVec };

// clang-format off
template <typename T>
concept GpReg = std::same_as<T, asmjit::x86::Gp> || std::same_as<T, asmjit::a64::Gp>;
template <typename T>
concept VecReg = std::same_as<T, asmjit::x86::Vec> || std::same_as<T, asmjit::a64::Vec>;
template <typename T>
concept AnyReg = GpReg<T> || VecReg<T>;
// clang-format on

/* A set of register, using a bitset. */
template <AnyReg T>
class RegisterMask {
 private:
  // A given register group (general purpose or vector) has only at most 32
  // registers in it.
  static constexpr size_t kMaxRegGroupNumRegisters = 32;
  using UnderlyingMask = std::bitset<kMaxRegGroupNumRegisters>;

 public:
  RegisterMask() noexcept = default;
  RegisterMask(std::initializer_list<T> regs) noexcept {
    for (const auto& reg : regs) {
      Set(reg);
    }
  };

  void Set(const T& reg) noexcept { _mask.set(reg.id()); }
  void Reset(const T& reg) noexcept { _mask.reset(reg.id()); }
  void Reset() noexcept { _mask.reset(); }
  bool Test(const T& reg) const noexcept { return _mask.test(reg.id()); }

  class ConstIterator {
   public:
    using iterator_category = std::forward_iterator_tag;

    explicit ConstIterator(const UnderlyingMask& mask) noexcept : _mask(mask) {}

    ConstIterator operator++() noexcept {
      SeekNext();
      return *this;
    }

    const ConstIterator operator++(int) noexcept {
      ConstIterator prev_this = *this;
      SeekNext();
      return prev_this;
    }

    T operator*() const noexcept {
      if constexpr (std::is_same_v<T, asmjit::a64::Gp>) {
        return asmjit::a64::GpX(_index);
      } else {
        return asmjit::x86::Gpq(_index);
      }
    }

    bool operator==(const ConstIterator& rhs) const noexcept {
      return (_index == rhs._index) && (_mask == rhs._mask);
    }
    bool operator!=(const ConstIterator& rhs) const noexcept {
      return not operator==(rhs);
    }

   private:
    friend ConstIterator RegisterMask<T>::begin() const noexcept;
    friend ConstIterator RegisterMask<T>::end() const noexcept;

    void SeekNext() noexcept {
      while (++_index < _mask.size()) {
        if (_mask.test(_index)) {
          break;
        }
      }
    }

    int32_t _index{-1};
    const UnderlyingMask& _mask;
  };

  ConstIterator begin() const noexcept {
    auto it = ConstIterator(_mask);
    it.SeekNext();
    return it;
  }

  ConstIterator end() const noexcept {
    auto it = ConstIterator(_mask);
    it._index = kMaxRegGroupNumRegisters;
    return it;
  }

 private:
  uint32_t ComputeRegBit(const T& reg) const noexcept {
    return 1ULL << reg.id();
  }

  UnderlyingMask _mask;
};

// clang-format off
template <typename T>
concept CallingConvention = requires(T cc) {
  // Register types
  GpReg<typename T::GpReg>;
  VecReg<typename T::VecReg>;
  // Stack pointer
  { T::kSp } -> std::same_as<const typename T::GpReg&>;
  // Arg registers
  { T::kGpArgs } -> std::ranges::common_range;
  { T::kVecArgs } -> std::ranges::common_range;
  // Return registers
  { T::kGpRets } -> std::ranges::common_range;
  { T::kVecRets } -> std::ranges::common_range;
  // The called function will save these (and restore these) if they want 
  // to use them.
  { T::kGpCalleeSavedRegisters } -> std::same_as<const RegisterMask<typename T::GpReg>&>;
  { T::kVecCalleeSavedRegisters } -> std::same_as<const RegisterMask<typename T::VecReg>&>;
  // The calling function will save these if they want them to be kept.
  { T::kGpCallerSavedRegisters } -> std::same_as<const RegisterMask<typename T::GpReg>&>;
  { T::kVecCallerSavedRegisters } -> std::same_as<const RegisterMask<typename T::VecReg>&>;
  { T::kStackAlignment } -> std::same_as<const size_t&>;
};
// clang-format on

}  // namespace wasmcc
