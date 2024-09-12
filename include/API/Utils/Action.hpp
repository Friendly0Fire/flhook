#pragma once

#include "Exceptions/GameException.hpp"

#include "result.hpp"

/**
 * @brief An action class is a wrapper around an internal function that may need to have its error handled gracefully.
 * In the event that something goes wrong, there are multiple ways to handle, including return a default value,
 * run an lambda function with an error code and message paramaters, or throw an exception if the result is not desired in the event of error.
 *
 * @note Internally the action class uses cpp::result, a library based on the C++23 standard library 'result_of' function.
 * @note The specified type can only be one that has a default constructor.
 *
 * @tparam Ret The return type of the calling function
 */
template <typename Ret>
class Action
{
        cpp::result<Ret, Error> result;

    public:
        // ReSharper disable once CppNonExplicitConvertingConstructor
        Action(cpp::result<Ret, Error> res) : result(res) {} // NOLINT(*-explicit-constructor)

        /**
         *  @brief Return the value of the called function that is of type Ret. If no param is provided, and the function errored when called
         *  this will cause a GameException to be thrown. If a std::function of type bool(Error, std::wstring_view) is provided, this will be called
         *  instead of an exception being raised.
         *
         *  @exception GameException A game exception is raised when Handle is called without a parameter and the called function errors.
         *  false.
         */
        Ret Handle()
        {
            if (result.has_error())
            {
                throw GameException(L"An error has occurred.", result.error());
            }

            return result.value();
        }

        /**
         * @brief Return the value of the underlying return type. If the called function errored, the default constructor for the specified type will be called
         * instead.
         */
        Ret Unwrap() noexcept
        {
            if constexpr (std::is_same_v<Ret, void>)
            {
                static_assert(!std::is_same_v<Ret, void>, "Using unwrap of an Action<void> is illogical and malformed.");
                return;
            }
            else
            {
                return result.value_or(Ret());
            }
        }

        /**
         * @brief Returns the underlying internal error object. This is not recommended for standard usage and consumption of functions
         * and is designed to only be used internally.
         */
        const cpp::result<Ret, Error>& Raw() { return result; }
};
