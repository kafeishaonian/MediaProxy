//
// Created by Hongmingwei on 2025/11/28.
// Result type for unified error handling
//

#ifndef MEDIAPROXY_DNSRESULT_H
#define MEDIAPROXY_DNSRESULT_H

#include <string>
#include <optional>
#include <variant>

namespace dns {

    // Error codes for DNS operations
    enum class DNSErrorCode {
        SUCCESS = 0,
        INVALID_HOSTNAME,
        NETWORK_ERROR,
        TIMEOUT,
        NO_RESULTS,
        CACHE_ERROR,
        PARSE_ERROR,
        UNKNOWN_ERROR
    };

    // Error information
    struct DNSError {
        DNSErrorCode code;
        std::string message;

        DNSError(DNSErrorCode c, const std::string& msg)
            : code(c), message(msg) {}

        static DNSError success() {
            return DNSError(DNSErrorCode::SUCCESS, "");
        }

        static DNSError invalid_hostname(const std::string& hostname) {
            return DNSError(DNSErrorCode::INVALID_HOSTNAME,
                          "Invalid hostname: " + hostname);
        }

        static DNSError network_error(const std::string& detail) {
            return DNSError(DNSErrorCode::NETWORK_ERROR,
                          "Network error: " + detail);
        }

        static DNSError timeout(const std::string& detail = "") {
            return DNSError(DNSErrorCode::TIMEOUT,
                          "Operation timeout" + (detail.empty() ? "" : ": " + detail));
        }

        static DNSError no_results(const std::string& hostname) {
            return DNSError(DNSErrorCode::NO_RESULTS,
                          "No results for: " + hostname);
        }

        static DNSError cache_error(const std::string& detail) {
            return DNSError(DNSErrorCode::CACHE_ERROR,
                          "Cache error: " + detail);
        }

        static DNSError parse_error(const std::string& detail) {
            return DNSError(DNSErrorCode::PARSE_ERROR,
                          "Parse error: " + detail);
        }

        static DNSError unknown(const std::string& detail = "") {
            return DNSError(DNSErrorCode::UNKNOWN_ERROR,
                          "Unknown error" + (detail.empty() ? "" : ": " + detail));
        }

        bool is_success() const {
            return code == DNSErrorCode::SUCCESS;
        }
    };

    // Result type that can hold either a value or an error
    template<typename T>
    class Result {
    public:
        // Success constructor
        explicit Result(T value)
            : value_(std::move(value)), error_(DNSError::success()) {}

        // Error constructor
        explicit Result(DNSError error)
            : value_(std::nullopt), error_(std::move(error)) {}

        // Check if result is successful
        bool is_ok() const {
            return value_.has_value();
        }

        bool is_error() const {
            return !is_ok();
        }

        // Get value (throws if error)
        T& value() {
            if (!is_ok()) {
                throw std::runtime_error("Accessing value of error result: " + error_.message);
            }
            return *value_;
        }

        const T& value() const {
            if (!is_ok()) {
                throw std::runtime_error("Accessing value of error result: " + error_.message);
            }
            return *value_;
        }

        // Get value or default
        T value_or(T default_value) const {
            return is_ok() ? *value_ : default_value;
        }

        // Get error
        const DNSError& error() const {
            return error_;
        }

        // Convenience operators
        explicit operator bool() const {
            return is_ok();
        }

        T* operator->() {
            return value_.has_value() ? &(*value_) : nullptr;
        }

        const T* operator->() const {
            return value_.has_value() ? &(*value_) : nullptr;
        }

    private:
        std::optional<T> value_;
        DNSError error_;
    };

    // Helper functions to create results
    template<typename T>
    Result<T> Ok(T value) {
        return Result<T>(std::move(value));
    }

    template<typename T>
    Result<T> Err(DNSError error) {
        return Result<T>(std::move(error));
    }

} // namespace dns

#endif //MEDIAPROXY_DNSRESULT_H
