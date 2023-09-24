#pragma once
#include <stdexcept>

namespace core {

	/*
	* Thrown if the requested resource does not contain a valid/supported signature
	*/
	class BadSignatureException : public std::runtime_error {
	public:
		BadSignatureException(std::string resource, std::string actual_signature, std::string expected_signature) :
			std::runtime_error("Bad signature at " + resource + ", " + actual_signature + " != " + expected_signature) {}
		BadSignatureException(const char* message) : std::runtime_error(message) {}
		BadSignatureException(std::string message) : std::runtime_error(message) {}
	};

	/*
	* Thrown if the requested resource doesnt contain a valid/supported structure e.g wrong sizes or offsets.
	*/
	class BadStructureException : public std::runtime_error {
	public:
		BadStructureException(std::string resource, std::string reason) :
			std::runtime_error("Bad structure at " + resource + ", " + reason) {}
		BadStructureException(const char* message) : std::runtime_error(message) {}
		BadStructureException(std::string message) : std::runtime_error(message) {}
	};


	/*
	* Thrown if a file cannot be opened, accessed, or saved.
	*/
	class FileIOException : public std::runtime_error {
	public:
		FileIOException(std::string resource, std::string reason) :
			std::runtime_error("File error at " + resource + ", " + reason) {}
		FileIOException(const char* message) : std::runtime_error(message) {}
		FileIOException(std::string message) : std::runtime_error(message) {}
	};
}