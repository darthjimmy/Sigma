#ifndef BITARRAY_H_INCLUDED
#define BITARRAY_H_INCLUDED

#include <vector>
#include <stdexcept>
#include <memory>
#include <iostream>
#include "SharedPointerMap.hpp"
#include "AlignedVectorAllocator.hpp"

#define ROUND_DOWN(x, s) ((x) & ~((s)-1)) // rounds down x to a multiple of s (i.e. ROUND_DOWN(5, 4) becomes 4)

namespace Sigma {
    /** \brief A reference to simulate an lvalue
     */
    template<class T>
    class reference {
    public:
        reference(T& c, const size_t offset) : c(c), offset(offset) {};

        reference(reference& r) : c(r.c) {
            c = r.c;
            offset = offset;
        }

        reference(reference&& r) : c(r.c) {
            offset = std::move(offset);
        }

        virtual ~reference() {};

        reference& operator=(bool b) {
            if (b) {
                c |= 1 << offset;
            }
            else {
                c &= ~ (1 << offset);
            }
            return *this;
        };

        reference& operator=(reference& r) {
            c = r.c;
            offset = offset;
            return *this;
        }

        reference& operator=(reference&& r) {
            c = std::move(r.c);
            offset = std::move(offset);
            return *this;
        }

        reference& operator|=(bool b) {
            if (b) {
                c |= 1 << offset;
            }
            return *this;
        };

        reference& operator&=(bool b) {
            if (! b) {
                c &= ~ (1 << offset);
            }
            return *this;
        };

        operator bool() const { return (c & (1 << offset)) != 0; };

    private:
        T& c;
        size_t offset;
    };

    /** \brief A bitset like boost::dynamic_bitset
     */
    template<class T>
    class BitArray : public std::enable_shared_from_this<BitArray<T>> {
    public:

        template<class ...Args>
        /** \brief Call the constructor of a BitArray
         *
         * \param args Args&&... arguments to forward
         * \return std::shared_ptr<BitArray> a shared_ptr on the BitArray
         *
         */
        static std::shared_ptr<BitArray<T>> Create(Args&&... args) {
            return std::shared_ptr<BitArray<T>>(new BitArray<T>(std::forward<Args>(args)...));
        }

        // Default destructor
        virtual ~BitArray() {};
        // Copy constructor
        BitArray(BitArray& ba) {
            bitarray = ba.bitarray;
            def_value = ba.def_value;
        }
        // Move Constructor
        BitArray(BitArray&& ba) {
            bitarray = std::move(ba.bitarray);
            def_value = std::move(ba.def_value);
        }
        // Copy assignment
        BitArray& operator=(BitArray& ba) {
            bitarray = ba.bitarray;
            def_value = ba.def_value;
            return *this;
        }
        // Move assignment
        BitArray& operator=(BitArray&& ba) {
            bitarray = std::move(ba.bitarray);
            def_value = std::move(ba.def_value);
            return *this;
        }

        // Compound assignment operators
        // OR combination between 2 BitArrays
        BitArray& operator|=(const BitArray& ba) {
            if (bitarray.size() != ba.bitarray.size()) {
                throw std::length_error("BitArrays have different size !");
            }
            auto it = ba.bitarray.cbegin();
            for (auto &c : bitarray) {
                c |= *it++;
            }
            return *this;
        }

        // AND combination between 2 BitArrays
        BitArray& operator&=(const BitArray& ba) {
            if (bitarray.size() != ba.bitarray.size()) {
                throw new std::length_error("BitArrays have different size !");
            }
            auto it = ba.bitarray.cbegin();
            for (auto &c : bitarray) {
                c &= *it++;
            }
            return *this;
        }

        // XOR combination between 2 BitArrays
        BitArray& operator^=(const BitArray& ba) {
            if (bitarray.size() != ba.bitarray.size()) {
                throw new std::length_error("BitArrays have different size !");
            }
            auto it = ba.bitarray.cbegin();
            for (auto &c : bitarray) {
                c ^= *it++;
            }
            return *this;
        }

        // Bitwise logical operators
        // NOT operation
        BitArray& operator~() {
            for (auto &c : bitarray) {
                c = ~c;
            }
            return *this;
        }

        // OR combination between 2 BitArrays
        BitArray operator|(const BitArray& ba) {
            if (bitarray.size() != ba.bitarray.size()) {
                throw std::length_error("BitArrays have different size !");
            }
            BitArray result(size());
            auto itr = ba.bitarray.cbegin();
            auto itl = bitarray.cbegin();
            for (auto &c : result.bitarray) {
                c = *itl++ | *itr++;
            }
            return result;
        }

        // AND combination between 2 BitArrays
        BitArray operator&(const BitArray& ba) {
            if (bitarray.size() != ba.bitarray.size()) {
                throw new std::length_error("BitArrays have different size !");
            }
            BitArray result(size());
            auto itr = ba.bitarray.cbegin();
            auto itl = bitarray.cbegin();
            for (auto &c : result.bitarray) {
                c = *itl++ & *itr++;
            }
            return result;
        }

        // XOR combination between 2 BitArrays
        BitArray operator^(const BitArray& ba) {
            if (bitarray.size() != ba.bitarray.size()) {
                throw new std::length_error("BitArrays have different size !");
            }
            BitArray result(size());
            auto itr = ba.bitarray.cbegin();
            auto itl = bitarray.cbegin();
            for (auto &c : result.bitarray) {
                c = *itl++ ^ *itr++;
            }
            return result;
        }

        // Access to an element of the BitArray
        bool operator[](size_t idx) const {
            auto offset = idx / blocksize;
            if (offset >= bitarray.size()) {
                throw new std::out_of_range("Index does not exist");
            }
            auto bit_id = idx % blocksize;
            return ((bitarray[offset] & (1 << bit_id)) != 0);
        }

        // left-side reference
        reference<T> operator[](size_t idx) {
            auto offset = idx / blocksize;
            if (offset >= bitarray.size()) {
                    bitarray.resize(offset + 1, def_value);
                    bsize = idx + 1;
            }
            auto bit_id = idx % blocksize;
            return reference<T>(bitarray[offset], bit_id);
        }

        const size_t size() const {
            return bsize;
        }

        const T* data() const {
            return bitarray.data();
        }

        const size_t count() const {
            size_t sum = 0;
            auto length = size();
            #if defined(__GNUG__) || defined(_MSC_VER)
                size_t j = ROUND_DOWN(size(), 64);
                unsigned long long* start = (unsigned long long*) bitarray.data();
                auto end = start + (j >> 6);
                for (auto i = start; i < end; i++) {
                    #if defined(__GNUG__)
                    sum += __builtin_popcountll(*i);
                    #elif defined(_MSC_VER)
                    sum += __popcnt64(*i);
                    #endif
                }
            #else
                size_t j = 0;
            #endif
            for (; j < length; j++) {
                if ((*this)[j]) {
                    sum++;
                }
            }
            return sum;
        }

    private:
        // Default constructor
        BitArray() : bsize(0), def_value(0) {};
        // Constructor with default value
        BitArray(const bool b) : bsize(0), def_value(b ? -1 : 0) {};
        // Constructor with initial size
        BitArray(const size_t s) : bsize(s), def_value(0) {
            bitarray = std::vector<T, AlignedVectorAllocator<T>>((s / blocksize) + 1);
            bitarray.assign(bitarray.size(), this->def_value);
        };
        // Constructor with initial size and default value
        BitArray(const size_t s, const bool b) : bsize(s), def_value(b ? -1 : 0) {
            bitarray = std::vector<T, AlignedVectorAllocator<T>>((s / blocksize) + 1);
            bitarray.assign(bitarray.size(), this->def_value);
        };

        std::vector<T, AlignedVectorAllocator<T>> bitarray;
        size_t bsize;
        T def_value;
        const unsigned int blocksize = sizeof(T) << 3;
    };
}

#endif // BITARRAY_H_INCLUDED
