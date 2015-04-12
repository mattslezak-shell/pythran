#ifndef PYTHONIC_TYPES_SLICE_HPP
#define PYTHONIC_TYPES_SLICE_HPP

#include "pythonic/__builtin__/None.hpp"

#include <cassert>
#include <stdexcept>
#include <iostream>


namespace pythonic {

    namespace types {

        struct contiguous_slice;

        struct normalized_slice {
            long lower, upper, step;
            normalized_slice() {}
            normalized_slice(long lower, long upper, long step = 1)
                : lower(lower), upper(upper), step(step) {}

            template<class S>
            normalized_slice operator*(S const& other) const {
                return S(lower + step * other.lower, lower + step * other.upper, step * other.step);
            }

            long size() const
            {
                return std::max(0L, long(ceil(double(upper - lower)/double(step))));
            }

            inline long get(long i) const
            {
                return lower + i * step;
            }
        };

        struct slice {
            typedef normalized_slice normalized_type;
            none<long> lower, upper;
            long step;
            slice(none<long> lower, none<long> upper, none<long> step = 1)
                : lower(lower),
                  upper(upper),
                  step(step.is_none?1:(long)step)
            {}
            slice(){}

            slice operator*(slice const& other) const {
                // We do not implement these because it required to know the "end"
                // value of the slice which is not possible if it is not "step == 1" slice
                // TODO : We can skip these constraints if we know begin, end and step.
                if((other.step < 0 or static_cast<long>(other.upper) < 0 or
                            static_cast<long>(other.lower) < 0) and step != 1 and step != -1)
                    throw std::runtime_error("not implemented");

                none<long> new_lower;
                if(other.lower.is_none or (long)other.lower == 0) {
                    if(other.step > 0)
                        new_lower = lower;
                    else if(step > 0) {
                        if(upper.is_none or (long)upper == 0)
                            // 0 means the first value and not the last value
                            new_lower = none_type{};
                        else
                            new_lower = (long)upper - 1;
                    } else {
                        if(upper.is_none or (long)upper == -1)
                            new_lower = none_type{};
                        else
                            new_lower = (long)upper + 1;
                    }
                } else {
                    none<long> ref = ((long)other.lower > 0)?lower:upper;     
                    if(ref.is_none) {
                        if(step > 0)
                            new_lower = (long)other.lower * step;
                        else
                            new_lower = (long)other.lower * step - 1;
                    } else
                        new_lower = (long)ref + (long)other.lower * step;
                }

                long new_step = step * other.step;

                none<long> new_upper;
                if(other.upper.is_none) {
                    if(other.step > 0)
                        new_upper = upper;
                    else if(step > 0) {
                        if(lower.is_none or (long)lower == 0)
                            new_upper = none_type{};
                        else
                            new_upper = (long)lower - 1;
                    } else {
                        if(lower.is_none or (long)lower == -1)
                            // 0 means the first value and not the last value
                            new_upper = none_type{};
                        else
                            new_upper = (long)lower + 1;
                    }
                } else {
                    none<long> ref = ((long)other.upper > 0)?lower:upper;
                    if(ref.is_none) {
                        if(step > 0)
                            new_upper = (long)other.upper * step;
                        else
                            new_upper = (long)other.upper * step - 1;
                    } else
                        new_upper = (long)ref+ (long)other.upper * step;
                }
                return {new_lower, new_upper, new_step};
            }

            slice operator*(contiguous_slice const& other) const;

            /*
               Normalize change a[:-1] to a[:len(a)-1] to have positif index.
               It also check for value bigger than len(a) to fit the size of the container
               */
            normalized_slice normalize(long max_size) const
            {
                long normalized_upper;
                if (upper.is_none) {
                  if (step > 0L)
                    normalized_upper = max_size;
                  else
                    normalized_upper = -1L;
                }
                else {
                  if(upper<0L)
                      normalized_upper = std::max(-1L, max_size + upper);
                  else if(upper> max_size) normalized_upper = max_size;
                  else normalized_upper = (long)upper;
                }

                long normalized_lower;
                if(lower.is_none and step>0L) normalized_lower = 0L;
                else if(lower.is_none and step<0L) normalized_lower = max_size - 1L;
                else if(lower <0L) normalized_lower = std::max(0L, max_size + lower);
                else if(lower > max_size) normalized_lower = max_size - 1L;
                else normalized_lower = (long)lower;

                return {normalized_lower, normalized_upper, step};
            }

            long size() const
            {
                if(upper.is_none and step * lower < 0)
                    return ceil(static_cast<double>(lower) / static_cast<double>(step));
                if(lower.is_none and step * upper > 0)
                    return ceil(static_cast<double>(upper) / static_cast<double>(step));
                assert(not upper.is_none and not lower.is_none);
                return std::max(0L, long(ceil(double(upper - lower)/double(step))));
            }

            long get(long i) const
            {
                assert(not upper.is_none and not lower.is_none);
                return (long)lower + i * (long)step;
            }
        };

        struct contiguous_normalized_slice {
            long lower, upper;
            static constexpr long step = 1;
            contiguous_normalized_slice() {}
            contiguous_normalized_slice(long lower, long upper)
                : lower(lower), upper(upper) {
                }

            contiguous_normalized_slice operator*(contiguous_normalized_slice const& other) const {
                return contiguous_normalized_slice(lower + other.lower, upper + other.upper);
            }
            normalized_slice operator*(normalized_slice const& other) const {
                return normalized_slice(lower + step * other.lower, lower + step * other.upper, step * other.step);
            }

            long size() const
            {
                return std::max(0L, long(ceil(double(upper - lower))));
            }

            inline long get(long i) const
            {
                return lower + i;
            }
        };

        struct contiguous_slice {
            typedef contiguous_normalized_slice normalized_type;
            long lower;
            none<long> upper;
            static const long step;
            contiguous_slice(none<long> lower, none<long> upper)
                : lower(lower.is_none?0:(long)lower), upper(upper) {}
            contiguous_slice(){}

            contiguous_slice operator*(contiguous_slice const& other) const {
                long new_lower;
                if(other.lower < 0)
                    new_lower = upper + other.lower * step;
                else
                    new_lower = lower + other.lower * step;

                none<long> new_upper;
                if(other.upper.is_none)
                    new_upper = upper;
                else if((long)other.upper < 0) {
                    if(upper.is_none)
                        new_upper = (long)other.upper * step;
                    else
                        new_upper = upper + (long)other.upper * step;
                } else
                    new_upper = lower + (long)other.upper * step;

                return {new_lower, new_upper};
            }

            slice operator*(slice const& other) const {
                none<long> new_lower;
                if(other.lower.is_none or (long)other.lower == 0) {
                    if(other.step > 0)
                        new_lower = lower;
                    else if(upper.is_none or (long)upper == 0)
                        // 0 means the first value and not the last value
                        new_lower = none_type{};
                    else
                        new_lower = (long)upper - 1;
                } else {
                    if((long)other.lower > 0)
                        new_lower = lower + (long)other.lower * step;
                    else if(upper.is_none)
                        new_lower = (long)other.lower * step;
                    else
                        new_lower = (long)upper + (long)other.lower * step;
                }

                long new_step = other.step;

                none<long> new_upper;
                if(other.upper.is_none) {
                    if(other.step > 0)
                        new_upper = upper;
                    else if((long)lower == 0)
                        new_upper = none_type{};
                    else
                        new_upper = (long)lower - 1;
                } else {
                    if((long)other.upper > 0)
                        new_upper = lower + (long)other.upper * step;
                    else if(upper.is_none)
                        new_upper = (long)other.upper * step;
                    else
                        new_upper = (long)upper + (long)other.upper * step;
                }
                return {new_lower, new_upper, new_step};
            }


            /*
               Normalize change a[:-1] to a[:len(a)-1] to have positif index.
               It also check for value bigger than len(a) to fit the size of the container
               */
            contiguous_normalized_slice normalize(long max_size) const
            {
                long normalized_upper;
                if(upper.is_none) normalized_upper = max_size;
                else if(upper<0L) normalized_upper = std::max(-1L, max_size + upper);
                else if(upper> max_size) normalized_upper = max_size;
                else normalized_upper = (long)upper;

                long normalized_lower = (long)lower;
                if(lower <0L) normalized_lower = std::max(0L, max_size + lower);
                else if(lower > max_size) normalized_lower = max_size - 1L;
                else normalized_lower = (long)lower;

                return contiguous_normalized_slice(normalized_lower, normalized_upper);
            }

            long size() const
            {
                assert(not upper.is_none);
                return std::max(0L, long(ceil(double(upper - lower))));
            }

            inline long get(long i) const
            {
                return int(lower) + i;
            }
        };

        const long contiguous_slice::step = 1;

        slice slice::operator*(contiguous_slice const& other) const {
            // We do not implement these because it required to know the "end"
            // value of the slice which is not possible if it is not "step == 1" slice
            // TODO : We can skip these constraints if we know begin, end and step.
            if((static_cast<long>(other.upper) < 0 or
                static_cast<long>(other.lower) < 0) and step != 1 and step != -1)
                throw std::runtime_error("not implemented");

            none<long> new_lower;
            if((long)other.lower == 0) {
                new_lower = lower;
            } else {
                none<long> ref = ((long)other.lower > 0)?lower:upper;     
                if(ref.is_none) {
                    if(step > 0)
                        new_lower = (long)other.lower * step;
                    else
                        new_lower = (long)other.lower * step - 1;
                } else
                    new_lower = (long)ref + (long)other.lower * step;
            }

            long new_step = step;

            none<long> new_upper;
            if(other.upper.is_none) {
                new_upper = upper;
            } else {
                none<long> ref = ((long)other.upper > 0)?lower:upper;
                if(ref.is_none) {
                    if(step > 0)
                        new_upper = (long)other.upper * step;
                    else
                        new_upper = (long)other.upper * step - 1;
                } else
                    new_upper = (long)ref+ (long)other.upper * step;
            }
            return {new_lower, new_upper, new_step};
        }
    }
}

#endif
